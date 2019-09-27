@echo off

IF "%1"=="" GOTO USAGE

iidx-irbeat-patch.exe 9 %1 e\\settings.bin.0
iidx-irbeat-patch.exe 9 %1 f\\settings.bin.1
GOTO END

:USAGE
	ECHO "Usage: iidx-irbeat-patch.bat <beat phase>"

:END
