set X_APP_NAME=ExitWindowsSmart.exe
set X_LIBS=-ladvapi32 -lbcrypt -lgdi32 -lntdll -lole32 -lpowrprof -luser32

set X_ROOT=%~dp0
call %X_ROOT%..\cmd_compile.cmd