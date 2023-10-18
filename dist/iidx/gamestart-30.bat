@echo off
cd /d %~dp0

if not exist dev mkdir dev
if not exist dev\e mkdir dev\e
if not exist dev\g mkdir dev\g
if not exist dev\nvram mkdir dev\nvram
if not exist dev\raw mkdir dev\raw
if not exist dev\raw\log mkdir dev\raw\log
if not exist dev\raw\fscache mkdir dev\raw\fscache

for /R prop\defaults %%D in (*.*) do (
    if not exist dev\nvram\%%~nxD copy /y prop\defaults\%%~nxD dev\nvram
)

modules\launcher -H 134217728 -B iidxhook9.dll bm2dx.dll --config iidxhook-30.conf %*
