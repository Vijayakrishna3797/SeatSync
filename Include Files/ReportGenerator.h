/**
 * @file ReportGenerator.h
 * @brief Text output: hall layouts, student seat lookup, attendance sheets,
 *        examination report and seating-plan export.
 *
 * All printing functions take an std::ostream so the same code writes to the
 * console and to files.
 */
#pragma once
#include "SeatAllocator.h"
#include <iosfwd>
#include <string>

namespace report {

void printHallLayout(const Hall& hall, std::ostream& os);                          // seat matrix picture
void printStudentSeat(const SeatAllocator& a, const std::string& reg, std::ostream& os);
void printWaitingList(const SeatAllocator& a, std::ostream& os);
void printAttendanceSheet(const SeatAllocator& a, int hallIdx, std::ostream& os);
void printExamReport(const SeatAllocator& a, std::ostream& os);

// Writes one attendance sheet per hall into dir and echoes it to the console.
// Returns the number of files written.
int writeAttendanceSheets(const SeatAllocator& a, const std::string& dir, std::ostream& console);

// Writes seating_plan.csv and seating_plan.txt into dir. Returns true on success.
bool exportSeatingPlan(const SeatAllocator& a, const std::string& dir, std::ostream& console);

}  // namespace report
