set X_BIT=32
set X_LIBS=-ladvapi32 -lbcrypt -liphlpapi -lkernel32 -lntdll -lole32 -lshell32 -lshlwapi -luser32
set X_TYPE=dll

set X_ROOT=%~dp0
call %X_ROOT%..\cmd_compile.cmd