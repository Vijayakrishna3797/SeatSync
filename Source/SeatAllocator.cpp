#include "SeatAllocator.h"
#include "Utils.h"
#include <algorithm>

namespace {

// Builds the processing order from the waiting queue.
// Students are grouped into one FIFO queue per department, and the department
// queues are then served round-robin (CSE, ECE, EEE, CSE, ECE, ...). This mixes
// departments so the "no same department side by side" rule is rarely violated,
// while students of one department are still seated in registration order.
StudentQueue buildRoundRobinQueue(const StudentQueue& waiting) {
    std::vector<std::string> deptOrder;                       // order of first appearance
    std::unordered_map<std::string, StudentQueue> deptQueues;

    for (const Student& s : waiting.toVector()) {
        if (deptQueues.find(s.dept) == deptQueues.end()) deptOrder.push_back(s.dept);
        deptQueues[s.dept].enqueue(s);
    }

    StudentQueue merged;
    bool movedAny = true;
    while (movedAny) {
        movedAny = false;
        for (const std::string& dept : deptOrder) {
            Student s;
            if (deptQueues[dept].dequeue(s)) {
                merged.enqueue(s);
                movedAny = true;
            }
        }
    }
    return merged;
}

}  // namespace

// ------------------------------------------------------------------ setup

bool SeatAllocator::addHall(const std::string& rawName, int rows, int cols, std::string& err) {
    std::string name = util::trim(rawName);
    if (name.empty()) { err = "Hall name cannot be empty."; return false; }
    if (rows < 1 || rows > Hall::MAX_ROWS) {
        err = "Rows must be between 1 and " + std::to_string(Hall::MAX_ROWS) + ".";
        return false;
    }
    if (cols < 1 || cols > Hall::MAX_COLS) {
        err = "Columns must be between 1 and " + std::to_string(Hall::MAX_COLS) + ".";
        return false;
    }
    for (const Hall& h : halls_) {
        if (util::toUpper(h.name()) == util::toUpper(name)) {
            err = "A hall named '" + h.name() + "' already exists.";
            return false;
        }
    }
    halls_.push_back(Hall(name, rows, cols));
    return true;
}

RegisterResult SeatAllocator::registerStudent(const Student& raw, std::string& err) {
    Student s;
    s.regNo = util::toUpper(util::trim(raw.regNo));
    s.name = util::trim(raw.name);
    s.dept = util::toUpper(util::trim(raw.dept));

    if (!util::isAlphaNumeric(s.regNo)) { err = "Registration number must be letters/digits only."; return REG_INVALID; }
    if (s.name.empty()) { err = "Name cannot be empty."; return REG_INVALID; }
    if (!util::isAlphaNumeric(s.dept)) { err = "Department must be letters/digits only (e.g. CSE)."; return REG_INVALID; }
    if (regIndex_.count(s.regNo)) { err = "Registration number " + s.regNo + " is already registered."; return REG_DUPLICATE; }

    regIndex_[s.regNo] = static_cast<int>(students_.size());
    students_.push_back(s);
    waiting_.enqueue(s);     // every new student joins the back of the queue
    return REG_OK;
}

// ------------------------------------------------------------------ allocation

bool SeatAllocator::findSeatFor(const Student& s, SeatLocation& loc) const {
    for (std::size_t h = 0; h < halls_.size(); h++) {
        for (int r = 0; r < halls_[h].rows(); r++) {
            for (int c = 0; c < halls_[h].cols(); c++) {
                if (halls_[h].canPlace(r, c, s.dept, rules_)) {
                    loc.hall = static_cast<int>(h);
                    loc.row = r;
                    loc.col = c;
                    return true;
                }
            }
        }
    }
    return false;
}

