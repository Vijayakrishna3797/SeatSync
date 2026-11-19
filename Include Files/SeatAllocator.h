/**
 * @file SeatAllocator.h
 * @brief Core engine of SeatSync: owns the halls, the student registry and the waiting queue.
 *
 * Data structures used:
 *   - StudentQueue (FIFO)        : students waiting for a seat, processed in order
 *   - Hall (2D matrix of Seats)  : physical layout, used to enforce spacing rules
 *   - unordered_map (hash table) : O(1) lookup of a student's registration / seat
 */
#pragma once
#include "Hall.h"
#include "Student.h"
#include "StudentQueue.h"
#include <string>
#include <unordered_map>
#include <vector>

// Position of a seated student: which hall, which row and column.
struct SeatLocation {
    int hall = -1;
    int row = -1;
    int col = -1;
    bool operator==(const SeatLocation& o) const { return hall == o.hall && row == o.row && col == o.col; }
};

enum RegisterResult { REG_OK, REG_DUPLICATE, REG_INVALID };

// What happened when an admin blocked a seat.
struct BlockOutcome {
    bool displaced = false;    // true if a student was sitting there
    std::string regNo;         // the displaced student
    bool reseated = false;     // true if a new seat was found automatically
    SeatLocation newLoc;       // where the student was re-seated
};

class SeatAllocator {
public:
    // ---- setup ----
    bool addHall(const std::string& name, int rows, int cols, std::string& err);
    RegisterResult registerStudent(const Student& s, std::string& err);

    // ---- allocation ----
    int allocatePending();                      // seats everyone in the waiting queue; returns #seated
    void clearAllocations();                    // empties all seats, everybody goes back to the queue

    // ---- administrator modifications (all keep the spacing rules intact) ----
    bool swapSeats(const std::string& regA, const std::string& regB, std::string& err);
    bool moveStudent(const std::string& reg, int hallIdx, int row, int col, std::string& err);
    bool blockSeat(int hallIdx, int row, int col, BlockOutcome& out, std::string& err);
    bool unblockSeat(int hallIdx, int row, int col, std::string& err);
    bool removeStudent(const std::string& reg, std::string& err);

    // ---- queries ----
    const std::vector<Hall>& halls() const { return halls_; }
    const std::vector<Student>& students() const { return students_; }
    const StudentQueue& waiting() const { return waiting_; }
    const Student* findStudent(const std::string& reg) const;
    bool locationOf(const std::string& reg, SeatLocation& loc) const;
    int allocatedCount() const { return static_cast<int>(seatOf_.size()); }
    int totalConflicts() const;                 // adjacent same-dept pairs over all halls

    // ---- rules ----
    const SeatingRules& rules() const { return rules_; }
    void setDiagonalRule(bool on) { rules_.checkDiagonals = on; }
    std::string rulesDescription() const;

private:
    std::vector<Hall> halls_;
    std::vector<Student> students_;                         // registration order
    std::unordered_map<std::string, int> regIndex_;         // regNo -> index in students_
    std::unordered_map<std::string, SeatLocation> seatOf_;  // regNo -> seat (only seated students)
    StudentQueue waiting_;                                  // students without a seat
    SeatingRules rules_;

    bool findSeatFor(const Student& s, SeatLocation& loc) const;  // first-fit scan of all halls
    void removeFromWaiting(const std::string& reg);
    bool isWaiting(const std::string& reg) const;
    void rebuildIndex();
};
