#include "DataLoader.h"
#include "Utils.h"
#include <algorithm>
#include <fstream>
using namespace std;
bool loadStudentsFromCSV(const string &path, SeatAllocator &alloc, LoadSummary &summary)
{
    ifstream file(path);
    if (!file)
        return false;

    string line;
    int lineNo = 0;
    while (getline(file, line))
    {
        lineNo++;
        line = util::trim(line);
        if (line.empty() || line[0] == '#')
            continue; // blank line or comment

        vector<string> fields = util::splitCSV(line);
        if (fields[0].size() && util::toUpper(fields[0]) == "REGNO")
            continue; // header line

        summary.linesRead++;
        if (fields.size() != 3 || fields[0].empty() || fields[1].empty() || fields[2].empty())
        {
            summary.invalid++;
            summary.messages.push_back("Line " + to_string(lineNo) + ": expected 'RegNo,Name,Dept' -> skipped");
            continue;
        }

        Student s{fields[0], fields[1], fields[2]};
        string err;
        RegisterResult result = alloc.registerStudent(s, err);
        if (result == REG_OK)
        {
            summary.loaded++;
        }
        else if (result == REG_DUPLICATE)
        {
            summary.duplicates++;
            summary.messages.push_back("Line " + to_string(lineNo) + ": " + err + " -> skipped");
        }
        else
        {
            summary.invalid++;
            summary.messages.push_back("Line " + to_string(lineNo) + ": " + err + " -> skipped");
        }
    }
    return true;
}

int generateDemoStudents(SeatAllocator &alloc, int count)
{
    // Department pattern repeats every 10 students; CSE is deliberately the biggest group.
    const char *depts[10] = {"CSE", "ECE", "CSE", "EEE", "MECH", "CSE", "CIVIL", "ECE", "EEE", "MECH"};
    const char *first[10] = {"Arun", "Priya", "Karthik", "Divya", "Rahul", "Meena", "Suresh", "Lakshmi", "Vikram", "Anitha"};
    const char *last[6] = {"Kumar", "Raj", "Nair", "Iyer", "Reddy", "Sharma"};

    int added = 0;
    long serial = 1; // registration numbers 106125001, 106125002, ...
    while (added < count)
    {
        string num =to_string(serial);
        string reg = "106125" + string(3 - min<size_t>(3, num.size()), '0') + num;
        Student s{reg, string(first[serial % 10]) + " " + last[(serial / 3) % 6], depts[serial % 10]};
        string err;
        if (alloc.registerStudent(s, err) == REG_OK)
            added++;
        serial++;
        if (serial > 100000)
            break; // safety stop
    }
    return added;
}
