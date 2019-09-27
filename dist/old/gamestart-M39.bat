@echo off

cd /d %~dp0

if not exist dev\nvram mkdir dev\nvram
if not exist dev\raw\bookkeeping mkdir dev\raw\bookkeeping
if not exist dev\raw\ranking mkdir dev\raw\ranking
if not exist dev\raw\settings mkdir dev\raw\settings

launcher popn21.dll
