# SeatSync - Test Cases and Test Data

All test cases are **scripted**: the file `tests/inputs/<name>.in` holds exactly what a user would type, and `tests/expected/<name>.out` holds the expected console output. `make test` (or `sh tests/run_tests.sh`) replays every script and compares the result; `make test` also runs 91 unit tests (`tests/test_core.cpp`) covering the queue, the seat matrix, the allocator, the CSV loader and the helper functions.

Seat allocation is fully deterministic (no random numbers), so the outputs below are reproducible. The repeated main menu is removed from the transcripts below to keep them readable.

## Test data files (`data/`)

| File | Content |
|---|---|
| `students_sample.csv` | 15 students, 5 departments |
| `students_overflow.csv` | 25 students, 5 departments (more than two small halls can hold) |
| `students_dominant.csv` | 12 students: 9 CSE + 3 ECE (stress test for the spacing rule) |
| `students_errors.csv` | deliberately faulty lines: duplicate, missing field, bad number, empty name |

## Summary

| # | Test case | Result |
|---|---|---|
| 1 | Basic allocation, seat lookup | PASS |
| 2 | More students than seats, several halls | PASS |
| 3 | One department dominates; effect of the diagonal rule | PASS |
| 4 | Administrator modifications | PASS |
| 5 | CSV loading and manual registration (error handling) | PASS |
| 6 | Input validation and edge cases | PASS |
| 7 | Reports and file export | PASS |

Unit tests: 91 passed, 0 failed. Scripted tests: 7 passed, 0 failed.

---

## TESTCASE 1 - Basic allocation, seat lookup

**Setup:** Hall-A 4x5 (20 seats). 15 students loaded from data/students_sample.csv (CSE, ECE, EEE, MECH, CIVIL). Allocate, view the seat matrix, look up one student, look up an unknown student.

**Expected result:** All 15 students are seated, 0 rule conflicts. Unknown registration number is reported politely.

**Input script:** `tests/inputs/tc1_basic_allocation.in`

**Console transcript:**

```
Enter your choice: 1

Hall name (e.g. Hall-A): Hall-A
Number of rows (1-26): 4
Seats per row (1-30): 5
Hall 'Hall-A' added with 20 seats.

Enter your choice: 3

CSV file path (e.g. data/students_sample.csv): data/students_sample.csv
Lines read: 15 | Loaded: 15 | Duplicates: 0 | Invalid: 0

Enter your choice: 5

Allocation complete: 15 student(s) seated, 0 still waiting.
Rule check: 0 conflict(s) found.

Enter your choice: 6

  1. Hall-A (4x5)
Select hall number: 1

Hall: Hall-A  (4 rows x 5 columns)
Occupied: 15/20   Blocked seats: 0

         1         2         3         4         5     
  A  [CSE-001 ][ECE-002 ][EEE-004 ][MECH-005][CIVI-007]
  B  [ECE-008 ][CSE-003 ][MECH-010][EEE-009 ][CSE-006 ]
  C  [EEE-014 ][ECE-012 ][CSE-011 ][MECH-015][  ---   ]
  D  [CSE-013 ][  ---   ][  ---   ][  ---   ][  ---   ]

Legend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked

Enter your choice: 7

Enter registration number: 106125003

Name       : Karthik Das
Reg. No.   : 106125003
Department : CSE
Hall       : Hall-A
Seat       : B2  (row B, column 2)

Enter your choice: 7

Enter registration number: 106125999

No student is registered with number 106125999.

Enter your choice: 15

==================================================
          EXAMINATION SEATING REPORT
==================================================
Seating rule        : No two students of the same department may sit next to each other (front, back, left, right).
Halls               : 1
Students registered : 15
Students seated     : 15 (100.0%)
Students waiting    : 0
Rule conflicts      : 0  (layout is valid)

Hall-wise utilisation
Hall          Layout   Usable  Seated  Blocked  Utilisation
--------------------------------------------------------
Hall-A        4x5      20      15      0        75.0%
--------------------------------------------------------
TOTAL                  20      15               75.0%

Department-wise distribution
Dept    Registered  Seated  Waiting
-----------------------------------
CIVIL   1           1       0
CSE     5           5       0
ECE     3           3       0
EEE     3           3       0
MECH    3           3       0

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```

