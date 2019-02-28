set X_APP_NAME=version.dll
set X_LIBS=-ladvapi32 -lkernel32 -lntdll -lole32 -lshell32 -lshlwapi -lwininet
set X_TYPE=dll

set X_ROOT=%~dp0
call %X_ROOT%..\cmd_compile.cmd