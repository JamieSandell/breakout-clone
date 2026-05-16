@echo off

cls

set compilerflags=/Fd.\bin\ /Fo.\bin\ /Od /std:c17 /TC /W4 /Wall /wd4100 /WX /Zi
set linkerflags=/OUT:bin\breakout_clone.exe User32.Lib Gdi32.lib

cl.exe %compilerflags% Source\breakout_clone.c /link %linkerflags%