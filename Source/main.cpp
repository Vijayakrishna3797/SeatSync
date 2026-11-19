/**
 * @file main.cpp
 * @brief SeatSync - Online Exam Seating Allocation System (menu-driven administrator console).
 */
#include "SeatAllocator.h"
#include "DataLoader.h"
#include "ReportGenerator.h"
#include "Utils.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

namespace {

const string OUTPUT_DIR = "output";   // attendance sheets and seating plans are written here

// When the environment variable SEATSYNC_ECHO is set, every line read is echoed back.
// This makes transcripts of scripted runs (used for the test cases) look like a real session.
bool echoInput = false;

// Thrown when the input stream ends (Ctrl+D / end of a test script) so we can exit cleanly.
struct InputClosed {};

// ---------------------------------------------------------------- input helpers

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) throw InputClosed();
    if (echoInput) cout << line << "\n";
    return util::trim(line);
}

// Keeps asking until the user enters a whole number between lo and hi.
int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string line = readLine(prompt);
        try {
            size_t used = 0;
            int value = stoi(line, &used);
            if (used == line.size() && value >= lo && value <= hi) return value;
        } catch (...) {
            // fall through to the message below
        }
        cout << "  Please enter a whole number between " << lo << " and " << hi << ".\n";
    }
}

// Lists the halls and lets the user pick one. Returns false if there are no halls.
bool chooseHall(const SeatAllocator& a, int& hallIdx) {
    if (a.halls().empty()) {
        cout << "No halls available yet. Add a hall first (option 1).\n";
        return false;
    }
    for (size_t i = 0; i < a.halls().size(); i++)
        cout << "  " << i + 1 << ". " << a.halls()[i].name() << " (" << a.halls()[i].rows() << "x"
             << a.halls()[i].cols() << ")\n";
    hallIdx = readInt("Select hall number: ", 1, static_cast<int>(a.halls().size())) - 1;
    return true;
}

// Asks for a seat label such as B4 until a valid one for this hall is entered.
void chooseSeat(const Hall& hall, int& row, int& col) {
    while (true) {
        string label = readLine("Enter seat (e.g. B4): ");
        if (Hall::parseSeatLabel(label, hall.rows(), hall.cols(), row, col)) return;
        cout << "  Invalid seat. Rows are A-" << static_cast<char>('A' + hall.rows() - 1) << ", columns 1-"
             << hall.cols() << ".\n";
    }
}

// ---------------------------------------------------------------- menu actions

void addHall(SeatAllocator& a) {
    string name = readLine("Hall name (e.g. Hall-A): ");
    int rows = readInt("Number of rows (1-26): ", 1, 26);
    int cols = readInt("Seats per row (1-30): ", 1, 30);
    string err;
    if (a.addHall(name, rows, cols, err)) cout << "Hall '" << name << "' added with " << rows * cols << " seats.\n";
    else cout << "Could not add hall: " << err << "\n";
}

void registerOneStudent(SeatAllocator& a) {
    Student s;
    s.regNo = readLine("Registration number: ");
    s.name = readLine("Name: ");
    s.dept = readLine("Department (e.g. CSE): ");
    string err;
    if (a.registerStudent(s, err) == REG_OK) cout << "Student registered and added to the allocation queue.\n";
    else cout << "Could not register student: " << err << "\n";
}

void loadCSV(SeatAllocator& a) {
    string path = readLine("CSV file path (e.g. data/students_sample.csv): ");
    LoadSummary summary;
    if (!loadStudentsFromCSV(path, a, summary)) {
        cout << "Could not open file '" << path << "'.\n";
        return;
    }
    cout << "Lines read: " << summary.linesRead << " | Loaded: " << summary.loaded
         << " | Duplicates: " << summary.duplicates << " | Invalid: " << summary.invalid << "\n";
    for (const string& m : summary.messages) cout << "  " << m << "\n";
}

void demoStudents(SeatAllocator& a) {
    int count = readInt("How many demo students (1-500)? ", 1, 500);
    cout << generateDemoStudents(a, count) << " demo students registered.\n";
}

void runAllocation(SeatAllocator& a) {
    if (a.halls().empty()) { cout << "Add at least one hall first (option 1).\n"; return; }
    if (a.waiting().isEmpty()) { cout << "No students are waiting for a seat.\n"; return; }
    int seated = a.allocatePending();
    cout << "Allocation complete: " << seated << " student(s) seated, " << a.waiting().size() << " still waiting.\n";
    cout << "Rule check: " << a.totalConflicts() << " conflict(s) found.\n";
    if (!a.waiting().isEmpty())
        cout << "Some students could not be placed legally (not enough seats, or spacing rule too strict).\n"
             << "Add another hall or unblock seats, then run allocation again (option 5).\n";
}

void viewLayout(const SeatAllocator& a) {
    int h;
    if (!chooseHall(a, h)) return;
    cout << "\n";
    report::printHallLayout(a.halls()[h], cout);
}

void findSeat(const SeatAllocator& a) {
    string reg = readLine("Enter registration number: ");
    cout << "\n";
    report::printStudentSeat(a, reg, cout);
}

void swapSeats(SeatAllocator& a) {
    string r1 = readLine("First student's registration number: ");
    string r2 = readLine("Second student's registration number: ");
    string err;
    if (a.swapSeats(r1, r2, err)) cout << "Seats swapped successfully.\n";
    else cout << "Swap rejected: " << err << "\n";
}

