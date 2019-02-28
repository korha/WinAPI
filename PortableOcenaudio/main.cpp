//PortableOcenaudio
//Fix import table:
//bearer\qgenericbearer.dll: iphlpapi.dll
//libbase.dll: iphlpapi.dll/netapi32.dll/psapi.dll/ws2_32.dll/wsock32.dll
//libglib-2.0-0.dll: ws2_32.dll
//libiaudio.dll: ws2_32.dll/wsock32.dll
//Qt5Core.dll: mpr.dll/netapi32.dll/shell32.dll/userenv.dll/ws2_32.dll
//Qt5Network.dll: crypt32.dll/dnsapi.dll/iphlpapi.dll/ws2_32.dll
#include <winsock2.h>
#include <iphlpapi.h>
#include <shlobj.h>
#include <windns.h>
#include <ws2tcpip.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\ocenaudio\\ocenaudio.settings");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//crypt32.dll
PCCERT_CONTEXT WINAPI CertCreateCertificateContext_Stub(DWORD, const BYTE *, DWORD)
{
    return nullptr;
}
VOID WINAPI CertFreeCertificateChain_Stub(PCCERT_CHAIN_CONTEXT)
{
}
WINBOOL WINAPI CertFreeCertificateContext_Stub(PCCERT_CONTEXT)
{
    return TRUE;
}
WINBOOL WINAPI CertGetCertificateChain_Stub(HCERTCHAINENGINE, PCCERT_CONTEXT, LPFILETIME, HCERTSTORE, PCERT_CHAIN_PARA, DWORD, LPVOID, PCCERT_CHAIN_CONTEXT *)
{
    return FALSE;
}
//dnsapi.dll
VOID WINAPI DnsFree_Stub(PVOID, DNS_FREE_TYPE)
{
}
DNS_STATUS WINAPI DnsQuery_W_Stub(PCWSTR, WORD, DWORD, PIP4_ARRAY, PDNS_RECORD *, PVOID *)
{
    return ERROR_ACCESS_DENIED;
}
//iphlpapi.dll
NETIO_STATUS NETIOAPI_API_ ConvertInterfaceIndexToLuid_Stub(NET_IFINDEX, PNET_LUID)
{
    return ERROR_FILE_NOT_FOUND;
}
NETIO_STATUS NETIOAPI_API_ ConvertInterfaceLuidToIndex_ConvertInterfaceNameToLuidW_Stub(CONST PVOID, PVOID)
{
    return ERROR_INVALID_PARAMETER;
}
NETIO_STATUS NETIOAPI_API_ ConvertInterfaceLuidToNameW_Stub(CONST NET_LUID *, PWSTR, SIZE_T)
{
    return ERROR_INVALID_PARAMETER;
}
ULONG WINAPI GetAdaptersAddresses_Stub(ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG)
{
    return ERROR_NO_DATA;
}
ULONG WINAPI GetAdaptersInfo_GetNetworkParams_Stub(PVOID, PULONG)
{
    return ERROR_NO_DATA;
}
//mpr.dll
DWORD WINAPI WNetGetUniversalNameW_Stub(LPCWSTR, DWORD, LPVOID, LPDWORD)
{
    return ERROR_NO_NETWORK;
}
//netapi32.dll
NET_API_STATUS WINAPI NetApiBufferFree_Stub(LPVOID)
{
    return ERROR_ACCESS_DENIED;
}
NET_API_STATUS WINAPI NetShareEnum_Stub(LMSTR, DWORD, LPBYTE *, DWORD, LPDWORD, LPDWORD, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
NET_API_STATUS WINAPI NetUserGetInfo_Stub(LPCWSTR, LPCWSTR, DWORD, LPBYTE *)
{
    return ERROR_ACCESS_DENIED;
}
//psapi.dll
DWORD WINAPI GetModuleFileNameExW_Stub(HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
    return K32GetModuleFileNameExW(hProcess, hModule, lpFilename, nSize);
}
//shell32.dll
LPWSTR * WINAPI CommandLineToArgvW_Stub(LPCWSTR lpCmdLine, int *pNumArgs)
{
    return CommandLineToArgvW(lpCmdLine, pNumArgs);
}
HRESULT WINAPI SHGetKnownFolderPath_Stub(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *ppszPath)
{
    if ((*ppszPath = static_cast<wchar_t*>(CoTaskMemAlloc((g_iDirLen + 1U)*sizeof(wchar_t)))))        //alloc with '\0'
    {
        FStrCopy(*ppszPath, g_pwDirFrom, g_iDirLen);
        return S_OK;
    }
    return S_FALSE;
}
WINBOOL WINAPI SHGetSpecialFolderPathW_Stub(HWND, LPWSTR pszPath, int, WINBOOL)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return TRUE;
}
//userenv.dll
WINBOOL WINAPI GetUserProfileDirectoryW_Stub(HANDLE, LPWSTR, LPDWORD)
{
    return FALSE;
}
//ws2_32.dll & wsock32.dll
SOCKET WSAAPI accept_Stub(SOCKET, sockaddr *, int *)
{
    return INVALID_SOCKET;
}
int WSAAPI bind_connect_Stub(SOCKET, const sockaddr *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI closesocket_Stub(SOCKET)
{
    return SOCKET_ERROR;
}
void WSAAPI freeaddrinfo_Stub(LPADDRINFO)
{
}
int WSAAPI getaddrinfo_Stub(const char *, const char *, const addrinfo *, addrinfo **)
{
    return EAI_FAIL;
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
int WSAAPI getnameinfo_Stub(const sockaddr *, socklen_t, char *, DWORD, char *, DWORD, int)
{
    return EAI_FAIL;
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
int WSAAPI listen_shutdown_Stub(SOCKET, int)
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
SOCKET WSAAPI socket_Stub(int, int, int)
{
    return INVALID_SOCKET;
}
SOCKET WSAAPI WSAAccept_Stub(SOCKET, sockaddr *, LPINT, LPCONDITIONPROC, DWORD_PTR)
{
    return INVALID_SOCKET;
}
int WSAAPI WSAAsyncSelect_Stub(SOCKET, HWND, u_int, __LONG32)
{
    return SOCKET_ERROR;
}
int WSAAPI WSACleanup_Stub()
{
    return SOCKET_ERROR;
}
WINBOOL WSAAPI WSACloseEvent_WSASetEvent_Stub(WSAEVENT)
{
    return FALSE;
}
int WSAAPI WSAConnect_Stub(SOCKET, const sockaddr *, int, LPWSABUF, LPWSABUF, LPQOS, LPQOS)
{
    return SOCKET_ERROR;
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
int WSAAPI WSAHtonl_WSANtohl_Stub(SOCKET, u_long, u_long *)
{
    return SOCKET_ERROR;
}
int WSAAPI WSAIoctl_Stub(SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    return SOCKET_ERROR;
}
int WSAAPI WSANtohs_Stub(SOCKET, u_short, u_short *)
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
int WSAAPI WSASendTo_Stub(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, const sockaddr *, int, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
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
    return WSASYSNOTREADY;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //GetTempPathW returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyA_RegOpenKeyW_Stub(HKEY, LPCVOID, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyExA_RegOpenKeyExW_Stub(HKEY, LPCVOID, DWORD, REGSAM, PHKEY)
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
                FHookD(RegOpenKeyA, RegOpenKeyA_RegOpenKeyW_Stub) &&
                FHookD(RegOpenKeyW, RegOpenKeyA_RegOpenKeyW_Stub) &&
                FHookD(RegOpenKeyExA, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHookD(RegOpenKeyExW, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHook(SHGetKnownFolderPath) &&
                FHook(SHGetSpecialFolderPathW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
#ifdef _WIN64
    else if (fdwReason == DLL_PROCESS_DETACH)
        FMinHooksFree();
#endif
    return FALSE;
}
