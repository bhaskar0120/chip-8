set CC=mingw32-gcc
%CC% main.c -o output.exe
@echo off
if NOT  %ERRORLEVEL% == 0 goto end
@echo on
output.exe
:end
@echo off
echo Program exited with error: %errorlevel%
