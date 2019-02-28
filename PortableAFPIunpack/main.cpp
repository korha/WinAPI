//PortableAFPIunpack
//Fix import table:
//AFPIunpack.exe: advapi32.dll/user32.dll
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-----------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return RegCloseKey(hKey);
}
LONG WINAPI RegCreateKeyExW_Stub(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegDeleteKeyW_RegDeleteValueW_Stub(HKEY, LPCWSTR)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegEnumKeyExW_RegEnumValueW_Stub(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPVOID, LPVOID, LPVOID)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegFlushKey_Stub(HKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyExW_Stub(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyW_Stub(HKEY, LPCWSTR, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryInfoKeyW_Stub(HKEY, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, PFILETIME)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueExW_Stub(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegSetValueExW_Stub(HKEY, LPCWSTR, DWORD, DWORD, CONST BYTE *, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
//user32.dll
int WINAPI MessageBoxA_Stub(HWND, LPCSTR, LPCSTR, UINT)
{
    return IDOK;
}
}
