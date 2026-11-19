#include "Utils.h"
#include <algorithm>
#include <cctype>

namespace util {

std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

std::string toUpper(std::string s) {
    for (char& ch : s) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    return s;
}

bool isAlphaNumeric(const std::string& s) {
    if (s.empty()) return false;
    for (char ch : s)
        if (!std::isalnum(static_cast<unsigned char>(ch))) return false;
    return true;
}

std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    for (char ch : line) {
        if (ch == ',') {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.push_back(trim(current));
    return fields;
}

std::string lastChars(const std::string& s, std::size_t n) {
    if (s.size() <= n) return s;
    return s.substr(s.size() - n);
}

std::string sanitizeFileName(const std::string& s) {
    std::string out = s;
    for (char& ch : out)
        if (!std::isalnum(static_cast<unsigned char>(ch))) ch = '_';
    return out;
}

std::string padRight(const std::string& s, std::size_t width) {
    if (s.size() >= width) return s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
}

std::string center(const std::string& s, std::size_t width) {
    if (s.size() >= width) return s.substr(0, width);
    std::size_t left = (width - s.size()) / 2;
    return std::string(left, ' ') + s + std::string(width - s.size() - left, ' ');
}

}  // namespace util
