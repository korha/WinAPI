set X_APP_NAME=dbghelp.dll
set X_LIBS=-ladvapi32 -lkernel32 -lntdll -lshell32
set X_TYPE=dll

set X_ROOT=%~dp0
call %X_ROOT%..\cmd_compile.cmd