---

## TESTCASE 2 - More students than seats, several halls

**Setup:** Hall-A 3x4 (12) + Hall-B 3x3 (9) = 21 seats, 25 students from data/students_overflow.csv. After the first run 4 students wait. Hall-C (3x4) is added and allocation is run again.

**Expected result:** 21 seated, 4 waiting (queue order shown), 0 conflicts. After adding Hall-C the 4 waiting students are seated; everybody has a seat and utilisation is reported per hall.

**Input script:** `tests/inputs/tc2_overflow_multiple_halls.in`

**Console transcript:**

```
Enter your choice: 1

Hall name (e.g. Hall-A): Hall-A
Number of rows (1-26): 3
Seats per row (1-30): 4
Hall 'Hall-A' added with 12 seats.

Enter your choice: 1

Hall name (e.g. Hall-A): Hall-B
Number of rows (1-26): 3
Seats per row (1-30): 3
Hall 'Hall-B' added with 9 seats.

Enter your choice: 3

CSV file path (e.g. data/students_sample.csv): data/students_overflow.csv
Lines read: 25 | Loaded: 25 | Duplicates: 0 | Invalid: 0

Enter your choice: 5

Allocation complete: 21 student(s) seated, 4 still waiting.
Rule check: 0 conflict(s) found.
Some students could not be placed legally (not enough seats, or spacing rule too strict).
Add another hall or unblock seats, then run allocation again (option 5).

Enter your choice: 17

Students waiting for a seat (4), in queue order:
  106125020   Rahul Reddy             CSE
  106125021   Meena Kumar             ECE
  106125022   Suresh Iyer             CSE
  106125023   Lakshmi Das             ECE

Enter your choice: 15

==================================================
          EXAMINATION SEATING REPORT
==================================================
Seating rule        : No two students of the same department may sit next to each other (front, back, left, right).
Halls               : 2
Students registered : 25
Students seated     : 21 (84.0%)
Students waiting    : 4
Rule conflicts      : 0  (layout is valid)

Hall-wise utilisation
Hall          Layout   Usable  Seated  Blocked  Utilisation
--------------------------------------------------------
Hall-A        3x4      12      12      0        100.0%
Hall-B        3x3      9       9       0        100.0%
--------------------------------------------------------
TOTAL                  21      21               100.0%

Department-wise distribution
Dept    Registered  Seated  Waiting
-----------------------------------
CIVIL   3           3       0
CSE     7           5       2
ECE     7           5       2
EEE     4           4       0
MECH    4           4       0

Enter your choice: 1

Hall name (e.g. Hall-A): Hall-C
Number of rows (1-26): 3
Seats per row (1-30): 4
Hall 'Hall-C' added with 12 seats.

Enter your choice: 5

Allocation complete: 4 student(s) seated, 0 still waiting.
Rule check: 0 conflict(s) found.

Enter your choice: 17

No students are waiting. Everybody registered has a seat.

Enter your choice: 6

  1. Hall-A (3x4)
  2. Hall-B (3x3)
  3. Hall-C (3x4)
Select hall number: 3

Hall: Hall-C  (3 rows x 4 columns)
Occupied: 4/12   Blocked seats: 0

         1         2         3         4     
  A  [CSE-020 ][ECE-021 ][CSE-022 ][ECE-023 ]
  B  [  ---   ][  ---   ][  ---   ][  ---   ]
  C  [  ---   ][  ---   ][  ---   ][  ---   ]

Legend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked

Enter your choice: 15

==================================================
          EXAMINATION SEATING REPORT
==================================================
Seating rule        : No two students of the same department may sit next to each other (front, back, left, right).
Halls               : 3
Students registered : 25
Students seated     : 25 (100.0%)
Students waiting    : 0
Rule conflicts      : 0  (layout is valid)

Hall-wise utilisation
Hall          Layout   Usable  Seated  Blocked  Utilisation
--------------------------------------------------------
Hall-A        3x4      12      12      0        100.0%
Hall-B        3x3      9       9       0        100.0%
Hall-C        3x4      12      4       0        33.3%
--------------------------------------------------------
TOTAL                  33      25               75.8%

Department-wise distribution
Dept    Registered  Seated  Waiting
-----------------------------------
CIVIL   3           3       0
CSE     7           7       0
ECE     7           7       0
EEE     4           4       0
MECH    4           4       0

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```

