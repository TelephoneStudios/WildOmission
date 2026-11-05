@echo off
setlocal
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0..\WildOmission.uproject"
set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=WildOmissionEditor"
set "CONFIG=%~2"
if "%CONFIG%"=="" set "CONFIG=Development"

"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" %TARGET% Win64 %CONFIG% -Project="%PROJECT%" -WaitMutex
