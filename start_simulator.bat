@echo off
set "CIRCUIT=%~1"
if "%CIRCUIT%"=="" set "CIRCUIT=Computer\\computer.circ"

java -jar "%LOGISIM_PATH%\\logisim-evolution-4.1.0-all.jar" "%CIRCUIT%"
