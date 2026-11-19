/**
 * @file DataLoader.h
 * @brief Brings students into the system: from a CSV file or as generated demo data.
 *
 * CSV format (header line optional, '#' starts a comment):
 *     RegNo,Name,Dept
 *     106125127,S Vijayakrishna,CSE
 */
#pragma once
#include "SeatAllocator.h"
#include <string>
#include <vector>

struct LoadSummary {
    int linesRead = 0;                    // non-empty data lines examined
    int loaded = 0;                       // students successfully registered
    int duplicates = 0;                   // skipped: registration number already present
    int invalid = 0;                      // skipped: malformed line
    std::vector<std::string> messages;    // one message per skipped line
};

// Returns false only if the file cannot be opened.
bool loadStudentsFromCSV(const std::string& path, SeatAllocator& alloc, LoadSummary& summary);

// Registers `count` students with deterministic names/departments. Returns how many were added.
int generateDemoStudents(SeatAllocator& alloc, int count);
