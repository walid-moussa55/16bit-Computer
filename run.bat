@echo off

@REM echo Generator Instruction Table and Dictionary...
@REM g++ ./Generator/generator.cpp -o ./Generator/generator.exe
@REM .\Generator\generator.exe
@REM pause

@REM echo Generator CU Unicode...
@REM g++ ./Unicode_CU/unicode_cu.cpp -o ./Unicode_CU/unicode_cu.exe
@REM .\Unicode_CU\unicode_cu.exe
@REM pause

echo Generating Assembly code (Compilation)...
@REM g++ .\Compiler\compiler.cpp -std=c++17 -o .\Compiler\compiler.exe
.\Compiler\compiler.exe ./Code/program.tom ./Code/program.ass
pause

echo Generating Program RAM Code...
@REM g++ .\Assembler\assembler.cpp -o .\Assembler\assembler.exe
.\Assembler\assembler.exe ./Code/program.ass
pause

@REM echo Clean up...
@REM del temp.tom
@REM del tempf.ass
@REM del .\\Code\\program.ass
