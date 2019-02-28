//PortableGIMP
//Fix import table:
//bin\libintl-8.dll: advapi32.dll
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\GIMP\\x.xx\\tool-options\\gimp-brightness-contrast-tool");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return RegCloseKey(hKey);
}
LONG WINAPI RegOpenKeyExA_RegOpenKeyExW_Stub(HKEY, LPCSTR, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueExA_Stub(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
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
static LONG WINAPI RegCreateKeyExA_RegCreateKeyExW_Stub(HKEY, LPCVOID, DWORD, LPVOID, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegOpenCurrentUser_Stub(REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static HRESULT WINAPI SHGetFolderPathA_Stub(HWND, int, HANDLE, DWORD, LPSTR pszPath)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return S_OK;
}

[[nodiscard]]
static HRESULT WINAPI SHGetFolderPathW_Stub(HWND, int, HANDLE, DWORD, LPWSTR pszPath)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return S_OK;
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
static HRESULT WINAPI SHGetSpecialFolderLocation_Stub(HWND, int, PIDLIST_ABSOLUTE *ppidl)
{
    wchar_t wBuf[g_iDirMaxLen + 1];        //alloc with '\0'
    FStrCopy(wBuf, g_pwDirFrom, g_iDirLen);
    return SHILCreateFromPath(wBuf, ppidl, nullptr);
}

[[nodiscard]]
static WINBOOL WINAPI SHGetSpecialFolderPathA_Stub(HWND, LPSTR pszPath, int, WINBOOL)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return TRUE;
}

[[nodiscard]]
static WINBOOL WINAPI SHGetSpecialFolderPathW_Stub(HWND, LPWSTR pszPath, int, WINBOOL)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return TRUE;
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
                FHookD(RegCreateKeyExA, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                FHookD(RegCreateKeyExW, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                FHook(RegOpenCurrentUser) &&
                FHookD(RegOpenKeyExA, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHookD(RegOpenKeyExW, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHook(SHGetFolderPathA) &&
                FHook(SHGetFolderPathW) &&
                FHook(SHGetKnownFolderPath) &&
                FHook(SHGetSpecialFolderLocation) &&
                FHook(SHGetSpecialFolderPathA) &&
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
