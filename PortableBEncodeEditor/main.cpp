//PortableBEncodeEditor
//Fix import table:
//BEncodeEditor*.exe: advapi32.dll/mpr.dll/psapi.dll/wininet.dll/wsock32.dll
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <wininet.h>
#include "../helper.h"

//-----------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
WINBOOL WINAPI CreateProcessAsUserW_Stub(HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, WINBOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION)
{
    return FALSE;
}
WINBOOL WINAPI CreateProcessWithLogonW_Stub(LPCWSTR, LPCWSTR, LPCWSTR, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION)
{
    return FALSE;
}
WINBOOL WINAPI GetUserNameW_Stub(LPWSTR, LPDWORD)
{
    return FALSE;
}
LONG WINAPI RegConnectRegistryW_Stub(LPCWSTR, HKEY, PHKEY)
{
    return ERROR_ACCESS_DENIED;
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
LONG WINAPI RegOpenKeyExW_Stub(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY)
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
//mpr.dll
DWORD WINAPI WNetAddConnection2W_Stub(LPNETRESOURCEW, LPCWSTR, LPCWSTR, DWORD)
{
    return ERROR_NO_NETWORK;
}
DWORD WINAPI WNetCancelConnection2W_Stub(LPCWSTR, DWORD, WINBOOL)
{
    return ERROR_NOT_CONNECTED;
}
DWORD WINAPI WNetGetConnectionW_Stub(LPCWSTR, LPWSTR, LPDWORD)
{
    return ERROR_NO_NETWORK;
}
DWORD WINAPI WNetUseConnectionW_Stub(HWND, LPNETRESOURCEW, LPCWSTR, LPCWSTR, DWORD, LPWSTR, LPDWORD, LPDWORD)
{
    return ERROR_NO_NETWORK;
}
//wininet.dll
DWORD WINAPI FtpGetFileSize_Stub(HINTERNET, LPDWORD)
{
    return 0;
}
HINTERNET WINAPI FtpOpenFileW_Stub(HINTERNET, LPCWSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
HINTERNET WINAPI HttpOpenRequestW_Stub(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR *, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI HttpQueryInfoW_Stub(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI HttpSendRequestW_Stub(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetCloseHandle_Stub(HINTERNET)
{
    return FALSE;
}
HINTERNET WINAPI InternetConnectW_Stub(HINTERNET, LPCWSTR, INTERNET_PORT, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI InternetCrackUrlW_Stub(LPCWSTR, DWORD, DWORD, LPURL_COMPONENTSW)
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
//wsock32.dll
SOCKET WSAAPI accept_Stub(SOCKET, sockaddr *, int *)
{
    return INVALID_SOCKET;
}
int WSAAPI bind_Stub(SOCKET, const sockaddr *, int)
{
    return SOCKET_ERROR;
}
int WSAAPI closesocket_Stub(SOCKET)
{
    return SOCKET_ERROR;
}
int WSAAPI connect_Stub(SOCKET, const sockaddr *, int)
{
    return SOCKET_ERROR;
}
hostent * WSAAPI gethostbyname_Stub(const char *)
{
    return nullptr;
}
int WSAAPI gethostname_Stub(char *, int)
{
    return SOCKET_ERROR;
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
int WSAAPI listen_Stub(SOCKET, int)
{
    return SOCKET_ERROR;
}
u_short WSAAPI ntohs_Stub(u_short)
{
    return 0;
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
int WSAAPI WSACleanup_Stub()
{
    return SOCKET_ERROR;
}
int WSAAPI WSAGetLastError_Stub()
{
    return 0;
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
