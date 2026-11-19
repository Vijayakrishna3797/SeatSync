# SeatSync - Online Exam Seating Allocation System

**Team Data Sphere** | CSLR31 Data Structures Laboratory | NIT Tiruchirappalli
S Vijayakrishna (106125127) | Tanishaa E A (106125129) | Torim Chakma (106125131) | Vadlakonda Sathwika (106125133)

**SDG alignment:** SDG 4 - Quality Education (main), SDG 9 - Industry, Innovation and Infrastructure (secondary)

SeatSync is a C++ console application that automatically assigns examination seats to students.
An administrator defines the exam halls (rows x seats), registers students (manually, from a CSV
file, or as demo data) and runs the allocator. The program seats every student so that
**no two students of the same department sit next to each other**, then lets the administrator
inspect, modify and print the result (attendance sheets, examination report, seating plan).

## Data structures used
| Data structure | Where | Why |
|---|---|---|
| **Queue** (own linked-list implementation, `StudentQueue`) | students waiting for a seat; per-department queues | fair first-come-first-served processing |
| **Matrix / 2D array** (`Hall`) | physical seat layout of each hall | neighbours of a seat are simply (r+-1, c+-1), so spacing rules are O(1) |
| **Hash table** (`unordered_map`) | registration number -> student, registration number -> seat | instant "find my seat" and duplicate detection |

## Features
- Automatic seating plan with a spacing rule (optional stricter rule that also forbids diagonal neighbours)
- Several halls of different sizes; students overflow into the next hall automatically
- Students who cannot be placed legally stay in the waiting queue and are seated when a hall is added
- Administrator tools: swap two students, move a student, block/unblock a broken seat (displaced student is re-seated automatically), remove a student
- "Find a student's seat" lookup by registration number
- Reports: seat-matrix picture, per-hall attendance sheets, examination report (utilisation, department distribution, rule check), CSV/TXT seating plan export

## Build and run (Linux / macOS / Windows with g++ and make)
```
make            # builds ./seatsync (Windows: mingw32-make builds seatsync.exe)
make run        # start the program
make test       # 91 unit tests + 7 scripted test cases
make clean
```
Run commands from the project root. Without make: `g++ -std=c++17 -Iinclude Source/*.cpp -o seatsync`.

## Quick start (about one minute)
```
1  ->  Hall-A, 4 rows, 5 seats per row          (add a hall)
3  ->  data/students_sample.csv                 (load 15 students)
5                                               (run the allocation)
6  ->  1                                        (see the seat matrix of Hall-A)
7  ->  106125003                                (find one student's seat)
15                                              (examination report)
```

## Menu
1 Add hall | 2 Register student | 3 Load CSV | 4 Generate demo students | 5 Run allocation |
6 View hall layout | 7 Find a student's seat | 8 Swap seats | 9 Move student | 10 Block/unblock seat |
11 Remove student | 12 Toggle diagonal rule | 13 Clear all allocations | 14 Attendance sheets |
15 Examination report | 16 Export seating plan | 17 List unseated students | 18 Exit

## CSV format
```
RegNo,Name,Dept
106125127,S Vijayakrishna,CSE
```
Header line optional, `#` starts a comment, blank lines ignored. Duplicate, malformed or
invalid lines are skipped and reported. Output files are written to `output/`.

## Project layout
```
Makefile  seatsync  README.md
include/   *.h   (declarations)      Source/  *.cpp (definitions + main.cpp)
object/    *.o   (build output)      data/    sample CSV files
tests/     unit tests, scripted test inputs and expected outputs
Document/  report.tex/.pdf, UserManual.tex/.pdf, testcases.md, testlogs/
Video/     project experience video
```

See the root `README.md` for the canonical build instructions.
