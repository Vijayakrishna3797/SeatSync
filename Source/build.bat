@echo off
setlocal

rem Build SeatSync from this Source folder. Headers live in the sibling include folder.
set "SOURCE_DIR=%~dp0"
set "PROJECT_DIR=%SOURCE_DIR%.."

g++ -std=c++17 -Wall -Wextra -pedantic -I"%PROJECT_DIR%\include" ^
  "%SOURCE_DIR%main.cpp" ^
  "%SOURCE_DIR%Hall.cpp" ^
  "%SOURCE_DIR%StudentQueue.cpp" ^
  "%SOURCE_DIR%SeatAllocator.cpp" ^
  "%SOURCE_DIR%DataLoader.cpp" ^
  "%SOURCE_DIR%ReportGenerator.cpp" ^
  "%SOURCE_DIR%Utils.cpp" ^
  -o "%SOURCE_DIR%seatsync.exe"

if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

echo Build succeeded: "%SOURCE_DIR%seatsync.exe"
endlocal
