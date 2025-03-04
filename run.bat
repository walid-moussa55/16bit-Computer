@echo off

echo Generator Instruction Table and Dictionary...
g++ ./Generator/generator.cpp -o ./Generator/generator.exe
@REM .\Generator\generator.exe
pause

echo Generator CU Unicode...
g++ ./Unicode_CU/unicode_cu.cpp -o ./Unicode_CU/unicode_cu.exe
@REM .\Unicode_CU\unicode_cu.exe
pause


@REM Mesure the time to execute .\Assembler\assembler.exe program
@REM Measure-Command { & ".\Assembler\assembler.exe" }

echo Generating Assembly code...
g++ .\Compiler\compiler.cpp -std=c++17 -o .\Compiler\compiler.exe
@REM .\Compiler\compiler.exe 
pause

echo Generating Program RAM Code...
g++ .\Assembler\assembler.cpp -o .\Assembler\assembler.exe
@REM .\Assembler\assembler.exe
pause