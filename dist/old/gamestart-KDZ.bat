@echo off

cd /d %~dp0

if not exist dev\nvram mkdir dev\nvram
if not exist dev\raw mkdir dev\raw

subst /D E:
subst /D F:

subst E: dev\E
subst F: dev\F

launcher -k iidxhook.dll bm2dx.dll -u -i 1002:7146 %*
