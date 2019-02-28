cd %~dp0\
%SystemRoot%\System32\attrib.exe +R "%~dp0helper.h"
%SystemRoot%\System32\attrib.exe +R "%~dp0minhook.h"
@echo off
call :label
goto :eof
:label
for %%f in (*.cmd *.def *.dat *.dll *.exe *.ico *.manifest *.rc *.txt) do echo %%f && %SystemRoot%\System32\attrib.exe +R "%%f"
for /D %%d in (*) do (
cd %%d
call :label
cd ..
)