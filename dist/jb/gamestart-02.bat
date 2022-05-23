@echo off

cd /d %~dp0

if not exist dev\nvram mkdir dev\nvram

inject jbhook1.dll jubeat.exe --config jbhook-02.conf %*