void moveStudent(SeatAllocator& a) {
    string reg = readLine("Student's registration number: ");
    int h;
    if (!chooseHall(a, h)) return;
    int row, col;
    chooseSeat(a.halls()[h], row, col);
    string err;
    if (a.moveStudent(reg, h, row, col, err))
        cout << "Student moved to " << a.halls()[h].name() << ", seat " << a.halls()[h].seatLabel(row, col) << ".\n";
    else cout << "Move rejected: " << err << "\n";
}

void blockOrUnblock(SeatAllocator& a) {
    int h;
    if (!chooseHall(a, h)) return;
    int row, col;
    chooseSeat(a.halls()[h], row, col);
    int action = readInt("1 = Block seat (broken/reserved), 2 = Unblock seat: ", 1, 2);
    string err;
    if (action == 1) {
        BlockOutcome out;
        if (!a.blockSeat(h, row, col, out, err)) { cout << "Could not block seat: " << err << "\n"; return; }
        cout << "Seat " << a.halls()[h].seatLabel(row, col) << " is now blocked.\n";
        if (out.displaced) {
            cout << "Student " << out.regNo << " was sitting there. ";
            if (out.reseated)
                cout << "Re-seated automatically at " << a.halls()[out.newLoc.hall].name() << ", seat "
                     << a.halls()[out.newLoc.hall].seatLabel(out.newLoc.row, out.newLoc.col) << ".\n";
            else cout << "No legal seat is free, so the student is back in the waiting queue.\n";
        }
    } else {
        if (a.unblockSeat(h, row, col, err)) cout << "Seat " << a.halls()[h].seatLabel(row, col) << " is available again.\n";
        else cout << "Could not unblock seat: " << err << "\n";
    }
}

void removeStudent(SeatAllocator& a) {
    string reg = readLine("Registration number to remove: ");
    string err;
    if (a.removeStudent(reg, err)) cout << "Student removed; their seat (if any) is free again.\n";
    else cout << "Could not remove student: " << err << "\n";
}

void toggleDiagonal(SeatAllocator& a) {
    a.setDiagonalRule(!a.rules().checkDiagonals);
    cout << "Diagonal spacing rule is now " << (a.rules().checkDiagonals ? "ON" : "OFF") << ".\n";
    cout << a.rulesDescription() << "\n";
    cout << "(Applies to future placements. Use option 13 then option 5 to re-allocate everyone.)\n";
}

void clearAllocations(SeatAllocator& a) {
    string answer = readLine("This empties every seat. Type 'yes' to confirm: ");
    if (util::toUpper(answer) == "YES") {
        a.clearAllocations();
        cout << "All seats cleared. " << a.waiting().size() << " student(s) are waiting for allocation.\n";
    } else {
        cout << "Cancelled.\n";
    }
}

void printMenu() {
    cout << "\n=====================================\n";
    cout << "           SEATSYNC MENU\n";
    cout << "=====================================\n";
    cout << " 1. Add Examination Hall\n";
    cout << " 2. Register Student\n";
    cout << " 3. Load Students from CSV File\n";
    cout << " 4. Generate Demo Students\n";
    cout << " 5. Run Seat Allocation\n";
    cout << " 6. View Hall Seating Layout\n";
    cout << " 7. Find a Student's Seat\n";
    cout << " 8. Swap Two Students' Seats\n";
    cout << " 9. Move a Student to Another Seat\n";
    cout << "10. Block / Unblock a Seat\n";
    cout << "11. Remove a Student\n";
    cout << "12. Toggle Diagonal Spacing Rule\n";
    cout << "13. Clear All Allocations\n";
    cout << "14. Generate Attendance Sheets\n";
    cout << "15. Generate Examination Report\n";
    cout << "16. Export Seating Plan (CSV + TXT)\n";
    cout << "17. List Students Without a Seat\n";
    cout << "18. Exit\n";
    cout << "-------------------------------------\n";
}

}  // namespace

int main() {
    cout << "==========================================\n";
    cout << "   SEATSYNC - EXAM SEATING ALLOCATION\n";
    cout << "==========================================\n";

    echoInput = (std::getenv("SEATSYNC_ECHO") != nullptr);

    SeatAllocator allocator;
    cout << "Rule: " << allocator.rulesDescription() << "\n";

    try {
        bool running = true;
        while (running) {
            printMenu();
            int choice = readInt("Enter your choice: ", 1, 18);
            cout << "\n";
            switch (choice) {
                case 1:  addHall(allocator); break;
                case 2:  registerOneStudent(allocator); break;
                case 3:  loadCSV(allocator); break;
                case 4:  demoStudents(allocator); break;
                case 5:  runAllocation(allocator); break;
                case 6:  viewLayout(allocator); break;
                case 7:  findSeat(allocator); break;
                case 8:  swapSeats(allocator); break;
                case 9:  moveStudent(allocator); break;
                case 10: blockOrUnblock(allocator); break;
                case 11: removeStudent(allocator); break;
                case 12: toggleDiagonal(allocator); break;
                case 13: clearAllocations(allocator); break;
                case 14:
                    if (report::writeAttendanceSheets(allocator, OUTPUT_DIR, cout) == 0)
                        cout << "No seated students yet, so there are no attendance sheets.\n";
                    break;
                case 15: report::printExamReport(allocator, cout); break;
                case 16: report::exportSeatingPlan(allocator, OUTPUT_DIR, cout); break;
                case 17: report::printWaitingList(allocator, cout); break;
                case 18:
                    running = false;
                    cout << "Exiting SeatSync. Good luck to all the students!\n";
                    break;
            }
        }
    } catch (const InputClosed&) {
        cout << "\n\nInput ended. Exiting SeatSync.\n";
    }
    return 0;
}