---

## TESTCASE 3 - One department dominates; effect of the diagonal rule

**Setup:** Hall-A 3x4 (12 seats), 12 students from data/students_dominant.csv: 9 CSE + 3 ECE. Normal rule first, then all seats are cleared, the diagonal rule is switched on and allocation is repeated.

**Expected result:** Normal rule: 9 seated, 3 CSE students wait (not enough room to keep CSE students apart). Diagonal rule: stricter, so only 7 seated and 5 wait. Rule check always shows 0 conflicts.

**Input script:** `tests/inputs/tc3_dominant_department.in`

**Console transcript:**

```
Enter your choice: 1

Hall name (e.g. Hall-A): Hall-A
Number of rows (1-26): 3
Seats per row (1-30): 4
Hall 'Hall-A' added with 12 seats.

Enter your choice: 3

CSV file path (e.g. data/students_sample.csv): data/students_dominant.csv
Lines read: 12 | Loaded: 12 | Duplicates: 0 | Invalid: 0

Enter your choice: 5

Allocation complete: 9 student(s) seated, 3 still waiting.
Rule check: 0 conflict(s) found.
Some students could not be placed legally (not enough seats, or spacing rule too strict).
Add another hall or unblock seats, then run allocation again (option 5).

Enter your choice: 6

  1. Hall-A (3x4)
Select hall number: 1

Hall: Hall-A  (3 rows x 4 columns)
Occupied: 9/12   Blocked seats: 0

         1         2         3         4     
  A  [CSE-001 ][ECE-004 ][CSE-002 ][ECE-008 ]
  B  [ECE-012 ][CSE-003 ][  ---   ][CSE-005 ]
  C  [CSE-006 ][  ---   ][CSE-007 ][  ---   ]

Legend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked

Enter your choice: 17

Students waiting for a seat (3), in queue order:
  106125009   Vikram Raj              CSE
  106125010   Anitha Reddy            CSE
  106125011   Naveen Kumar            CSE

Enter your choice: 13

This empties every seat. Type 'yes' to confirm: yes
All seats cleared. 12 student(s) are waiting for allocation.

Enter your choice: 12

Diagonal spacing rule is now ON.
No two students of the same department may sit next to each other (front, back, left, right AND diagonals).
(Applies to future placements. Use option 13 then option 5 to re-allocate everyone.)

Enter your choice: 5

Allocation complete: 7 student(s) seated, 5 still waiting.
Rule check: 0 conflict(s) found.
Some students could not be placed legally (not enough seats, or spacing rule too strict).
Add another hall or unblock seats, then run allocation again (option 5).

Enter your choice: 6

  1. Hall-A (3x4)
Select hall number: 1

Hall: Hall-A  (3 rows x 4 columns)
Occupied: 7/12   Blocked seats: 0

         1         2         3         4     
  A  [CSE-001 ][ECE-004 ][CSE-002 ][ECE-008 ]
  B  [  ---   ][  ---   ][  ---   ][  ---   ]
  C  [CSE-003 ][ECE-012 ][CSE-005 ][  ---   ]

Legend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked

Enter your choice: 17

Students waiting for a seat (5), in queue order:
  106125006   Meena Kumar             CSE
  106125007   Suresh Iyer             CSE
  106125009   Vikram Raj              CSE
  106125010   Anitha Reddy            CSE
  106125011   Naveen Kumar            CSE

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```

---

## TESTCASE 4 - Administrator modifications

**Setup:** Hall-A 4x5 with the 15 sample students. Valid swap; invalid swap (rejected, nothing changes); move into a seat next to a same-department student (rejected); move into an occupied seat (rejected); valid move; block an occupied seat (student re-seated automatically); unblock; remove a student.

**Expected result:** Every change keeps the layout legal. Rejections state the exact reason. Final examination report shows 14 students and 0 conflicts.

**Input script:** `tests/inputs/tc4_admin_changes.in`

**Console transcript:**

