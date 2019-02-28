//PortablePVSStudio2
//Fix import table:
//x64\PVS-Studio.exe: shell32.dll
#define WIN32_LEAN_AND_MEAN
#ifdef WIN32_LEAN_AND_MEAN
#endif
#include <windef.h>

//-------------------------------------------------------------------------------------------------
extern "C"
WINBOOL WINAPI SHGetSpecialFolderPathA_Stub(HWND, LPSTR, int, WINBOOL)
{
#ifndef _WIN64
    static_assert(false, "only 64-bit");
#endif
    return FALSE;
}
