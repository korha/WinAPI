//PortableUltraSurf
//Fix import table:
//u.exe: advapi32.dll/comctl32.dll/netapi32.dll/shell32.dll/winmm.dll/winspool.drv/wsock32.dll
/*
INFO:
Unpack upx:
u.exe

Patch resource:
Dialog/139/1033: VISIBLE=false

Copy resource:
u.exe [BIN/227] to utmp\u

Unpack upx:
utmp\u to utmp\u.exe

Replace resource:
u.exe [BIN/227] with empty stub

Patch binary:
8.8.8.8
->
0.0.0.0

http://www.openssl.org/support/faq.html
http://wujieliulan.com/wujie.htm
http://ultrasurf.us/search.htm
->
http://0.0.0.0/

https://s3-ap-southeast-1.amazonaws.com/wujiesg/u.exe
https://s3.amazonaws.com/ultrasurfus/u.exe
->
https://0.0.0.0/

Software\Microsoft\Windows\CurrentVersion\App Paths\chrome.exe
->
Software\Microsoft\Windows\CurrentVersion\App Paths\chrome.ex_

Applications\iexplore.exe\shell\open\command
->
Applications\iexplore.ex_\shell\open\comman_

C:\Program Files\Internet Explorer\iexplore.exe
->
C:\Program Files\Internet Explorer\iexplore.ex_

ultrasurf.us
->
0.0.0.0

/download/u.exe
->
/
*/

#define WIN32_LEAN_AND_MEAN
#include <iphlpapi.h>
#include <nb30.h>
#include <ntstatus.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <winspool.h>
#include "../helper.h"

