@echo off

cd /d %~dp0

if not exist dev\nvram mkdir dev\nvram
if not exist dev\nvram\coin.xml copy prop\defaults\coin.xml dev\nvram\coin.xml
if not exist dev\nvram\eacoin.xml copy prop\defaults\eacoin.xml dev\nvram\eacoin.xml
if not exist dev\raw mkdir dev\raw

launcher -K jbhook2.dll jubeat.dll
