@echo off

@REM echo Generator Instruction Table and Dictionary...
@REM g++ ./Generator/generator.cpp -o ./build/generator.exe
@REM .\build\generator.exe
@REM pause

@REM echo Generator CU Unicode...
@REM g++ ./Unicode_CU/unicode_cu.cpp -o ./build/unicode_cu.exe
@REM .\build\unicode_cu.exe
@REM pause

echo Generating Assembly code (Compilation)...
@REM g++ .\Compiler\compiler.cpp -std=c++17 -o .\build\compiler.exe
.\build\compiler.exe ./Code/program.tom ./Code/program.ass
pause

echo Generating Program RAM Code...
@REM g++ .\Assembler\assembler.cpp -o .\build\assembler.exe
.\build\assembler.exe ./Code/program.ass
pause

@REM echo Runing Simulator in Prgram Code...
@REM g++ .\Simulator\simulator.cpp -o .\build\simulator.exe
@REM .\build\simulator.exe ram_unicode --debug
@REM pause

@REM echo Clean up...
@REM del temp.tom
@REM del tempf.ass
@REM del .\\Code\\program.ass
