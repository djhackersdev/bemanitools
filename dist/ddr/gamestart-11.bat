@echo off

cd /d %~dp0

if not exist conf mkdir conf
if not exist conf\nvram mkdir conf\nvram
if not exist conf\raw mkdir conf\raw

regsvr32 /s xactengine2_10.dll

inject ddrhook1.dll DDR.exe --config ddr-11.conf %*