//-----------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\utmp\\0123456789ABCDEF");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-----------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
WINBOOL WINAPI CloseServiceHandle_Stub(SC_HANDLE)
{
    return FALSE;
}
WINBOOL WINAPI ControlService_StartServiceW_Stub(SC_HANDLE, DWORD, LPCVOID)
{
    return FALSE;
}
SC_HANDLE WINAPI CreateServiceA_Stub(SC_HANDLE, LPCSTR, LPCSTR, DWORD, DWORD, DWORD, DWORD, LPCSTR, LPCSTR, LPDWORD, LPCSTR, LPCSTR, LPCSTR)
{
    return nullptr;
}
WINBOOL WINAPI CryptAcquireContextW_Stub(HCRYPTPROV *, LPCWSTR, LPCWSTR, DWORD, DWORD)
{
    return TRUE;
}
WINBOOL WINAPI CryptGenRandom_Stub(HCRYPTPROV, DWORD dwLen, BYTE *pbBuffer)
{
    return BCryptGenRandom(nullptr, pbBuffer, dwLen, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == STATUS_SUCCESS;
}
WINBOOL WINAPI CryptReleaseContext_Stub(HCRYPTPROV, DWORD)
{
    return TRUE;
}
SC_HANDLE WINAPI OpenSCManagerW_OpenServiceA_Stub(LPCVOID, LPCVOID, DWORD)
{
    static_assert(sizeof(LPCWSTR) == sizeof(SC_HANDLE), "");
    return nullptr;
}
WINBOOL WINAPI QueryServiceStatus_Stub(SC_HANDLE, LPSERVICE_STATUS)
{
    return FALSE;
}
LONG WINAPI RegCreateKeyA_RegOpenKeyA_Stub(HKEY, LPCSTR, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegCreateKeyExA_Stub(HKEY, LPCSTR ,DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegDeleteKeyA_RegDeleteValueA_Stub(HKEY, LPCSTR)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegEnumKeyA_RegEnumKeyW_Stub(HKEY, DWORD, LPVOID, DWORD)
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
LONG WINAPI RegSetValueExA_Stub(HKEY, LPCSTR, DWORD, DWORD, CONST BYTE *, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
//comctl32.dll
void WINAPI InitCommonControls_Stub()
{
}
WINBOOL WINAPI TrackMouseEvent_Stub(LPTRACKMOUSEEVENT)
{
    return FALSE;
}
//netapi32.dll
UCHAR WINAPI Netbios_Stub(PNCB)
{
    return NRC_BADNCB;
}
//shell32.dll
HINSTANCE WINAPI ShellExecuteA_Stub(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT)
{
    return reinterpret_cast<HINSTANCE>(ERROR_FILE_NOT_FOUND);
}
WINBOOL WINAPI ShellExecuteExA_Stub(SHELLEXECUTEINFOA *)
{
    return FALSE;
}
WINBOOL WINAPI Shell_NotifyIconA_Stub(DWORD, PNOTIFYICONDATAA)
{
    return FALSE;
}
//winmm.dll
WINBOOL WINAPI PlaySoundA_Stub(LPCSTR, HMODULE, DWORD)
{
    return FALSE;
}
//winspool.drv
WINBOOL WINAPI ClosePrinter_Stub(HANDLE)
{
    return FALSE;
}
LONG WINAPI DocumentPropertiesA_Stub(HWND, HANDLE, LPSTR, PDEVMODEA, PDEVMODEA, DWORD)
{
    return -1;
}
WINBOOL WINAPI OpenPrinterA_Stub(LPSTR, LPHANDLE, LPPRINTER_DEFAULTSA)
{
    return FALSE;
}
}

//-----------------------------------------------------------------------------
[[nodiscard]]
static HANDLE WINAPI CreateFileA_Stub(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    if (lpFileName)
    {
        wchar_t wBuf[MAX_PATH];
        wchar_t *pwDst = wBuf;
        const wchar_t * const pwEnd = FCGetEnd(wBuf);
        do
        {
            if ((*pwDst = static_cast<wchar_t>(*lpFileName)) == '\0')
            {
                constexpr const char cHelperExe[] = {'\\','u','.','e','x','e','\0'};
                pwDst -= FCStrLen(cHelperExe);
                if (pwDst >= wBuf && FStrCompare(pwDst, cHelperExe))
                    pwDst[FCStrLen("\\u")] = '\0';
                return CreateFileW(wBuf, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
            }
            ++pwDst;
            ++lpFileName;
        } while (pwDst < pwEnd);
    }
    return INVALID_HANDLE_VALUE;
}

[[nodiscard]]
static DWORD WINAPI GetTempPathA_Stub(DWORD, LPSTR lpBuffer)
{
    //GetTempPathA returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
}

[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //GetTempPathW returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
}

[[nodiscard]]
static NTSTATUS NTAPI NtCreateKey_Stub(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, ULONG, PUNICODE_STRING, ULONG, PULONG)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static WINBOOL WINAPI RegisterHotKey_Stub(HWND, int, UINT, UINT)
{
    return FALSE;
}

[[nodiscard]]
static LONG WINAPI RegOpenCurrentUser_Stub(REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static DWORD WINAPI SendARP_Stub(IPAddr, IPAddr, PVOID, PULONG)
{
    return ERROR_NOT_SUPPORTED;
}

[[nodiscard]]
static HRESULT WINAPI SHGetKnownFolderPath_Stub(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *ppszPath)
{
    if ((*ppszPath = static_cast<wchar_t*>(CoTaskMemAlloc((g_iDirLen + 1U)*sizeof(wchar_t)))))        //alloc with '\0'
    {
        FStrCopy(*ppszPath, g_pwDirFrom, g_iDirLen);
        return S_OK;
    }
    return S_FALSE;
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

[[nodiscard]]
static WINBOOL WINAPI UnregisterHotKey_Stub(HWND, int)
{
    return FALSE;
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
                FHook(CreateFileA) &&
                FHook(GetTempPathA) &&
                FHook(GetTempPathW) &&
                FHook(NtCreateKey) &&
                FHook(RegisterHotKey) &&
                FHook(RegOpenCurrentUser) &&
                FHook(SendARP) &&
                FHook(SHGetKnownFolderPath) &&
                FHook(SHGetValueW) &&
                FHook(SHSetValueW) &&
                FHook(UnregisterHotKey) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
    return FALSE;
}
