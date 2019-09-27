@echo off

if "%2"=="" goto USAGE

echo Flashing ezusb firmware (%1)...
ezusb-tool.exe flash %1

if %ERRORLEVEL% neq 0 ( 
   exit 1
)

:: Wait a moment for the ezusb to re-enumerate properly
ping 127.0.0.1 -n 8 > nul
echo Writing FPGA data (%2)...
ezusb-iidx-fpga-flash.exe v1 %2

if %ERRORLEVEL% neq 0 ( 
   exit 1
)

ping 127.0.0.1 -n 3 > nul

goto END

:USAGE
	echo Usage: ezusb-boot.bat ^<ezusb_v1.bin^> ^<fpga_v1.bin^>

:END