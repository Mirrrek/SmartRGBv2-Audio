@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cl src\main.cpp src\capturer.cpp src\emitter.cpp user32.lib Kernel32.lib ws2_32.lib winmm.lib Ole32.lib /std:c++17 /EHsc /Fo:out\ /link /out:out\main.exe
call out\main.exe