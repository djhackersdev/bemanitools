@echo off

cd /d %~dp0

inject iidxhook1.dll bm2dx.exe -D --config iidxhook-10.conf %*

