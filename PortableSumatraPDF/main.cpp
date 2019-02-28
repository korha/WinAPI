//PortableSumatraPDF
//Fix import table:
//SumatraPDF.exe: urlmon.dll/wininet.dll
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <shlwapi.h>
#include <wininet.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\SumatraPDF\\sumatrapdfcache\\0123456789abcdef0123456789abcdef.png");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//urlmon.dll
HRESULT WINAPI CoInternetGetSession_Stub(DWORD, IInternetSession **, DWORD)
{
    return E_INVALIDARG;
}
//wininet.dll
HINTERNET WINAPI HttpOpenRequestW_Stub(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR *, DWORD, DWORD_PTR)
{
    return nullptr;
}
WINBOOL WINAPI HttpQueryInfoW_Stub(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD)
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
HINTERNET WINAPI InternetConnectW_Stub(HINTERNET, LPCWSTR, INTERNET_PORT, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
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
WINBOOL WINAPI InternetSetOptionW_Stub(HINTERNET, DWORD, LPVOID, DWORD)
{
    return FALSE;
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
static NTSTATUS NTAPI NtCreateKey_Stub(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, ULONG, PUNICODE_STRING, ULONG, PULONG)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyExW_Stub(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

static void WINAPI SHAddToRecentDocs_Stub(UINT, LPCVOID)
{
}

static DWORD WINAPI SHDeleteKeyW_Stub(HKEY, LPCWSTR)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static DWORD WINAPI SHDeleteValueW_Stub(HKEY, LPCWSTR, LPCWSTR)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static HRESULT WINAPI SHGetFolderPathW_Stub(HWND, int, HANDLE, DWORD, LPWSTR pszPath)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return S_OK;
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
                FHook(RegOpenKeyExW) &&
                FHook(SHAddToRecentDocs) &&
                FHook(SHDeleteKeyW) &&
                FHook(SHDeleteValueW) &&
                FHook(SHGetFolderPathW) &&
                FHook(SHGetValueW) &&
                FHook(SHSetValueW) &&
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
