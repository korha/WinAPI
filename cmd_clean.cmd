cd %~dp0\
@echo off
call :label
goto :eof
:label
for %%f in (*.a *.gch *.o *.plist *.pro.user) do echo %%f && del "%%f"
for /D %%d in (*) do (
cd %%d
call :label
cd ..
)