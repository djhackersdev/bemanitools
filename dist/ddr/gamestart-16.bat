@echo off

cd /d %~dp0

if not exist conf\nvram mkdir conf\nvram
if not exist conf\nvram\ea3-config.xml copy prop\eamuse-config.xml conf\nvram\ea3-config.xml
if not exist conf\nvram\coin.xml copy prop\coin.xml conf\nvram\coin.xml
if not exist conf\nvram\eacoin.xml copy prop\eacoin.xml conf\nvram\eacoin.xml
if not exist conf\nvram\testmode-v.xml copy prop\testmode-v.xml conf\nvram\testmode-v.xml
if not exist conf\raw mkdir conf\raw

regsvr32 /s com\k-clvsd.dll
regsvr32 /s com\xactengine2_10.dll

.\launcher.exe -H 33554432 -K .\ddrhook2.dll .\arkmdxp3.dll %*
