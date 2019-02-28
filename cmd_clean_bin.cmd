cd %~dp0\
@echo off
call :label
goto :eof
:label
for %%f in (*.dat *.dll *.exe) do echo %%f && %SystemRoot%\System32\attrib.exe -R "%%f" && del "%%f"
for /D %%d in (*) do (
cd %%d
call :label
cd ..
)