```
Enter your choice: 1

Hall name (e.g. Hall-A): Hall-A
Number of rows (1-26): 4
Seats per row (1-30): 5
Hall 'Hall-A' added with 20 seats.

Enter your choice: 3

CSV file path (e.g. data/students_sample.csv): data/students_sample.csv
Lines read: 15 | Loaded: 15 | Duplicates: 0 | Invalid: 0

Enter your choice: 5

Allocation complete: 15 student(s) seated, 0 still waiting.
Rule check: 0 conflict(s) found.

Enter your choice: 6

  1. Hall-A (4x5)
Select hall number: 1

Hall: Hall-A  (4 rows x 5 columns)
Occupied: 15/20   Blocked seats: 0

         1         2         3         4         5     
  A  [CSE-001 ][ECE-002 ][EEE-004 ][MECH-005][CIVI-007]
  B  [ECE-008 ][CSE-003 ][MECH-010][EEE-009 ][CSE-006 ]
  C  [EEE-014 ][ECE-012 ][CSE-011 ][MECH-015][  ---   ]
  D  [CSE-013 ][  ---   ][  ---   ][  ---   ][  ---   ]

Legend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked

Enter your choice: 8

First student's registration number: 106125007
Second student's registration number: 106125014
Seats swapped successfully.

Enter your choice: 8

First student's registration number: 106125002
Second student's registration number: 106125009
Swap rejected: 106125009 cannot take the seat of 106125002: Neighbouring seat A3 already has a EEE student (106125004).

Enter your choice: 9

Student's registration number: 106125001
  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): D2
Move rejected: Neighbouring seat D1 already has a CSE student (106125013).

Enter your choice: 9

Student's registration number: 106125001
  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): B1
Move rejected: Seat B1 is already occupied by 106125008.

Enter your choice: 9

Student's registration number: 106125013
  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): D4
Student moved to Hall-A, seat D4.

Enter your choice: 10

  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): B2
1 = Block seat (broken/reserved), 2 = Unblock seat: 1
Seat B2 is now blocked.
Student 106125003 was sitting there. Re-seated automatically at Hall-A, seat D1.

Enter your choice: 10

  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): B2
1 = Block seat (broken/reserved), 2 = Unblock seat: 2
Seat B2 is available again.

Enter your choice: 11

Registration number to remove: 106125015
Student removed; their seat (if any) is free again.

Enter your choice: 6

  1. Hall-A (4x5)
Select hall number: 1

Hall: Hall-A  (4 rows x 5 columns)
Occupied: 14/20   Blocked seats: 0

         1         2         3         4         5     
  A  [CSE-001 ][ECE-002 ][EEE-004 ][MECH-005][EEE-014 ]
  B  [ECE-008 ][  ---   ][MECH-010][EEE-009 ][CSE-006 ]
  C  [CIVI-007][ECE-012 ][CSE-011 ][  ---   ][  ---   ]
  D  [CSE-003 ][  ---   ][  ---   ][CSE-013 ][  ---   ]

Legend: [DEPT-nnn] = department + last 3 digits of reg no, [---] = free, [XXX] = blocked

Enter your choice: 15

==================================================
          EXAMINATION SEATING REPORT
==================================================
Seating rule        : No two students of the same department may sit next to each other (front, back, left, right).
Halls               : 1
Students registered : 14
Students seated     : 14 (100.0%)
Students waiting    : 0
Rule conflicts      : 0  (layout is valid)

Hall-wise utilisation
Hall          Layout   Usable  Seated  Blocked  Utilisation
--------------------------------------------------------
Hall-A        4x5      20      14      0        70.0%
--------------------------------------------------------
TOTAL                  20      14               70.0%

Department-wise distribution
Dept    Registered  Seated  Waiting
-----------------------------------
CIVIL   1           1       0
CSE     5           5       0
ECE     3           3       0
EEE     3           3       0
MECH    2           2       0

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```

---

## TESTCASE 5 - CSV loading and manual registration (error handling)

**Setup:** data/students_errors.csv contains a duplicate registration number, a line with a missing field, a bad registration number and an empty name. Also: a non-existent file, a valid manual registration (lower-case department is normalised), a duplicate, an invalid number and an empty name; then allocation without any hall.

