//PortableNv14
//Fix import table:
//Nv14.exe: advapi32.dll/wininet.dll/wsock32.dll
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <wininet.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const LONG g_iKeyDummy = -0x00001000;
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\Macromedia\\Flash Player\\macromedia.com\\support\\flashplayer\\sys\\#local\\settings.sol");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return (hKey == reinterpret_cast<HKEY>(g_iKeyDummy)) ? ERROR_SUCCESS : RegCloseKey(hKey);
}
LONG WINAPI RegCreateKeyA_Stub(HKEY, LPCSTR, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyExA_Stub(HKEY hKey, LPCSTR lpSubKey, DWORD, REGSAM, PHKEY phkResult)
{
    if (hKey == HKEY_CURRENT_USER && lpSubKey && FStrCompare(lpSubKey, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"))
    {
        *phkResult = reinterpret_cast<HKEY>(g_iKeyDummy);
        return ERROR_SUCCESS;
    }
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueExA_Stub(HKEY hKey, LPCSTR lpValueName, LPDWORD, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    if (hKey == reinterpret_cast<HKEY>(g_iKeyDummy) && lpData && lpValueName && FStrCompare(lpValueName, "AppData"))
    {
        if (lpType)
            *lpType = REG_SZ;
        FStrCopy(pointer_cast<char*>(lpData), g_pwDirFrom, g_iDirLen);
        if (lpcbData)
            *lpcbData = g_iDirLen + 1U;
        return ERROR_SUCCESS;
    }
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegSetValueA_Stub(HKEY, LPCSTR, DWORD, LPCSTR, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegSetValueExA_Stub(HKEY, LPCSTR, DWORD, DWORD, CONST BYTE *, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
//wininet.dll
HINTERNET WINAPI HttpOpenRequestA_Stub(HINTERNET, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR *, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI HttpSendRequestA_Stub(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetCloseHandle_Stub(HINTERNET)
{
    return FALSE;
}
HINTERNET WINAPI InternetConnectA_Stub(HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
HINTERNET WINAPI InternetOpenA_Stub(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD)
{
    return nullptr;
}
WINBOOL WINAPI InternetReadFile_Stub(HINTERNET, LPVOID, DWORD, LPDWORD)
{
    return FALSE;
}
//wsock.dll
hostent * WSAAPI gethostbyname_Stub(const char *)
{
    return nullptr;
}
u_long WSAAPI htonl_Stub(u_long)
{
    return 0;
}
u_short WSAAPI htons_Stub(u_short)
{
    return 0;
}
unsigned __LONG32 WSAAPI inet_addr_Stub(const char *)
{
    return 0;
}
int WSAAPI setsockopt_Stub(SOCKET, int, int, const char *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI WSACleanup_Stub()
{
    return SOCKET_ERROR;
}
int WSAAPI WSAStartup_Stub(WORD, LPWSADATA)
{
    return WSASYSNOTREADY;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static UINT WINAPI GetSystemDirectoryA_Stub(LPSTR lpBuffer, UINT)
{
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen);
    return g_iDirLen;
}

[[nodiscard]]
static LONG WINAPI RegOpenCurrentUser_Stub(REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
#ifdef _WIN64
    static_assert(false, "only 32-bit");
#endif
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A") &&
                FHook(GetSystemDirectoryA) &&
                FHook(RegOpenCurrentUser) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
    return FALSE;
}
