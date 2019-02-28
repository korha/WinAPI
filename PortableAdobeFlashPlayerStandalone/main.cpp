//PortableAdobeFlashPlayerStandalone
//Fix import table (lib.dat):
//flashplayer_sa.exe: advapi32.dll/crypt32.dll(delay)/psapi.dll(delay)/urlmon.dll(delay)/wininet.dll(delay)/ws2_32.dll(delay)
/*
INFO:
Add file "Macromed\Flash\mms.cfg" with following content:
AllowUserLocalTrust=0
AssetCacheSize=0
AutoUpdateDisable=1
AVHardwareDisable=1
DisableContextMenuAdvertising=1
DisableDeviceFontEnumeration=1
DisableProductDownload=1
DisableSockets=1
FileDownloadDisable=1
FileUploadDisable=1
LegacyDomainMatching=0
LocalFileLegacyAction=0
LocalStorageLimit=1
ProtectedMode=0
RTMFPP2PDisable=1
SilentAutoUpdateEnable=0
ThirdPartyStorage=0

*/

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ntstatus.h>
#include <psapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wininet.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\Macromedia\\Flash Player\\macromedia.com\\support\\flashplayer\\sys\\#local\\settings.sol");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
LONG WINAPI RegCreateKeyExA_RegCreateKeyExW_Stub(HKEY, LPCVOID, DWORD, LPVOID, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyA_Stub(HKEY, LPCSTR, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyExA_RegOpenKeyExW_Stub(HKEY, LPCVOID, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueExA_RegQueryValueExW_Stub(HKEY, LPCVOID, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegSetValueExA_RegSetValueExW_Stub(HKEY, LPCVOID, DWORD, DWORD, CONST BYTE *, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
//crypt32.dll (delay)
WINBOOL WINAPI CertCloseStore_Stub(HCERTSTORE, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI CertCompareCertificate_Stub(DWORD, PCERT_INFO, PCERT_INFO)
{
    return FALSE;
}
PCCERT_CONTEXT WINAPI CertCreateCertificateContext_Stub(DWORD, const BYTE *, DWORD)
{
    return nullptr;
}
PCCERT_CONTEXT WINAPI CertEnumCertificatesInStore_Stub(HCERTSTORE, PCCERT_CONTEXT)
{
    return nullptr;
}
PCCERT_CONTEXT WINAPI CertFindCertificateInStore_Stub(HCERTSTORE, DWORD, DWORD, DWORD, const void *, PCCERT_CONTEXT)
{
    return nullptr;
}
WINBOOL WINAPI CertFreeCertificateContext_Stub(PCCERT_CONTEXT)
{
    return TRUE;
}
HCERTSTORE WINAPI CertOpenStore_Stub(LPCSTR, DWORD, HCRYPTPROV_LEGACY, DWORD, const void *)
{
    return nullptr;
}
WINBOOL WINAPI CertVerifySubjectCertificateContext_Stub(PCCERT_CONTEXT, PCCERT_CONTEXT, DWORD *)
{
    return FALSE;
}
HCERTSTORE WINAPI CryptGetMessageCertificates_Stub(DWORD, HCRYPTPROV_LEGACY, DWORD, const BYTE *, DWORD)
{
    return nullptr;
}
WINBOOL WINAPI CryptVerifyMessageSignature_Stub(PCRYPT_VERIFY_MESSAGE_PARA, DWORD, const BYTE *, DWORD, BYTE *, DWORD *, PCCERT_CONTEXT *)
{
    return FALSE;
}
//psapi.dll (delay)
WINBOOL WINAPI GetProcessMemoryInfo_Stub(HANDLE Process, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb)
{
    return K32GetProcessMemoryInfo(Process, ppsmemCounters, cb);
}
//urlmon.dll (delay)
HRESULT WINAPI CopyStgMedium_Stub(const STGMEDIUM *, STGMEDIUM *)
{
    return S_FALSE;
}
//wininet.dll (delay)
HINTERNET WINAPI HttpOpenRequestA_Stub(HINTERNET, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR *, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI HttpQueryInfoA_Stub(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD)
{
    return FALSE;
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
//ws2_32.dll (delay)
int WSAAPI bind_connect_Stub(SOCKET, const sockaddr *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI closesocket_Stub(SOCKET)
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
int WSAAPI gethostname_Stub(char *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI getpeername_getsockname_Stub(SOCKET, sockaddr *, int *)
{
    return SOCKET_ERROR;
}
servent * WSAAPI getservbyname_Stub(const char *, const char *)
{
    return nullptr;
}
servent * WSAAPI getservbyport_Stub(int, const char *)
{
    return nullptr;
}
int WSAAPI getsockopt_Stub(SOCKET, int, int, char *, int *)
{
    return SOCKET_ERROR;
}
u_long WSAAPI htonl_ntohl_Stub(u_long)
{
    return 0;
}
u_short WSAAPI htons_ntohs_Stub(u_short)
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
int WSAAPI ioctlsocket_Stub(SOCKET, __LONG32, u_long *)
{
    return SOCKET_ERROR;
}
int WSAAPI recv_send_Stub(SOCKET, const char *, int, int)
{
    return SOCKET_ERROR;
}
int WSAAPI recvfrom_Stub(SOCKET, char *, int, int, sockaddr *, int *)
{
    return SOCKET_ERROR;
}
int WSAAPI select_Stub(int, fd_set *, fd_set *, fd_set *, const PTIMEVAL)
{
    return SOCKET_ERROR;
}
int WSAAPI sendto_Stub(SOCKET, const char *, int, int, const sockaddr *, int)
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
    return INVALID_SOCKET;
}
INT WSAAPI WSAAddressToStringA_Stub(LPSOCKADDR, DWORD, LPWSAPROTOCOL_INFOA, LPSTR, LPDWORD)
{
    return SOCKET_ERROR;
}
int WSAAPI WSAAsyncSelect_Stub(SOCKET, HWND, u_int, __LONG32)
{
    return SOCKET_ERROR;
}
int WSAAPI WSACleanup_Stub()
{
    return SOCKET_ERROR;
}
WINBOOL WSAAPI WSACloseEvent_Stub(WSAEVENT)
{
    return FALSE;
}
WSAEVENT WSAAPI WSACreateEvent_Stub(void)
{
    return WSA_INVALID_EVENT;
}
int WSAAPI WSAEnumNetworkEvents_Stub(SOCKET, WSAEVENT, LPWSANETWORKEVENTS)
{
    return SOCKET_ERROR;
}
int WSAAPI WSAEventSelect_Stub(SOCKET, WSAEVENT, __LONG32)
{
    return SOCKET_ERROR;
}
int WINAPI WSAFDIsSet_Stub(SOCKET, fd_set *)
{
    return 0;
}
int WSAAPI WSAGetLastError_Stub()
{
    return 0;
}
int WSAAPI WSAIoctl_Stub(SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    return SOCKET_ERROR;
}
void WSAAPI WSASetLastError_Stub(int)
{
}
SOCKET WSAAPI WSASocketA_WSASocketW_Stub(int, int, int, PVOID, GROUP, DWORD)
{
    return INVALID_SOCKET;
}
int WSAAPI WSAStartup_Stub(WORD, LPWSADATA)
{
    return WSASYSNOTREADY;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static HANDLE WINAPI FindFirstFileW_Stub(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
    if (lpFileName)
    {
        const wchar_t *pwIt = lpFileName;
        while (*pwIt)
            ++pwIt;
        constexpr const char cDlls[] = {'\\','*','.','d','l','l','\0'};
        pwIt -= FCStrLen(cDlls);
        if (!(pwIt >= lpFileName && FStrCompare(pwIt, cDlls)))
            return FindFirstFileExW(lpFileName, FindExInfoBasic, lpFindFileData, FindExSearchNameMatch, nullptr, 0);
    }
    return INVALID_HANDLE_VALUE;
}

[[nodiscard]]
static UINT WINAPI GetSystemWow64DirectoryW_Stub(LPWSTR lpBuffer, UINT)
{
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen);
    return g_iDirLen;
}

[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //GetTempPathW returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
}

[[nodiscard]]
static NTSTATUS NTAPI NtCreateUserProcess_Stub(PHANDLE, PHANDLE, ACCESS_MASK, ACCESS_MASK, POBJECT_ATTRIBUTES, POBJECT_ATTRIBUTES, ULONG, ULONG, EX_RTL_USER_PROCESS_PARAMETERS *, PVOID, PVOID)
{
    return STATUS_OBJECT_NAME_NOT_FOUND;
}

[[nodiscard]]
static WINBOOL WINAPI ShellExecuteExW_Stub(SHELLEXECUTEINFOW *)
{
    return FALSE;
}

[[nodiscard]]
static HRESULT WINAPI SHGetFolderPathW_Stub(HWND, int, HANDLE, DWORD, LPWSTR pszPath)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return S_OK;
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
                FHook(FindFirstFileW) &&
                FHook(GetSystemWow64DirectoryW) &&
                FHook(GetTempPathW) &&
                FHook(NtCreateUserProcess) &&
                FHook(ShellExecuteExW) &&
                FHook(SHGetFolderPathW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
    return FALSE;
}
