//PortableBurnAware
//Fix import table:
//*.exe: advapi32.dll/msimg32.dll/wininet.dll
//*.dll: advapi32.dll/msimg32.dll/ws2_32.dll
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <wininet.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\Burnaware\\BurnAware\\burnaware.ini");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
WINBOOL WINAPI CloseServiceHandle_Stub(SC_HANDLE)
{
    return FALSE;
}
SC_HANDLE WINAPI OpenSCManagerW_OpenServiceW_Stub(LPCVOID, LPCWSTR, DWORD)
{
    static_assert(sizeof(LPCWSTR) == sizeof(SC_HANDLE), "");
    return nullptr;
}
LONG WINAPI RegCreateKeyExW_Stub(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegDeleteKeyW_RegDeleteValueW_Stub(HKEY, LPCWSTR)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegEnumKeyW_Stub(HKEY, DWORD, LPWSTR, DWORD)
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
LONG WINAPI RegQueryInfoKeyW_Stub(HKEY, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, PFILETIME)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueW_Stub(HKEY, LPCWSTR, LPWSTR, PLONG)
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
WINBOOL WINAPI StartServiceW_Stub(SC_HANDLE, DWORD, LPCWSTR *)
{
    return FALSE;
}
//msimg32.dll
WINBOOL WINAPI AlphaBlend_TransparentBlt_Stub(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION)
{
    static_assert(sizeof(BLENDFUNCTION) == sizeof(UINT), "");
    return FALSE;
}
//wininet.dll
WINBOOL WINAPI HttpQueryInfoW_Stub(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetCloseHandle_Stub(HINTERNET)
{
    return FALSE;
}
HINTERNET WINAPI InternetOpenUrlW_Stub(HINTERNET, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
HINTERNET WINAPI InternetOpenW_Stub(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD)
{
    return nullptr;
}
WINBOOL WINAPI InternetReadFile_Stub(HINTERNET, LPVOID, DWORD, LPDWORD)
{
    return FALSE;
}
//ws2_32.dll
SOCKET WSAAPI accept_Stub(SOCKET, sockaddr *, int *)
{
    return INVALID_SOCKET;
}
int WSAAPI connect_Stub(SOCKET, const sockaddr *, int)
{
    return SOCKET_ERROR;
}
hostent * WSAAPI gethostbyaddr_Stub(const char *, int, int)
{
    return nullptr;
}
hostent * WSAAPI gethostbyname_Stub(const char *)
{
    return nullptr;
}
servent * WSAAPI getservbyname_Stub(const char *, const char *)
{
    return nullptr;
}
servent * WSAAPI getservbyport_Stub(int, const char *)
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
char * WSAAPI inet_ntoa_Stub(in_addr)
{
    return nullptr;
}
u_short WSAAPI ntohs_Stub(u_short)
{
    return 0;
}
int WSAAPI recv_send_Stub(SOCKET, const char *, int, int)
{
    return SOCKET_ERROR;
}
int WSAAPI shutdown_Stub(SOCKET, int)
{
    return SOCKET_ERROR;
}
SOCKET WSAAPI socket_Stub(int, int, int)
{
    return INVALID_SOCKET;
}
int WSAAPI WSACleanup_Stub()
{
    return SOCKET_ERROR;
}
int WSAAPI WSAGetLastError_Stub()
{
    return 0;
}
void WSAAPI WSASetLastError_Stub(int)
{
}
int WSAAPI WSAStartup_Stub(WORD, LPWSADATA)
{
    return WSASYSNOTREADY;
}
int WINAPI WSAFDIsSet_Stub(SOCKET, fd_set *)
{
    return 0;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static DWORD WINAPI GetEnvironmentVariableW_Stub(LPCWSTR lpName, LPWSTR lpBuffer, DWORD nSize)
{
    if (lpName)
    {
        constexpr const char cAppData[] = {'A','P','P','D','A','T','A','\0'};
        if (FStrCompare(lpName, cAppData))
        {
            FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen);
            return g_iDirLen;
        }
    }
    return FRestoreOrigFunc(GetEnvironmentVariableW)(lpName, lpBuffer, nSize);
}

[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //GetTempPathW returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
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
                FHookSaveOrigFunc(GetEnvironmentVariableW) &&
                FHook(GetTempPathW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
    return FALSE;
}
