#include "ReportGenerator.h"
#include "Utils.h"
#include <algorithm>
#include <array>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <ostream>
#include <vector>
using namespace std;

namespace {

// Use the requested location when possible. Some IDEs launch the program with
// a working directory that is read-only, so fall back to the user's temporary
// directory instead of silently failing to create every report.
filesystem::path prepareOutputDirectory(const string& dir, ostream& console) {
    error_code ec;
    filesystem::path requested(dir);
    filesystem::create_directories(requested, ec);
    if (!ec && filesystem::is_directory(requested, ec) && !ec) return requested;

    ec.clear();
    filesystem::path fallback = filesystem::temp_directory_path(ec) / "SeatSync" / "output";
    if (!ec) {
        filesystem::create_directories(fallback, ec);
        if (!ec && filesystem::is_directory(fallback, ec) && !ec) {
            console << "Cannot use output directory '" << requested.string()
                    << "'; saving reports to '" << fallback.string() << "' instead.\n";
            return fallback;
        }
    }

    console << "Could not create output directory '" << requested.string() << "'.";
    if (ec) console << " " << ec.message();
    console << "\n";
    return {};
}

const size_t CELL_WIDTH = 8;   // text inside the [ ] of one seat

// Text shown inside a seat cell: DEPT-123 (dept cut to 4 letters + last 3 digits of the reg no).
string cellText(const Seat& seat) {
    if (seat.state == SEAT_BLOCKED) return util::center("XXX", CELL_WIDTH);
    if (seat.state == SEAT_FREE) return util::center("---", CELL_WIDTH);
    string text = seat.dept.substr(0, 4) + "-" + util::lastChars(seat.regNo, 3);
    return util::padRight(text, CELL_WIDTH);
}

// All students seated in a hall as (regNo, row, col), sorted by registration number.
struct Placement { string regNo; int row; int col; };

vector<Placement> placementsOf(const Hall& hall) {
    vector<Placement> list;
    for (int r = 0; r < hall.rows(); r++)
        for (int c = 0; c < hall.cols(); c++)
            if (hall.seatAt(r, c).state == SEAT_OCCUPIED) list.push_back({hall.seatAt(r, c).regNo, r, c});
    sort(list.begin(), list.end(), [](const Placement& x, const Placement& y) { return x.regNo < y.regNo; });
    return list;
}

string percent(int part, int whole) {
    if (whole == 0) return "0.0%";
    ostringstream ss;
    ss << fixed << setprecision(1) << (100.0 * part / whole) << "%";
    return ss.str();
}

}  // namespace

// ------------------------------------------------------------------ hall picture

void report::printHallLayout(const Hall& hall, ostream& os) {
    os << "Hall: " << hall.name() << "  (" << hall.rows() << " rows x " << hall.cols() << " columns)\n";
    os << "Occupied: " << hall.occupiedCount() << "/" << hall.capacity()
       << "   Blocked seats: " << hall.blockedCount() << "\n\n";

    os << "     ";
    for (int c = 0; c < hall.cols(); c++) os << util::center(to_string(c + 1), CELL_WIDTH + 2);
    os << "\n";
    for (int r = 0; r < hall.rows(); r++) {
        os << "  " << static_cast<char>('A' + r) << "  ";
        for (int c = 0; c < hall.cols(); c++) os << "[" << cellText(hall.seatAt(r, c)) << "]";
        os << "\n";
    }
    os << "\nLegend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked\n";
}

// ------------------------------------------------------------------ student view

void report::printStudentSeat(const SeatAllocator& a, const string& reg, ostream& os) {
    const Student* s = a.findStudent(reg);
    if (!s) {
        os << "No student is registered with number " << util::toUpper(util::trim(reg)) << ".\n";
        return;
    }
    os << "Name       : " << s->name << "\n";
    os << "Reg. No.   : " << s->regNo << "\n";
    os << "Department : " << s->dept << "\n";

    SeatLocation loc;
    if (!a.locationOf(s->regNo, loc)) {
        os << "Status     : Registered, but NOT yet allocated a seat.\n";
        return;
    }
    const Hall& h = a.halls()[loc.hall];
    os << "Hall       : " << h.name() << "\n";
    os << "Seat       : " << h.seatLabel(loc.row, loc.col)
       << "  (row " << static_cast<char>('A' + loc.row) << ", column " << loc.col + 1 << ")\n";
}

void report::printWaitingList(const SeatAllocator& a, ostream& os) {
    vector<Student> list = a.waiting().toVector();
    if (list.empty()) {
        os << "No students are waiting. Everybody registered has a seat.\n";
        return;
    }
    os << "Students waiting for a seat (" << list.size() << "), in queue order:\n";
    for (const Student& s : list)
        os << "  " << util::padRight(s.regNo, 12) << util::padRight(s.name, 24) << s.dept << "\n";
}

// ------------------------------------------------------------------ attendance

void report::printAttendanceSheet(const SeatAllocator& a, int hallIdx, ostream& os) {
    const Hall& hall = a.halls()[hallIdx];
    vector<Placement> list = placementsOf(hall);

    os << "==================================================================================\n";
    os << "                         ATTENDANCE SHEET  -  " << hall.name() << "\n";
    os << "==================================================================================\n";
    os << util::padRight("S.No", 6) << util::padRight("Reg. No.", 13) << util::padRight("Name", 25)
       << util::padRight("Dept", 7) << util::padRight("Seat", 6) << "Signature\n";
    os << "----------------------------------------------------------------------------------\n";
    int serial = 1;
    for (const Placement& p : list) {
        const Student* s = a.findStudent(p.regNo);
        os << util::padRight(to_string(serial++), 6) << util::padRight(p.regNo, 13)
           << util::padRight(s->name, 25) << util::padRight(s->dept, 7)
           << util::padRight(hall.seatLabel(p.row, p.col), 6) << "____________\n";
    }
    os << "----------------------------------------------------------------------------------\n";
    os << "Total present on sheet: " << list.size() << "\n";
}