int SeatAllocator::allocatePending() {
    if (halls_.empty()) return 0;

    StudentQueue processing = buildRoundRobinQueue(waiting_);
    StudentQueue stillWaiting;
    int seated = 0;

    Student s;
    while (processing.dequeue(s)) {
        SeatLocation loc;
        if (findSeatFor(s, loc)) {
            halls_[loc.hall].place(loc.row, loc.col, s.regNo, s.dept, rules_);
            seatOf_[s.regNo] = loc;
            seated++;
        } else {
            stillWaiting.enqueue(s);   // no legal seat right now
        }
    }
    waiting_ = stillWaiting;
    return seated;
}

void SeatAllocator::clearAllocations() {
    for (Hall& h : halls_) h.vacateAll();
    seatOf_.clear();
    waiting_.clear();
    for (const Student& s : students_) waiting_.enqueue(s);
}

// ------------------------------------------------------------------ admin modifications

bool SeatAllocator::swapSeats(const std::string& rawA, const std::string& rawB, std::string& err) {
    std::string a = util::toUpper(util::trim(rawA));
    std::string b = util::toUpper(util::trim(rawB));
    if (a == b) { err = "Please enter two different students."; return false; }

    SeatLocation la, lb;
    if (!locationOf(a, la)) { err = a + " is not seated."; return false; }
    if (!locationOf(b, lb)) { err = b + " is not seated."; return false; }

    std::string deptA = students_[regIndex_.at(a)].dept;
    std::string deptB = students_[regIndex_.at(b)].dept;

    // Take both out, then try to place them crosswise.
    halls_[la.hall].vacate(la.row, la.col);
    halls_[lb.hall].vacate(lb.row, lb.col);

    std::string problem = halls_[lb.hall].placementProblem(lb.row, lb.col, deptA, rules_);
    if (problem.empty()) {
        halls_[lb.hall].restore(lb.row, lb.col, a, deptA);
        problem = halls_[la.hall].placementProblem(la.row, la.col, deptB, rules_);
        if (problem.empty()) {
            halls_[la.hall].restore(la.row, la.col, b, deptB);
            seatOf_[a] = lb;
            seatOf_[b] = la;
            return true;
        }
        halls_[lb.hall].vacate(lb.row, lb.col);   // undo the first placement
        err = b + " cannot take the seat of " + a + ": " + problem;
    } else {
        err = a + " cannot take the seat of " + b + ": " + problem;
    }
    // Swap rejected: put everybody back exactly where they were.
    halls_[la.hall].restore(la.row, la.col, a, deptA);
    halls_[lb.hall].restore(lb.row, lb.col, b, deptB);
    return false;
}

bool SeatAllocator::moveStudent(const std::string& rawReg, int hallIdx, int row, int col, std::string& err) {
    std::string reg = util::toUpper(util::trim(rawReg));
    const Student* s = findStudent(reg);
    if (!s) { err = "No student with registration number " + reg + "."; return false; }
    if (hallIdx < 0 || hallIdx >= static_cast<int>(halls_.size())) { err = "Invalid hall."; return false; }
    if (!halls_[hallIdx].inBounds(row, col)) { err = "Seat is outside the hall."; return false; }

    SeatLocation old;
    bool seated = locationOf(reg, old);
    SeatLocation target;
    target.hall = hallIdx; target.row = row; target.col = col;
    if (seated && old == target) { err = reg + " is already sitting there."; return false; }

    if (seated) halls_[old.hall].vacate(old.row, old.col);   // free the old seat first

    std::string problem = halls_[hallIdx].placementProblem(row, col, s->dept, rules_);
    if (!problem.empty()) {
        if (seated) halls_[old.hall].restore(old.row, old.col, reg, s->dept);   // undo
        err = problem;
        return false;
    }
    halls_[hallIdx].restore(row, col, reg, s->dept);
    seatOf_[reg] = target;
    if (!seated) removeFromWaiting(reg);    // manual seating of a waiting student
    return true;
}

