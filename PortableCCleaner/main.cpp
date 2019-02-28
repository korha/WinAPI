//PortableCCleaner
//Fix import table:
//CCleaner*.exe: crypt32.dll(delay)/iphlpapi.dll(delay)/netapi32.dll(delay)/psapi.dll/winhttp.dll/wininet.dll(delay)/winspool.drv/ws2_32.dll
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <shlwapi.h>
#include <wininet.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = 0xFF;
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//crypt32.dll (delay)
WINBOOL WINAPI CertCloseStore_Stub(HCERTSTORE, DWORD)
{
    return FALSE;
}
PCCERT_CONTEXT WINAPI CertFindCertificateInStore_Stub(HCERTSTORE, DWORD, DWORD, DWORD, const void *, PCCERT_CONTEXT)
{
    return nullptr;
}
WINBOOL WINAPI CertFreeCertificateContext_Stub(PCCERT_CONTEXT)
{
    return TRUE;
}
DWORD WINAPI CertGetNameStringW_Stub(PCCERT_CONTEXT, DWORD, DWORD, void *, LPWSTR, DWORD)
{
    return 1;
}
WINBOOL WINAPI CryptDecodeObject_Stub(DWORD, LPCSTR, const BYTE *, DWORD, DWORD, void *, DWORD *)
{
    return FALSE;
}
WINBOOL WINAPI CryptMsgClose_Stub(HCRYPTMSG)
{
    return FALSE;
}
WINBOOL WINAPI CryptMsgGetParam_Stub(HCRYPTMSG, DWORD, DWORD, void *, DWORD *)
{
    return FALSE;
}
WINBOOL WINAPI CryptQueryObject_Stub(DWORD, const void *, DWORD, DWORD, DWORD, DWORD *, DWORD *, DWORD *, HCERTSTORE *, HCRYPTMSG *, const void **)
{
    return FALSE;
}
WINBOOL WINAPI CryptUnprotectData_Stub(DATA_BLOB *, LPWSTR *, DATA_BLOB *, PVOID, CRYPTPROTECT_PROMPTSTRUCT *, DWORD, DATA_BLOB *)
{
    return FALSE;
}
//iphlpapi.dll (delay)
ULONG WINAPI GetAdaptersAddresses_Stub(ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG)
{
    return ERROR_NO_DATA;
}
//netapi32.dll (delay)
NET_API_STATUS WINAPI NetApiBufferFree_Stub(LPVOID)
{
    return ERROR_ACCESS_DENIED;
}
NET_API_STATUS WINAPI NetLocalGroupGetMembers_Stub(LPCWSTR, LPCWSTR, DWORD, LPBYTE *, DWORD, LPDWORD, LPDWORD, PDWORD_PTR)
{
    return ERROR_ACCESS_DENIED;
}
//psapi.dll
DWORD WINAPI GetProcessImageFileNameW_Stub(HANDLE hProcess, LPWSTR lpImageFileName, DWORD nSize)
{
    return K32GetProcessImageFileNameW(hProcess, lpImageFileName, nSize);
}
//winhttp.dll
WINBOOL WINAPI WinHttpAddRequestHeaders_Stub(HINTERNET, LPCWSTR, DWORD, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpCloseHandle_Stub(HINTERNET)
{
    return FALSE;
}
HINTERNET WINAPI WinHttpConnect_Stub(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD)
{
    return nullptr;
}
HINTERNET WINAPI WinHttpOpen_Stub(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD)
{
    return nullptr;
}
HINTERNET WINAPI WinHttpOpenRequest_Stub(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR*, DWORD)
{
    return nullptr;
}
WINBOOL WINAPI WinHttpQueryDataAvailable_WinHttpReceiveResponse_Stub(HINTERNET, LPVOID)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpQueryHeaders_Stub(HINTERNET, DWORD, LPCWSTR, LPVOID, LPDWORD, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpQueryOption_Stub(HINTERNET, DWORD, LPVOID, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpReadData_WinHttpWriteData_Stub(HINTERNET, LPCVOID, DWORD, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpSendRequest_Stub(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpSetCredentials_Stub(HINTERNET, DWORD, DWORD, LPCWSTR, LPCWSTR, LPVOID)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpSetOption_Stub(HINTERNET, DWORD, LPVOID, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI WinHttpSetTimeouts_Stub(HINTERNET, int, int, int, int)
{
    return FALSE;
}
//wininet.dll (delay)
WINBOOL WINAPI DeleteUrlCacheEntryA_DeleteUrlCacheEntryW_Stub(LPCVOID)
{
    return FALSE;
}
WINBOOL WINAPI FindCloseUrlCache_Stub(HANDLE)
{
    return FALSE;
}
HANDLE WINAPI FindFirstUrlCacheEntryExW_Stub(LPCWSTR, DWORD, DWORD, GROUPID, LPINTERNET_CACHE_ENTRY_INFOW, LPDWORD, LPVOID, LPDWORD, LPVOID)
{
    return nullptr;
}
HANDLE WINAPI FindFirstUrlCacheEntryW_Stub(LPCWSTR, LPINTERNET_CACHE_ENTRY_INFOW, LPDWORD)
{
    return nullptr;
}
WINBOOL WINAPI FindNextUrlCacheEntryExW_Stub(HANDLE, LPINTERNET_CACHE_ENTRY_INFOW, LPDWORD, LPVOID, LPDWORD, LPVOID)
{
    return FALSE;
}
WINBOOL WINAPI FindNextUrlCacheEntryW_Stub(HANDLE, LPINTERNET_CACHE_ENTRY_INFOW, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI HttpAddRequestHeadersW_Stub(HINTERNET, LPCWSTR, DWORD, DWORD)
{
    return FALSE;
}
HINTERNET WINAPI HttpOpenRequestA_HttpOpenRequestW_Stub(HINTERNET, LPCVOID, LPCVOID, LPCVOID, LPCVOID, LPCVOID *, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI HttpQueryInfoA_HttpQueryInfoW_Stub(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI HttpSendRequestA_HttpSendRequestW_Stub(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetCloseHandle_Stub(HINTERNET)
{
    return FALSE;
}
HINTERNET WINAPI InternetConnectA_InternetConnectW_Stub(HINTERNET, LPCVOID, INTERNET_PORT, LPCVOID, LPCVOID, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI InternetCrackUrlW_Stub(LPCWSTR, DWORD, DWORD, LPURL_COMPONENTSW)
{
    return FALSE;
}
DWORD InternetErrorDlg_Stub(HWND, HINTERNET, DWORD, DWORD, LPVOID *)
{
    return ERROR_CANCELLED;
}
HINTERNET WINAPI InternetOpenA_InternetOpenW_Stub(LPCVOID, DWORD, LPCVOID, LPCVOID, DWORD)
{
    return nullptr;
}
HINTERNET WINAPI InternetOpenUrlW_Stub(HINTERNET, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI InternetQueryDataAvailable_Stub(HINTERNET, LPDWORD, DWORD, DWORD_PTR)
{
    return FALSE;
}
WINBOOL WINAPI InternetQueryOptionW_Stub(HINTERNET, DWORD, LPVOID, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetReadFile_Stub(HINTERNET, LPVOID, DWORD, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetSetOptionW_Stub(HINTERNET, DWORD, LPVOID, DWORD)
{
    return FALSE;
}
//winspool.drv
WINBOOL WINAPI GetDefaultPrinterW_Stub(LPWSTR, LPDWORD)
{
    return FALSE;
}
//ws2_32.dll
int WSAAPI bind_Stub(SOCKET, const sockaddr *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI closesocket_Stub(SOCKET)
{
    return 0;        //sic! avoid crash the current app
    //return SOCKET_ERROR;
}
void WSAAPI FreeAddrInfoW_Stub(PADDRINFOW)
{
}
int WSAAPI GetAddrInfoW_Stub(PCWSTR, PCWSTR, const ADDRINFOW *, PADDRINFOW *)
{
    return EAI_FAIL;
}
int WSAAPI getsockopt_Stub(SOCKET, int, int, char *, int *)
{
    return 0;        //sic! avoid crash the current app
    //return SOCKET_ERROR;
}
u_short WSAAPI htons_Stub(u_short)
{
    return 0;
}
int WSAAPI ioctlsocket_Stub(SOCKET, __LONG32, u_long *)
{
    return SOCKET_ERROR;
}
int WSAAPI listen_Stub(SOCKET, int)
{
    return SOCKET_ERROR;
}
int WSAAPI select_Stub(int, fd_set *, fd_set *, fd_set *, const PTIMEVAL)
{
    return SOCKET_ERROR;
}
int WSAAPI setsockopt_Stub(SOCKET, int, int, const char *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI shutdown_Stub(SOCKET, int)
{
    return SOCKET_ERROR;
}
SOCKET WSAAPI socket_Stub(int, int, int)
{
    return 0;        //sic! avoid crash the current app
    //return INVALID_SOCKET;
}
int WSAAPI WSAGetLastError_Stub()
{
    return 0;
}
int WSAAPI WSAIoctl_Stub(SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    return SOCKET_ERROR;
}
int WSAAPI WSARecv_Stub(SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    return SOCKET_ERROR;
}
int WSAAPI WSARecvFrom_Stub(SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, sockaddr *, LPINT, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    return SOCKET_ERROR;
}
int WSAAPI WSASend_Stub(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    return SOCKET_ERROR;
}
void WSAAPI WSASetLastError_Stub(int)
{
}
SOCKET WSAAPI WSASocketW_Stub(int, int, int, LPWSAPROTOCOL_INFOW, GROUP, DWORD)
{
    return INVALID_SOCKET;
}
int WSAAPI WSAStartup_Stub(WORD, LPWSADATA)
{
    return 0;        //sic! avoid crash the current app
    //return WSASYSNOTREADY;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //string already ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen);
    return g_iDirLen;
}

[[nodiscard]]
static NTSTATUS NTAPI NtCreateKey_Stub(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, ULONG, PUNICODE_STRING, ULONG, PULONG)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegCreateKeyExW_Stub(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static DWORD WINAPI SHGetValueW_Stub(HKEY, LPCWSTR, LPCWSTR, DWORD *, void *, DWORD *)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static DWORD WINAPI SHSetValueW_Stub(HKEY, LPCWSTR, LPCWSTR, DWORD, LPCVOID, DWORD)
{
    return ERROR_ACCESS_DENIED;
}

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A") &&
                FHook(GetTempPathW) &&
                FHook(NtCreateKey) &&
                FHook(RegCreateKeyExW) &&
                FHook(SHGetValueW) &&
                FHook(SHSetValueW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            ++g_iDirLen;        //GetTempPathW returned string ends with a backslash
            return TRUE;
        }
    }
#ifdef _WIN64
    else if (fdwReason == DLL_PROCESS_DETACH)
        FMinHooksFree();
#endif
    return FALSE;
}
