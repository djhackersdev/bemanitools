@echo off

cd /d %~dp0

if not exist conf\nvram mkdir conf\nvram
if not exist conf\raw mkdir conf\raw

regsvr32 /s k-clvsd.dll
regsvr32 /s xactengine2_10.dll

.\launcher.exe -K .\ddrhook.dll .\ddr.dll %*