bool SeatAllocator::blockSeat(int hallIdx, int row, int col, BlockOutcome& out, std::string& err) {
    if (hallIdx < 0 || hallIdx >= static_cast<int>(halls_.size())) { err = "Invalid hall."; return false; }
    Hall& hall = halls_[hallIdx];
    if (!hall.inBounds(row, col)) { err = "Seat is outside the hall."; return false; }

    const Seat& seat = hall.seatAt(row, col);
    if (seat.state == SEAT_BLOCKED) { err = "Seat " + hall.seatLabel(row, col) + " is already blocked."; return false; }

    out = BlockOutcome();
    if (seat.state == SEAT_OCCUPIED) {
        out.displaced = true;
        out.regNo = seat.regNo;
        hall.vacate(row, col);
        seatOf_.erase(out.regNo);
    }
    hall.setBlocked(row, col);

    if (out.displaced) {
        // Try to re-seat the displaced student straight away; otherwise queue them.
        const Student& s = students_[regIndex_.at(out.regNo)];
        SeatLocation loc;
        if (findSeatFor(s, loc)) {
            halls_[loc.hall].place(loc.row, loc.col, s.regNo, s.dept, rules_);
            seatOf_[s.regNo] = loc;
            out.reseated = true;
            out.newLoc = loc;
        } else {
            waiting_.enqueue(s);
        }
    }
    return true;
}

bool SeatAllocator::unblockSeat(int hallIdx, int row, int col, std::string& err) {
    if (hallIdx < 0 || hallIdx >= static_cast<int>(halls_.size())) { err = "Invalid hall."; return false; }
    Hall& hall = halls_[hallIdx];
    if (!hall.inBounds(row, col)) { err = "Seat is outside the hall."; return false; }
    if (!hall.setUnblocked(row, col)) { err = "Seat " + hall.seatLabel(row, col) + " is not blocked."; return false; }
    return true;
}

bool SeatAllocator::removeStudent(const std::string& rawReg, std::string& err) {
    std::string reg = util::toUpper(util::trim(rawReg));
    if (!regIndex_.count(reg)) { err = "No student with registration number " + reg + "."; return false; }

    SeatLocation loc;
    if (locationOf(reg, loc)) {
        halls_[loc.hall].vacate(loc.row, loc.col);
        seatOf_.erase(reg);
    } else {
        removeFromWaiting(reg);
    }
    students_.erase(students_.begin() + regIndex_[reg]);
    rebuildIndex();
    return true;
}

// ------------------------------------------------------------------ queries

const Student* SeatAllocator::findStudent(const std::string& reg) const {
    auto it = regIndex_.find(util::toUpper(util::trim(reg)));
    if (it == regIndex_.end()) return nullptr;
    return &students_[it->second];
}

bool SeatAllocator::locationOf(const std::string& reg, SeatLocation& loc) const {
    auto it = seatOf_.find(util::toUpper(util::trim(reg)));
    if (it == seatOf_.end()) return false;
    loc = it->second;
    return true;
}

int SeatAllocator::totalConflicts() const {
    int total = 0;
    for (const Hall& h : halls_) total += h.countConflicts(rules_);
    return total;
}

std::string SeatAllocator::rulesDescription() const {
    if (rules_.checkDiagonals)
        return "No two students of the same department may sit next to each other "
               "(front, back, left, right AND diagonals).";
    return "No two students of the same department may sit next to each other "
           "(front, back, left, right).";
}

// ------------------------------------------------------------------ private helpers

void SeatAllocator::removeFromWaiting(const std::string& reg) {
    StudentQueue rebuilt;
    Student s;
    while (waiting_.dequeue(s))
        if (s.regNo != reg) rebuilt.enqueue(s);
    waiting_ = rebuilt;
}

bool SeatAllocator::isWaiting(const std::string& reg) const {
    for (const Student& s : waiting_.toVector())
        if (s.regNo == reg) return true;
    return false;
}

void SeatAllocator::rebuildIndex() {
    regIndex_.clear();
    for (std::size_t i = 0; i < students_.size(); i++) regIndex_[students_[i].regNo] = static_cast<int>(i);
}
