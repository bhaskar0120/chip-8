gcc main.c -o output.exe
@echo off
if NOT  %ERRORLEVEL% == 0 goto end
@echo on
output.exe
:end
