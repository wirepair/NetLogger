NetLogger:
================================

Author: @_wirepair / isaac.dawson{}gmail.com.

Logs sent and received buffers to a file, hooks send, recv, sendto and recvfrom.
Writes hex bytes + ascii out to disk, optionally allowing for writing only ascii.
Only really tested in Windows.

How to build:
-------------------------

Copy this project into your pin source directory:
%pin%\source\tools\NetLogger
Open Visual Studio (2008) and build.

How to run:
-------------------------

Hooks netcat's network functions and writes to bonk.log:

c:\pin\pin.exe -t C:\pin\NetLogger.dll -o bonk.log -- nc.exe -l -v -p 999

Log only ascii strings, careful if they are not null terminated we probably will crash.

c:\pin\pin.exe -t C:\pin\NetLogger.dll -a -o donks.log -- nc.exe -l -v -p 999