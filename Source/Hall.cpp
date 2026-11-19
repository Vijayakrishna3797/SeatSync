#include "Hall.h"
#include "Utils.h"
using namespace std;
namespace {
// Neighbour offsets: the first 4 are orthogonal, the last 4 are diagonal.
const int DR[8] = {-1, 1, 0, 0, -1, -1, 1, 1};
const int DC[8] = {0, 0, -1, 1, -1, 1, -1, 1};
}  // namespace

Hall::Hall(const string& name, int rows, int cols)
    : name_(name), rows_(rows), cols_(cols), grid_(rows, vector<Seat>(cols)) {}

const string& Hall::name() const { return name_; }
int Hall::rows() const { return rows_; }
int Hall::cols() const { return cols_; }

bool Hall::inBounds(int r, int c) const { return r >= 0 && r < rows_ && c >= 0 && c < cols_; }

const Seat& Hall::seatAt(int r, int c) const { return grid_[r][c]; }

int Hall::capacity() const { return rows_ * cols_ - blockedCount(); }

int Hall::occupiedCount() const {
    int count = 0;
    for (const auto& row : grid_)
        for (const Seat& s : row)
            if (s.state == SEAT_OCCUPIED) count++;
    return count;
}

int Hall::blockedCount() const {
    int count = 0;
    for (const auto& row : grid_)
        for (const Seat& s : row)
            if (s.state == SEAT_BLOCKED) count++;
    return count;
}

int Hall::freeCount() const { return capacity() - occupiedCount(); }

string Hall::seatLabel(int r, int c) const {
    return string(1, static_cast<char>('A' + r)) + to_string(c + 1);
}

bool Hall::parseSeatLabel(const string& label, int rows, int cols, int& r, int& c) {
    string s = util::toUpper(util::trim(label));
    if (s.size() < 2 || s[0] < 'A' || s[0] > 'Z') return false;
    string digits = s.substr(1);
    for (char ch : digits)
        if (ch < '0' || ch > '9') return false;
    if (digits.size() > 3) return false;
    int row = s[0] - 'A';
    int col = stoi(digits) - 1;
    if (row < 0 || row >= rows || col < 0 || col >= cols) return false;
    r = row;
    c = col;
    return true;
}

string Hall::placementProblem(int r, int c, const string& dept, const SeatingRules& rules) const {
    if (!inBounds(r, c)) return "Seat is outside the hall.";
    const Seat& seat = grid_[r][c];
    if (seat.state == SEAT_BLOCKED) return "Seat " + seatLabel(r, c) + " is blocked.";
    if (seat.state == SEAT_OCCUPIED) return "Seat " + seatLabel(r, c) + " is already occupied by " + seat.regNo + ".";

    int neighbours = rules.checkDiagonals ? 8 : 4;
    for (int k = 0; k < neighbours; k++) {
        int nr = r + DR[k], nc = c + DC[k];
        if (!inBounds(nr, nc)) continue;
        const Seat& other = grid_[nr][nc];
        if (other.state == SEAT_OCCUPIED && other.dept == dept)
            return "Neighbouring seat " + seatLabel(nr, nc) + " already has a " + dept + " student (" + other.regNo + ").";
    }
    return "";
}

bool Hall::canPlace(int r, int c, const string& dept, const SeatingRules& rules) const {
    return placementProblem(r, c, dept, rules).empty();
}

bool Hall::place(int r, int c, const string& regNo, const string& dept, const SeatingRules& rules) {
    if (!canPlace(r, c, dept, rules)) return false;
    restore(r, c, regNo, dept);
    return true;
}

void Hall::restore(int r, int c, const string& regNo, const string& dept) {
    grid_[r][c].state = SEAT_OCCUPIED;
    grid_[r][c].regNo = regNo;
    grid_[r][c].dept = dept;
}

bool Hall::vacate(int r, int c) {
    if (!inBounds(r, c) || grid_[r][c].state != SEAT_OCCUPIED) return false;
    grid_[r][c] = Seat();
    return true;
}

int Hall::vacateAll() {
    int count = 0;
    for (int r = 0; r < rows_; r++)
        for (int c = 0; c < cols_; c++)
            if (vacate(r, c)) count++;
    return count;
}

bool Hall::setBlocked(int r, int c) {
    if (!inBounds(r, c) || grid_[r][c].state != SEAT_FREE) return false;
    grid_[r][c].state = SEAT_BLOCKED;
    return true;
}

bool Hall::setUnblocked(int r, int c) {
    if (!inBounds(r, c) || grid_[r][c].state != SEAT_BLOCKED) return false;
    grid_[r][c].state = SEAT_FREE;
    return true;
}

int Hall::countConflicts(const SeatingRules& rules) const {
    int conflicts = 0;
    // Look only "forward" (right, down, and the two lower diagonals) so each pair is counted once.
    for (int r = 0; r < rows_; r++) {
        for (int c = 0; c < cols_; c++) {
            if (grid_[r][c].state != SEAT_OCCUPIED) continue;
            const int fr[4] = {0, 1, 1, 1};
            const int fc[4] = {1, 0, 1, -1};
            int limit = rules.checkDiagonals ? 4 : 2;
            for (int k = 0; k < limit; k++) {
                int nr = r + fr[k], nc = c + fc[k];
                if (inBounds(nr, nc) && grid_[nr][nc].state == SEAT_OCCUPIED && grid_[nr][nc].dept == grid_[r][c].dept)
                    conflicts++;
            }
        }
    }
    return conflicts;
}
