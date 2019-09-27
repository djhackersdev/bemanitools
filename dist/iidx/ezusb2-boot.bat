@echo off

if "%1"=="" goto USAGE

echo Scanning for ezusb2 board...

:: Yeah, this is one ugly way to pipe stdout to a variable...
for /f %%i in ('ezusb2-tool.exe scan') do set EZUSBDEV=%%i

if %ERRORLEVEL% neq 0 ( 
    echo Error, could not find a connected ezusb2 device
    exit 1
) 

echo Found ezusb2 device at path: "%EZUSBDEV%"

echo Flashing ezusb2 firmware (%1)...
ezusb2-tool.exe flash "%EZUSBDEV%" %1

if %ERRORLEVEL% neq 0 ( 
    echo Error, flashing ezusb2 board  
    exit 1
)

goto END

:USAGE
	echo Usage: ezusb2-boot.bat ^<ezusb2.bin^>

:END