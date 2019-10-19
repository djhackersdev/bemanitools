# Inject

Inject is a tool that creates a process for a specified application executable
file and injects a specified list of library/dll files into the process before
it is kicked off.

Typically, the tool is run when you use one of the bat script files to start
any of the games that require its usage, e.g. older IIDX games.

It comes with further option switches for development, e.g. Halting until
a debugger is attached, outputting debug output to a logfile etc.
Just run the application without any parameters to get a usage message:
```
inject.exe
```

How inject.exe is used can be taken from the bat script files of various
(typically older) games.
