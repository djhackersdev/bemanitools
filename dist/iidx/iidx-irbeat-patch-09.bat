@echo off
cd /d %~dp0

REM Open menu if no arguments specified
if "%~1" == "" goto menu

if "%~1" gtr "3" goto badbeat
if "%~1" == "0" goto badbeat

set beat = "%~1" - 1
iidx-irbeat-patch.exe 9 %beat% e\\settings.bin.0
iidx-irbeat-patch.exe 9 %beat% f\\settings.bin.1

if not errorlevel 0 goto error
echo Beat phase patched sucessfully
exit /b

:menu
echo Select beat phase:
echo 1 - beat#1
echo 2 - beat#2
echo 3 - beat#3

choice /n /c:123
set /a beat = %errorlevel% - 1

echo.
iidx-irbeat-patch.exe 9 %beat% e\\settings.bin.0
iidx-irbeat-patch.exe 9 %beat% f\\settings.bin.1
echo.

if not errorlevel 0 goto error
echo Beat phase patched sucessfully
timeout 5
exit /b

:error
echo Something has gone wrong with the patching process, have you run the game at least once?
pause
exit /b

:badbeat
echo Invalid beat phase specified, values are 1 to 3.
exit /b

