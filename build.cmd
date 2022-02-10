@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cl src\main.cpp ws2_32.lib winmm.lib user32.lib Kernel32.lib Ole32.lib /EHsc /Fo:out\ /link /out:out\main.exe
call out\main.exe