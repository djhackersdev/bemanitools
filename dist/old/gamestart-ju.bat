@echo off

cd /d %~dp0

if not exist dev\nvram mkdir dev\nvram
if not exist dev\raw mkdir dev\raw

launcher jubeat.dll
