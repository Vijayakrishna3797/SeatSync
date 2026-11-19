@echo off
setlocal
call "%~dp0build.bat"
if errorlevel 1 exit /b 1

rem Run from the project root so relative data/ and output/ paths resolve correctly.
set "PROJECT_DIR=%~dp0.."
pushd "%PROJECT_DIR%"
"%~dp0seatsync.exe"
set "RUN_RESULT=%ERRORLEVEL%"
popd
exit /b %RUN_RESULT%