int report::writeAttendanceSheets(const SeatAllocator& a, const string& dir, ostream& console) {
    filesystem::path outputDir = prepareOutputDirectory(dir, console);
    if (outputDir.empty()) return -1;
    int written = 0;
    for (size_t i = 0; i < a.halls().size(); i++) {
        if (a.halls()[i].occupiedCount() == 0) continue;     // nothing to sign in this hall
        printAttendanceSheet(a, static_cast<int>(i), console);
        filesystem::path path = outputDir / ("attendance_" + util::sanitizeFileName(a.halls()[i].name()) + ".txt");
        ofstream file(path);
        if (!file) { console << "Could not write " << path.string() << "\n"; continue; }
        printAttendanceSheet(a, static_cast<int>(i), file);
        console << "Saved: " << path.string() << "\n\n";
        written++;
    }
    return written;
}

// ------------------------------------------------------------------ examination report

void report::printExamReport(const SeatAllocator& a, ostream& os) {
    int total = static_cast<int>(a.students().size());
    int seated = a.allocatedCount();

    os << "==================================================\n";
    os << "          EXAMINATION SEATING REPORT\n";
    os << "==================================================\n";
    os << "Seating rule        : " << a.rulesDescription() << "\n";
    os << "Halls               : " << a.halls().size() << "\n";
    os << "Students registered : " << total << "\n";
    os << "Students seated     : " << seated << " (" << percent(seated, total) << ")\n";
    os << "Students waiting    : " << a.waiting().size() << "\n";
    os << "Rule conflicts      : " << a.totalConflicts() << (a.totalConflicts() == 0 ? "  (layout is valid)" : "  (CHECK LAYOUT!)") << "\n\n";

    os << "Hall-wise utilisation\n";
    os << util::padRight("Hall", 14) << util::padRight("Layout", 9) << util::padRight("Usable", 8)
       << util::padRight("Seated", 8) << util::padRight("Blocked", 9) << "Utilisation\n";
    os << "--------------------------------------------------------\n";
    int totalCap = 0, totalOcc = 0;
    for (const Hall& h : a.halls()) {
        string layout = to_string(h.rows()) + "x" + to_string(h.cols());
        os << util::padRight(h.name(), 14) << util::padRight(layout, 9) << util::padRight(to_string(h.capacity()), 8)
           << util::padRight(to_string(h.occupiedCount()), 8) << util::padRight(to_string(h.blockedCount()), 9)
           << percent(h.occupiedCount(), h.capacity()) << "\n";
        totalCap += h.capacity();
        totalOcc += h.occupiedCount();
    }
    os << "--------------------------------------------------------\n";
    os << util::padRight("TOTAL", 23) << util::padRight(to_string(totalCap), 8)
       << util::padRight(to_string(totalOcc), 17) << percent(totalOcc, totalCap) << "\n\n";

    // Department-wise table: registered / seated / waiting.
    map<string, array<int, 3>> byDept;   // sorted by dept name
    for (const Student& s : a.students()) {
        byDept[s.dept][0]++;
        SeatLocation loc;
        if (a.locationOf(s.regNo, loc)) byDept[s.dept][1]++;
        else byDept[s.dept][2]++;
    }
    os << "Department-wise distribution\n";
    os << util::padRight("Dept", 8) << util::padRight("Registered", 12) << util::padRight("Seated", 8) << "Waiting\n";
    os << "-----------------------------------\n";
    for (const auto& entry : byDept)
        os << util::padRight(entry.first, 8) << util::padRight(to_string(entry.second[0]), 12)
           << util::padRight(to_string(entry.second[1]), 8) << entry.second[2] << "\n";
}

// ------------------------------------------------------------------ export

bool report::exportSeatingPlan(const SeatAllocator& a, const string& dir, ostream& console) {
    filesystem::path outputDir = prepareOutputDirectory(dir, console);
    if (outputDir.empty()) return false;

    filesystem::path csvPath = outputDir / "seating_plan.csv";
    filesystem::path txtPath = outputDir / "seating_plan.txt";
    ofstream csv(csvPath);
    ofstream txt(txtPath);
    if (!csv || !txt) {
        console << "Could not create files inside '" << outputDir.string() << "'.\n";
        return false;
    }

    csv << "RegNo,Name,Dept,Hall,Seat,Row,Column\n";
    for (const Hall& h : a.halls()) {
        for (const Placement& p : placementsOf(h)) {
            const Student* s = a.findStudent(p.regNo);
            csv << p.regNo << "," << s->name << "," << s->dept << "," << h.name() << ","
                << h.seatLabel(p.row, p.col) << "," << static_cast<char>('A' + p.row) << "," << p.col + 1 << "\n";
        }
        printHallLayout(h, txt);
        txt << "\n";
    }
    console << "Saved: " << csvPath.string() << "\n";
    console << "Saved: " << txtPath.string() << "\n";
    return true;
}
