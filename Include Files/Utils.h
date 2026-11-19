/**
 * @file Utils.h
 * @brief Small string helpers shared by the SeatSync modules.
 */
#pragma once
#include <string>
#include <vector>

namespace util {

std::string trim(const std::string& s);                 // removes leading/trailing whitespace
std::string toUpper(std::string s);                     // upper-cases a copy of s
bool isAlphaNumeric(const std::string& s);              // true if non-empty and only letters/digits
std::vector<std::string> splitCSV(const std::string& line);   // splits on ',' and trims each field
std::string lastChars(const std::string& s, std::size_t n);   // last n characters (or all of s)
std::string sanitizeFileName(const std::string& s);     // replaces unsafe characters with '_'
std::string padRight(const std::string& s, std::size_t width); // pads / truncates to exactly width
std::string center(const std::string& s, std::size_t width);   // centres s inside width

}  // namespace util
