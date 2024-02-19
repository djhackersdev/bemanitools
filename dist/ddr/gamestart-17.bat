@echo off

:: Game doesn't work properly when not run with administrator privileges
>nul 2>&1 net session

if %errorlevel% neq 0 (
    echo This script requires administrative privileges.
    echo Please run the script as an administrator.
    pause
    exit 1
)

:: Script expects to be located in a subfolder "bemanitools" in the root folder
:: (contents/) next to the folders modules, data etc.
cd /d %~dp0

:: Script expects to be located in the root folder (contents/) next to the
:: folders modules, data etc.
set CONTENT_DIR=%CD%\..
set BEMANITOOLS_DIR=%CONTENT_DIR%\bemanitools
set COM_DIR=%CONTENT_DIR%\com
set MODULES_DIR=%CONTENT_DIR%\modules

:: Register video codecs, e.g. required for background videos
regsvr32 /s %COM_DIR%\k-clvsd.dll
regsvr32 /s %COM_DIR%\xactengine2_10.dll

:: Keep that data vanilla, no need to copy these around anymore
:: Just add them to the env PATH so launcher can find the libs and game executable
:: Remark: This also requires admin privileges to propage correctly to launcher
set PATH=^
%MODULES_DIR%;^
%BEMANITOOLS_DIR%;^
%PATH%

:: Current working dir is the game's root folder
cd /d %CONTENT_DIR%

%BEMANITOOLS_DIR%\launcher %BEMANITOOLS_DIR%\launcher-17.xml %*

:: Unregister video codec to avoid conflicts with other/older video codecs crashing different
:: game versions, e.g. DDR X
regsvr32 /u /s %COM_DIR%\k-clvsd.dll
regsvr32 /u /s %COM_DIR%\xactengine2_10.dll