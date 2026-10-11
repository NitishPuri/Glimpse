@echo off
echo Building the project...
cmake --build build/windows --config Release --target ALL_BUILD
if %errorlevel% neq 0 exit /b %errorlevel%
echo Running the project...
build\windows\Release\Glimpse_cli.exe %*
pause