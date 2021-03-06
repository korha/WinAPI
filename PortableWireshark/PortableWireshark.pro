TARGET = lib

TEMPLATE = lib

CONFIG -= exceptions
CONFIG -= qt
CONFIG(release, debug|release):DEFINES += NDEBUG
DEFINES += _WIN32_WINNT=0xFFFF

LIBS += -ladvapi32 -lntdll -lole32 -lshell32

SOURCES += main.cpp

DEF_FILE = def.def

equals(QT_ARCH, i386):QMAKE_LFLAGS += -e_DllEntryPoint@12
equals(QT_ARCH, x86_64):QMAKE_LFLAGS += -eDllEntryPoint
QMAKE_LFLAGS += -nodefaultlibs
QMAKE_LFLAGS += -nostartfiles
QMAKE_LFLAGS += -nostdlib
QMAKE_LFLAGS += -static
QMAKE_LFLAGS += -Wl,--enable-stdcall-fixup
QMAKE_LFLAGS += -Wl,--exclude-all-symbols
QMAKE_LFLAGS += -Wl,--no-insert-timestamp
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -ffast-math
QMAKE_CXXFLAGS += -fno-asynchronous-unwind-tables
QMAKE_CXXFLAGS += -fno-builtin
QMAKE_CXXFLAGS += -fno-dollars-in-identifiers
QMAKE_CXXFLAGS += -fno-exceptions
QMAKE_CXXFLAGS += -fno-extended-identifiers
QMAKE_CXXFLAGS += -fno-ident
QMAKE_CXXFLAGS += -fno-operator-names
QMAKE_CXXFLAGS += -fno-rtti
QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Wextra
QMAKE_CXXFLAGS += -Wpedantic
QMAKE_CXXFLAGS += -Wconversion
QMAKE_CXXFLAGS += -Weffc++
QMAKE_CXXFLAGS += -Wformat=2
QMAKE_CXXFLAGS += -Wabi
QMAKE_CXXFLAGS += -Wabi-tag
QMAKE_CXXFLAGS += -Waggregate-return
QMAKE_CXXFLAGS += -Waligned-new=all
QMAKE_CXXFLAGS += -Walloca
QMAKE_CXXFLAGS += -Walloca-larger-than=1
QMAKE_CXXFLAGS += -Walloc-size-larger-than=1
QMAKE_CXXFLAGS += -Walloc-zero
QMAKE_CXXFLAGS += -Warray-bounds=2
QMAKE_CXXFLAGS += -Wcast-align=strict
QMAKE_CXXFLAGS += -Wcast-qual
QMAKE_CXXFLAGS += -Wcatch-value=3
QMAKE_CXXFLAGS += -Wconditionally-supported
QMAKE_CXXFLAGS += -Wctor-dtor-privacy
QMAKE_CXXFLAGS += -Wdate-time
QMAKE_CXXFLAGS += -Wdisabled-optimization
QMAKE_CXXFLAGS += -Wdouble-promotion
QMAKE_CXXFLAGS += -Wduplicated-branches
QMAKE_CXXFLAGS += -Wduplicated-cond
QMAKE_CXXFLAGS += -Wextra-semi
QMAKE_CXXFLAGS += -Wfloat-equal
QMAKE_CXXFLAGS += -Wformat-overflow=2
QMAKE_CXXFLAGS += -Wformat-signedness
QMAKE_CXXFLAGS += -Wformat-truncation=2
QMAKE_CXXFLAGS += -Wimplicit-fallthrough=5
QMAKE_CXXFLAGS += -Winline
QMAKE_CXXFLAGS += -Winvalid-pch
QMAKE_CXXFLAGS += -Wlogical-op
QMAKE_CXXFLAGS += -Wmissing-braces
QMAKE_CXXFLAGS += -Wmissing-include-dirs
QMAKE_CXXFLAGS += -Wmultichar
QMAKE_CXXFLAGS += -Wmultiple-inheritance
QMAKE_CXXFLAGS += -Wnoexcept
QMAKE_CXXFLAGS += -Wnormalized=nfkc
QMAKE_CXXFLAGS += -Wnull-dereference
QMAKE_CXXFLAGS += -Wold-style-cast
QMAKE_CXXFLAGS += -Woverloaded-virtual
QMAKE_CXXFLAGS += -Wpacked
QMAKE_CXXFLAGS += -Wpadded
QMAKE_CXXFLAGS += -Wplacement-new=2
QMAKE_CXXFLAGS += -Wredundant-decls
QMAKE_CXXFLAGS += -Wregister
QMAKE_CXXFLAGS += -Wshadow
QMAKE_CXXFLAGS += -Wshift-overflow=2
QMAKE_CXXFLAGS += -Wsign-conversion
QMAKE_CXXFLAGS += -Wsign-promo
QMAKE_CXXFLAGS += -Wstack-protector
QMAKE_CXXFLAGS += -Wstrict-null-sentinel
QMAKE_CXXFLAGS += -Wstrict-overflow=5
QMAKE_CXXFLAGS += -Wstringop-overflow=4
QMAKE_CXXFLAGS += -Wsuggest-final-methods
QMAKE_CXXFLAGS += -Wsuggest-final-types
QMAKE_CXXFLAGS += -Wsuggest-override
QMAKE_CXXFLAGS += -Wswitch-enum
QMAKE_CXXFLAGS += -Wsynth
QMAKE_CXXFLAGS += -Wtrampolines
QMAKE_CXXFLAGS += -Wundef
QMAKE_CXXFLAGS += -Wunsafe-loop-optimizations
QMAKE_CXXFLAGS += -Wunused-const-variable=2
QMAKE_CXXFLAGS += -Wunused-macros
QMAKE_CXXFLAGS += -Wvector-operation-performance
QMAKE_CXXFLAGS += -Wvirtual-inheritance
QMAKE_CXXFLAGS += -Wvla-larger-than=1
QMAKE_CXXFLAGS += -Wzero-as-null-pointer-constant
