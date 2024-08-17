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
:: next to the folders data, JAA, JAB, etc.
cd /d %~dp0

set CONTENT_DIR=%CD%\..
set BEMANITOOLS_DIR=%CONTENT_DIR%\bemanitools
set REVISION_DIR=%CONTENT_DIR%

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

if not exist d mkdir d
if not exist e mkdir e
if not exist f mkdir f

if not exist e\avs_conf mkdir e\avs_conf
if not exist e\avs_conf\CONF mkdir e\avs_conf\CONF
if not exist e\avs_conf\CONF\NVRAM mkdir e\avs_conf\CONF\NVRAM
if not exist e\avs_conf\CONF\RAW mkdir e\avs_conf\CONF\RAW

if not exist dev\nvram mkdir dev\nvram
if not exist dev\raw mkdir dev\raw

%BEMANITOOLS_DIR%\inject.exe^
  %BEMANITOOLS_DIR%\inject-20-cn.xml^
  %*