set X_COMPILER_PATH64=D:\Dev\mingw64win_sjlj\bin
set X_COMPILER_PATH32=D:\Dev\mingw32win_sjlj\bin
set PATH=%SystemRoot%\System32
set X_COMPILER_FLAGS=-c -std=c++17 -O2 -ffast-math -fno-asynchronous-unwind-tables -fno-builtin -fno-dollars-in-identifiers -fno-exceptions -fno-extended-identifiers -fno-ident -fno-keep-inline-dllexport -fno-operator-names -fno-rtti -Werror -Wall -Wextra -Wpedantic -Wconversion -Weffc++ -Wformat=2 -Wabi -Wabi-tag -Waggregate-return -Waligned-new=all -Walloca -Walloca-larger-than=1 -Walloc-size-larger-than=1 -Walloc-zero -Warray-bounds=2 -Wcast-align=strict -Wcast-qual -Wcatch-value=3 -Wconditionally-supported -Wctor-dtor-privacy -Wdate-time -Wdisabled-optimization -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wextra-semi -Wfloat-equal -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wimplicit-fallthrough=5 -Winline -Winvalid-pch -Wlogical-op -Wmissing-braces -Wmissing-include-dirs -Wmultichar -Wmultiple-inheritance -Wnoexcept -Wnormalized=nfkc -Wnull-dereference -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpadded -Wplacement-new=2 -Wredundant-decls -Wregister -Wshadow -Wshift-overflow=2 -Wsign-conversion -Wsign-promo -Wstack-protector -Wstrict-null-sentinel -Wstrict-overflow=5 -Wstringop-overflow=4 -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-enum -Wsynth -Wtrampolines -Wundef -Wunsafe-loop-optimizations -Wunused-const-variable=2 -Wunused-macros -Wvector-operation-performance -Wvirtual-inheritance -Wvla-larger-than=1 -Wzero-as-null-pointer-constant -DUNICODE -D_UNICODE -DWIN32 -D_WIN32_WINNT=0xFFFF -DNDEBUG

if not defined X_ROOT goto end

if not defined X_BIT set X_BIT=64
if %X_BIT% NEQ 64 (if %X_BIT% NEQ 32 goto end)

if not defined X_CONSOLE set X_CONSOLE=0
if %X_CONSOLE% NEQ 0 (if %X_CONSOLE% NEQ 1 goto end)

if not defined X_RSRC set X_RSRC=0
if %X_RSRC% NEQ 0 (if %X_RSRC% NEQ 1 goto end)

if not defined X_TYPE set X_TYPE=exe
if %X_TYPE% NEQ exe (if %X_TYPE% NEQ dll goto end)

if not defined X_APP_NAME (
    if %X_TYPE% EQU exe set X_APP_NAME=App.exe
    if %X_TYPE% EQU dll set X_APP_NAME=lib.dll
)

if not defined X_ENTRY_POINT (
    if %X_TYPE% EQU exe (
        if %X_BIT% EQU 64 set X_ENTRY_POINT=start
        if %X_BIT% EQU 32 set X_ENTRY_POINT=_start
    )
    if %X_TYPE% EQU dll (
        if %X_BIT% EQU 64 set X_ENTRY_POINT=DllEntryPoint
        if %X_BIT% EQU 32 set X_ENTRY_POINT=_DllEntryPoint@12
    )
)

if %X_BIT% EQU 64 set X_COMPILER_PATH=%X_COMPILER_PATH64%
if %X_BIT% EQU 32 set X_COMPILER_PATH=%X_COMPILER_PATH32%

if not defined X_DEFINES set X_DEFINES=
set X_COMPILER_FLAGS=%X_COMPILER_FLAGS% %X_DEFINES%

if %X_CONSOLE% EQU 0 set X_SUBSYSTEM=windows
if %X_CONSOLE% EQU 1 set X_SUBSYSTEM=console

set X_LINKER_FLAGS=-nodefaultlibs -nostartfiles -nostdlib -static -Wl,--enable-stdcall-fixup -Wl,--exclude-all-symbols -Wl,--no-insert-timestamp -Wl,-s -Wl,-subsystem,%X_SUBSYSTEM%

if not defined X_FOLDER_SUFFIX set X_FOLDER_SUFFIX=
set X_APP_FOLDER=%X_ROOT%bin%X_BIT%%X_FOLDER_SUFFIX%

set X_APP_PATH=%X_APP_FOLDER%\%X_APP_NAME%

if %X_TYPE% EQU exe set X_LINKER_FLAGS=%X_LINKER_FLAGS% -o %X_APP_PATH% %X_APP_FOLDER%\main.o
if %X_TYPE% EQU dll set X_LINKER_FLAGS=%X_LINKER_FLAGS% -shared -mthreads -Wl,--out-implib,%X_APP_FOLDER%\liblib.a -Wl,%X_ROOT%def.def -o %X_APP_PATH% %X_APP_FOLDER%\main.o

:::::::::::::::::::::::::::::::::::::::::::::::::

%SystemRoot%\System32\attrib.exe -R %X_APP_PATH%
del %X_APP_PATH%
del %X_APP_FOLDER%\liblib.a
del %X_APP_FOLDER%\main.o
del %X_APP_FOLDER%\res_res.o

mkdir %X_APP_FOLDER%

:::::::::::::::::::::::::::::::::::::::::::::::::

%X_COMPILER_PATH%\g++.exe %X_COMPILER_FLAGS% -o %X_APP_FOLDER%\main.o %X_ROOT%main.cpp
if %ERRORLEVEL% NEQ 0 goto fail

if %X_RSRC% EQU 1 (
    %X_COMPILER_PATH%\windres.exe -i %X_ROOT%res.rc -o %X_APP_FOLDER%\res_res.o -DUNICODE -D_UNICODE -DWIN32 -D_WIN32_WINNT=0xFFFF -DNDEBUG
    if %ERRORLEVEL% NEQ 0 goto fail
)

if not defined X_SOURCES set X_SOURCES=
if not defined X_LIBS set X_LIBS=
set X_LINKER_CMD=%X_COMPILER_PATH%\g++.exe -e%X_ENTRY_POINT% %X_LINKER_FLAGS% %X_SOURCES% %X_LIBS%
if %X_RSRC% EQU 1 set X_LINKER_CMD=%X_LINKER_CMD% %X_APP_FOLDER%\res_res.o
%X_LINKER_CMD%
if %ERRORLEVEL% NEQ 0 goto fail

:::::::::::::::::::::::::::::::::::::::::::::::::

%SystemRoot%\System32\attrib.exe +R %X_APP_PATH%

:::::::::::::::::::::::::::::::::::::::::::::::::

echo OK!
goto end
:fail
echo ERROR!
pause
:end
