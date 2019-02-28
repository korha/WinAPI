set X_LIBS=-ladvapi32 -lkernel32 -lntdll -lole32 -lshell32 -luserenv
set X_TYPE=dll

set X_ROOT=%~dp0
call %X_ROOT%..\cmd_compile.cmd