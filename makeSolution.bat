REM @echo off
chcp 65001 > nul
.\Tools\premake5.exe --file=premake.lua vs2022
if %ERRORLEVEL% NEQ 0 PAUSE