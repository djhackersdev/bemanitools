@echo off

cd /d %~dp0

if not exist d mkdir d
if not exist e mkdir e
if not exist f mkdir f

if not exist dev\nvram mkdir dev\nvram
if not exist dev\nvram\coin.xml copy prop\defaults\coin.xml dev\nvram\coin.xml
if not exist dev\nvram\eacoin.xml copy prop\defaults\eacoin.xml dev\nvram\eacoin.xml
if not exist dev\raw mkdir dev\raw

launcher -K iidxhook4.dll bm2dx.dll --config iidxhook-18.conf %*
