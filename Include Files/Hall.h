/**
 * @file Hall.h
 * @brief Examination hall modelled as a 2D matrix of seats.
 *
 * Each cell of the matrix is a Seat. The matrix makes the spacing rules easy:
 * the neighbours of seat (r, c) are simply (r+-1, c+-1).
 */
#pragma once
#include <string>
#include <vector>

enum SeatState { SEAT_FREE, SEAT_OCCUPIED, SEAT_BLOCKED };

struct Seat {
    SeatState state = SEAT_FREE;
    std::string regNo;   // occupant (only valid when state == SEAT_OCCUPIED)
    std::string dept;    // occupant's department
};

// Seating rules enforced when a student is placed.
struct SeatingRules {
    // Rule 1 (always on): no two students of the same department may sit
    //                     directly in front/behind/left/right of each other.
    // Rule 2 (optional) : the same restriction also applies to the 4 diagonals.
    bool checkDiagonals = false;
};

class Hall {
public:
    static constexpr int MAX_ROWS = 26;   // rows are labelled A..Z
    static constexpr int MAX_COLS = 30;

    Hall(const std::string& name, int rows, int cols);

    // ---- basic information ----
    const std::string& name() const;
    int rows() const;
    int cols() const;
    bool inBounds(int r, int c) const;
    const Seat& seatAt(int r, int c) const;
    int capacity() const;        // usable seats (not blocked)
    int occupiedCount() const;
    int blockedCount() const;
    int freeCount() const;

    // ---- seat labels: (0,3) <-> "A4" ----
    std::string seatLabel(int r, int c) const;
    static bool parseSeatLabel(const std::string& label, int rows, int cols, int& r, int& c);

    // ---- placement ----
    // Returns "" if the student may sit at (r, c), otherwise a human-readable reason.
    std::string placementProblem(int r, int c, const std::string& dept, const SeatingRules& rules) const;
    bool canPlace(int r, int c, const std::string& dept, const SeatingRules& rules) const;
    bool place(int r, int c, const std::string& regNo, const std::string& dept, const SeatingRules& rules);
    void restore(int r, int c, const std::string& regNo, const std::string& dept);  // unchecked (undo helper)
    bool vacate(int r, int c);
    int vacateAll();                       // empties every occupied seat, returns how many
    bool setBlocked(int r, int c);         // only works on a FREE seat
    bool setUnblocked(int r, int c);       // only works on a BLOCKED seat

    // Number of adjacent same-department pairs (0 means the layout obeys the rules).
    int countConflicts(const SeatingRules& rules) const;

private:
    std::string name_;
    int rows_;
    int cols_;
    std::vector<std::vector<Seat>> grid_;   // the seat matrix
};
