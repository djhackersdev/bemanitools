@echo off

cd /d %~dp0

if not exist conf\nvram mkdir conf\nvram
if not exist conf\nvram\coin.xml copy prop\coin.xml conf\nvram\coin.xml
if not exist conf\nvram\eacoin.xml copy prop\eacoin.xml conf\nvram\eacoin.xml
if not exist conf\nvram\share-config.xml copy prop\share-config.xml conf\nvram\share-config.xml
if not exist conf\raw mkdir conf\raw

regsvr32 /s k-clvsd.dll
regsvr32 /s xactengine2_10.dll

.\launcher.exe -K .\ddrhook.dll .\mdxja_945.dll %*
