/**
 * @file Student.h
 * @brief Basic student record used by every SeatSync module.
 */
#pragma once
#include <string>

// One student who has registered for the examination.
struct Student {
    std::string regNo;   // unique registration number, e.g. 106125127 
    std::string name;    // full name
    std::string dept;    // department code, e.g. CSE, ECE, MECH
};
