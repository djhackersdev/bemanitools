@echo off

:: Keep all vars scoped to this script
setlocal

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
set MODULES_DIR=%CONTENT_DIR%\modules

if not exist "%BEMANITOOLS_DIR%" (
    echo The bemanitools directory "%BEMANITOOLS_DIR%" does not exist.
    pause
    exit 1
)

if not exist "%MODULES_DIR%" (
    echo The game modules directory "%MODULES_DIR%" does not exist.
    pause
    exit 1
)

:: Keep that data vanilla, no need to copy these around anymore
:: Just add them to the env PATH so launcher can find the libs and game executable
:: Remark: This also requires admin privileges to propage correctly to launcher
set PATH=^
%MODULES_DIR%;^
%BEMANITOOLS_DIR%;^
%PATH%

:: Current working dir is the game's root folder
cd /d %CONTENT_DIR%

%BEMANITOOLS_DIR%\launcher.exe^
  %BEMANITOOLS_DIR%\launcher-29.xml^
  --config %BEMANITOOLS_DIR%\iidxhook-29.conf^
  %*