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
:: next to the folders data and prog
cd /d %~dp0

set CONTENT_DIR=%CD%\..
set BEMANITOOLS_DIR=%CONTENT_DIR%\bemanitools
set REVISION_DIR=%CONTENT_DIR%\prog\2010040500

if not exist "%BEMANITOOLS_DIR%" (
    echo The bemanitools directory "%BEMANITOOLS_DIR%" does not exist.
    pause
    exit 1
)

if not exist "%REVISION_DIR%" (
    echo The game modules directory "%REVISION_DIR%" does not exist.
    pause
    exit 1
)

:: Keep that data vanilla, no need to copy these around anymore
:: Just add them to the env PATH so inject can find the libs and game executable
:: Remark: This also requires admin privileges to propage correctly to inject
set PATH=^
%REVISION_DIR%;^
%BEMANITOOLS_DIR%;^
%PATH%

:: Current working dir is the selected game revision root folder
cd /d %REVISION_DIR%

if not exist e\bookkeeping mkdir e\bookkeeping
if not exist e\CONF\NVRAM mkdir e\CONF\NVRAM
if not exist e\CONF\NVRAM\0 mkdir e\CONF\NVRAM\0
if not exist e\CONF\NVRAM\1 mkdir e\CONF\NVRAM\1
if not exist e\CONF\NVRAM\2 mkdir e\CONF\NVRAM\2
if not exist e\CONF\NVRAM\3 mkdir e\CONF\NVRAM\3
if not exist e\CONF\RAW mkdir e\CONF\RAW
if not exist e\settings mkdir e\settings
if not exist e\UP mkdir e\UP

%BEMANITOOLS_DIR%\inject.exe^
  %BEMANITOOLS_DIR%\inject-18.xml^
  %*