**Expected result:** 3 valid students loaded, 1 duplicate and 3 invalid lines skipped with line numbers. Manual registration validates every field. Allocation without halls is refused with a hint.

**Input script:** `tests/inputs/tc5_csv_and_registration.in`

**Console transcript:**

```
Enter your choice: 3

CSV file path (e.g. data/students_sample.csv): data/students_errors.csv
Lines read: 7 | Loaded: 3 | Duplicates: 1 | Invalid: 3
  Line 5: Registration number 106125001 is already registered. -> skipped
  Line 6: expected 'RegNo,Name,Dept' -> skipped
  Line 8: Registration number must be letters/digits only. -> skipped
  Line 9: expected 'RegNo,Name,Dept' -> skipped

Enter your choice: 3

CSV file path (e.g. data/students_sample.csv): data/nonexistent.csv
Could not open file 'data/nonexistent.csv'.

Enter your choice: 2

Registration number: 106125010
Name: Test Student
Department (e.g. CSE): cse
Student registered and added to the allocation queue.

Enter your choice: 2

Registration number: 106125010
Name: Another Person
Department (e.g. CSE): ece
Could not register student: Registration number 106125010 is already registered.

Enter your choice: 2

Registration number: bad id!
Name: Some Name
Department (e.g. CSE): CSE
Could not register student: Registration number must be letters/digits only.

Enter your choice: 2

Registration number: 106125011
Name: 
Department (e.g. CSE): CSE
Could not register student: Name cannot be empty.

Enter your choice: 17

Students waiting for a seat (4), in queue order:
  106125001   Arun Kumar              CSE
  106125002   Priya Raj               ECE
  106125006   Rahul Reddy             CSE
  106125010   Test Student            CSE

Enter your choice: 5

Add at least one hall first (option 1).

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```

---

## TESTCASE 6 - Input validation and edge cases

**Setup:** Menu choices 'abc', 99 and 0; using halls before any exist; empty hall name; 30 rows (too many); duplicate hall name in a different case; invalid seat label Z9; blocking an already-blocked seat; unblocking a free seat; swapping students who are not seated; attendance sheets with nobody seated; cancelling 'clear all'.

**Expected result:** The program never crashes: every wrong input gets a clear message and the menu continues.

**Input script:** `tests/inputs/tc6_input_validation.in`

**Console transcript:**

```
Enter your choice: abc
  Please enter a whole number between 1 and 18.
Enter your choice: 99
  Please enter a whole number between 1 and 18.
Enter your choice: 0
  Please enter a whole number between 1 and 18.
Enter your choice: 6

No halls available yet. Add a hall first (option 1).

Enter your choice: 7

Enter registration number: 106125001

No student is registered with number 106125001.

Enter your choice: 1

Hall name (e.g. Hall-A): 
Number of rows (1-26): 2
Seats per row (1-30): 2
Could not add hall: Hall name cannot be empty.

Enter your choice: 1

Hall name (e.g. Hall-A): Hall-A
Number of rows (1-26): 30
  Please enter a whole number between 1 and 26.
Number of rows (1-26): 4
Seats per row (1-30): 5
Hall 'Hall-A' added with 20 seats.

Enter your choice: 1

Hall name (e.g. Hall-A): hall-a
Number of rows (1-26): 2
Seats per row (1-30): 2
Could not add hall: A hall named 'Hall-A' already exists.

Enter your choice: 10

  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): Z9
  Invalid seat. Rows are A-D, columns 1-5.
Enter seat (e.g. B4): A1
1 = Block seat (broken/reserved), 2 = Unblock seat: 1
Seat A1 is now blocked.

Enter your choice: 10

  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): A1
1 = Block seat (broken/reserved), 2 = Unblock seat: 1
Could not block seat: Seat A1 is already blocked.

Enter your choice: 10

  1. Hall-A (4x5)
Select hall number: 1
Enter seat (e.g. B4): B2
1 = Block seat (broken/reserved), 2 = Unblock seat: 2
Could not unblock seat: Seat B2 is not blocked.

Enter your choice: 8

First student's registration number: 106125001
Second student's registration number: 106125002
Swap rejected: 106125001 is not seated.

Enter your choice: 14

No seated students yet, so there are no attendance sheets.

Enter your choice: 13

This empties every seat. Type 'yes' to confirm: no
Cancelled.

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```

