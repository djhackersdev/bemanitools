@echo off

cd /d %~dp0

if not exist e\bookkeeping mkdir e\bookkeeping
if not exist e\CONF\NVRAM mkdir e\CONF\NVRAM
if not exist e\CONF\NVRAM\0 mkdir e\CONF\NVRAM\0
if not exist e\CONF\NVRAM\1 mkdir e\CONF\NVRAM\1
if not exist e\CONF\NVRAM\2 mkdir e\CONF\NVRAM\2
if not exist e\CONF\NVRAM\3 mkdir e\CONF\NVRAM\3
if not exist e\CONF\RAW mkdir e\CONF\RAW
if not exist e\settings mkdir e\settings
if not exist e\UP mkdir e\UP

inject popnhook1.dll ezusb.dll=ezusb2-popn-shim.dll popn17.exe --config popnhook-17.conf %*

