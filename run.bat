@echo off
setlocal
cd /d "%~dp0"

if "%~1"=="" goto fast_run
if /i "%~1"=="help" goto usage
if /i "%~1"=="genInst" goto gen_inst
if /i "%~1"=="cu_unicode" goto cu_unicode
if /i "%~1"=="compile_c" goto compile_c
if /i "%~1"=="assembly_c" goto assembly_c
if /i "%~1"=="fast_run" goto fast_run
if /i "%~1"=="clean_run" goto clean_run

echo Unknown command: %~1
goto usage_error

:validate_args
if not "%~2"=="" goto usage_error
if not "%~1"=="" if /i not "%~1"=="verbose" goto usage_error
exit /b 0

:gen_inst
echo Generating instruction table and dictionary...
if not exist .\build mkdir .\build
g++ .\Generator\generator.cpp -std=c++17 -o .\build\generator.exe
if errorlevel 1 exit /b 1
.\build\generator.exe
if errorlevel 1 exit /b 1
exit /b 0

:cu_unicode
echo Generating Control Unit and seven-segment ROMs...
if not exist .\build mkdir .\build
g++ .\Unicode_CU\unicode_cu.cpp -std=c++17 -o .\build\unicode_cu.exe
if errorlevel 1 exit /b 1
.\build\unicode_cu.exe
if errorlevel 1 exit /b 1
exit /b 0

:compile_c
call :validate_args "%~2" "%~3"
if errorlevel 1 exit /b 2
echo Building TOM compiler and compiling Code\program.tom...
if not exist .\build mkdir .\build
g++ .\Compiler\compiler.cpp -std=c++17 -o .\build\compiler.exe
if errorlevel 1 exit /b 1
if /i "%~2"=="verbose" (
	.\build\compiler.exe .\Code\program.tom .\Code\program.ass --debug
) else (
.\build\compiler.exe .\Code\program.tom .\Code\program.ass
)
if errorlevel 1 exit /b 1
exit /b 0

:assembly_c
call :validate_args "%~2" "%~3"
if errorlevel 1 exit /b 2
echo Building assembler and assembling Code\program.ass...
if not exist .\build mkdir .\build
g++ .\Assembler\assembler.cpp -std=c++17 -o .\build\assembler.exe
if errorlevel 1 exit /b 1
if /i "%~2"=="verbose" (
	.\build\assembler.exe .\Code\program.ass --debug
) else (
.\build\assembler.exe .\Code\program.ass
)
if errorlevel 1 exit /b 1
exit /b 0

:fast_run
call :validate_args "%~2" "%~3"
if errorlevel 1 exit /b 2
call :run_existing_tools "%~2"
if errorlevel 1 exit /b 1
exit /b 0

:clean_run
call :validate_args "%~2" "%~3"
if errorlevel 1 exit /b 2
call :run_existing_tools "%~2"
if errorlevel 1 exit /b 1
echo.
echo Removing generated assembly and temporary files...
if exist .\temp.tom del /q .\temp.tom
if exist .\tempf.ass del /q .\tempf.ass
if exist .\Code\program.ass del /q .\Code\program.ass
exit /b 0

:run_existing_tools
if not exist .\build\compiler.exe (
	echo Missing .\build\compiler.exe. Run "run.bat compile_c" first.
	exit /b 1
)
if not exist .\build\assembler.exe (
	echo Missing .\build\assembler.exe. Run "run.bat assembly_c" first.
	exit /b 1
)
echo Generating Assembly code (Compilation)...
if /i "%~1"=="verbose" (
	.\build\compiler.exe .\Code\program.tom .\Code\program.ass --debug
) else (
.\build\compiler.exe .\Code\program.tom .\Code\program.ass
)
if errorlevel 1 exit /b 1
echo.
echo Generating Program RAM Code...
if /i "%~1"=="verbose" (
	.\build\assembler.exe .\Code\program.ass --debug
) else (
.\build\assembler.exe .\Code\program.ass
)
if errorlevel 1 exit /b 1
exit /b 0

:usage
echo Usage: run.bat [genInst^|cu_unicode^|compile_c^|assembly_c^|fast_run^|clean_run] [verbose]
echo   genInst     Build and run the instruction table/dictionary generator.
echo   cu_unicode  Build and run the Control Unit ROM generator.
echo   compile_c   Build the TOM compiler and compile Code\program.tom. Supports verbose.
echo   assembly_c  Build the assembler and assemble Code\program.ass. Supports verbose.
echo   fast_run    Run existing compiler and assembler executables. Default; supports verbose.
echo   clean_run   Run fast_run, then delete temp.tom, tempf.ass, and Code\program.ass.
echo   verbose     Pass --debug to compiler and assembler modes.
exit /b 0

:usage_error
call :usage
exit /b 2
