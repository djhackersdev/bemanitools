@echo off

cd /d %~dp0

inject pnmhook1.dll popn15.exe -D %*