---

## TESTCASE 7 - Reports and file export

**Setup:** Hall-A 4x5 + Hall-B 3x3, 20 generated demo students. Allocate, generate attendance sheets (files written to output/), export the seating plan (CSV + TXT), print the examination report.

**Expected result:** 19 students in Hall-A, 1 in Hall-B, 0 conflicts. Files created: output/attendance_Hall_A.txt, output/attendance_Hall_B.txt, output/seating_plan.csv, output/seating_plan.txt.

**Input script:** `tests/inputs/tc7_reports_and_export.in`

**Console transcript:**

```
Enter your choice: 1

Hall name (e.g. Hall-A): Hall-A
Number of rows (1-26): 4
Seats per row (1-30): 5
Hall 'Hall-A' added with 20 seats.

Enter your choice: 1

Hall name (e.g. Hall-A): Hall-B
Number of rows (1-26): 3
Seats per row (1-30): 3
Hall 'Hall-B' added with 9 seats.

Enter your choice: 4

How many demo students (1-500)? 20
20 demo students registered.

Enter your choice: 5

Allocation complete: 20 student(s) seated, 0 still waiting.
Rule check: 0 conflict(s) found.

Enter your choice: 14

==================================================================================
                         ATTENDANCE SHEET  -  Hall-A
==================================================================================
S.No  Reg. No.     Name                     Dept   Seat  Signature
----------------------------------------------------------------------------------
1     106125001    Priya Kumar              ECE    A1    ____________
2     106125002    Karthik Kumar            CSE    A2    ____________
3     106125003    Divya Raj                EEE    A3    ____________
4     106125004    Rahul Raj                MECH   A4    ____________
5     106125005    Meena Raj                CSE    B1    ____________
6     106125006    Suresh Nair              CIVIL  A5    ____________
7     106125007    Lakshmi Nair             ECE    B2    ____________
8     106125008    Vikram Nair              EEE    B4    ____________
9     106125009    Anitha Iyer              MECH   B3    ____________
10    106125010    Arun Iyer                CSE    C2    ____________
11    106125011    Priya Iyer               ECE    B5    ____________
12    106125012    Karthik Reddy            CSE    C5    ____________
13    106125013    Divya Reddy              EEE    C3    ____________
14    106125014    Rahul Reddy              MECH   C4    ____________
15    106125015    Meena Sharma             CSE    D4    ____________
16    106125016    Suresh Sharma            CIVIL  C1    ____________
17    106125017    Lakshmi Sharma           ECE    D1    ____________
18    106125018    Vikram Kumar             EEE    D2    ____________
19    106125019    Anitha Kumar             MECH   D3    ____________
----------------------------------------------------------------------------------
Total present on sheet: 19
Saved: output/attendance_Hall_A.txt

==================================================================================
                         ATTENDANCE SHEET  -  Hall-B
==================================================================================
S.No  Reg. No.     Name                     Dept   Seat  Signature
----------------------------------------------------------------------------------
1     106125020    Arun Kumar               CSE    A1    ____________
----------------------------------------------------------------------------------
Total present on sheet: 1
Saved: output/attendance_Hall_B.txt

Enter your choice: 16

Saved: output/seating_plan.csv
Saved: output/seating_plan.txt

Enter your choice: 15

==================================================
          EXAMINATION SEATING REPORT
==================================================
Seating rule        : No two students of the same department may sit next to each other (front, back, left, right).
Halls               : 2
Students registered : 20
Students seated     : 20 (100.0%)
Students waiting    : 0
Rule conflicts      : 0  (layout is valid)

Hall-wise utilisation
Hall          Layout   Usable  Seated  Blocked  Utilisation
--------------------------------------------------------
Hall-A        4x5      20      19      0        95.0%
Hall-B        3x3      9       1       0        11.1%
--------------------------------------------------------
TOTAL                  29      20               69.0%

Department-wise distribution
Dept    Registered  Seated  Waiting
-----------------------------------
CIVIL   2           2       0
CSE     6           6       0
ECE     4           4       0
EEE     4           4       0
MECH    4           4       0

Enter your choice: 18

Exiting SeatSync. Good luck to all the students!
```
