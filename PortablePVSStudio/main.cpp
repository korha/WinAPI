//PortablePVSStudio
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <wininet.h>
#include "../minhook.h"

//-------------------------------------------------------------------------------------------------
static constexpr const LONG g_iKeyDummy1 = -0x00001000;
static constexpr const LONG g_iKeyDummy2 = -0x00001001;
static constexpr const DWORD g_dwPrcValue = 12374;
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\Microsoft\\CLR_v4.0\\UsageLogs\\Standalone.exe.log");
static wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;
static decltype(&GetEnvironmentVariableW) GetEnvironmentVariableW_Real;
static decltype(&RegCloseKey) RegCloseKey_Real;
static decltype(&RegOpenKeyExW) RegOpenKeyExW_Real;
static decltype(&RegQueryValueExW) RegQueryValueExW_Real;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//version.dll
WINBOOL WINAPI GetFileVersionInfoA_GetFileVersionInfoW_Stub(LPCVOID, DWORD, DWORD, LPVOID)
{
    return FALSE;
}
int WINAPI GetFileVersionInfoByHandle_Stub(int, LPCWSTR, int, int)
{
    return 0;
}
BOOL WINAPI GetFileVersionInfoExA_GetFileVersionInfoExW_Stub(DWORD, LPCVOID, DWORD, DWORD, LPVOID)
{
    return FALSE;
}
DWORD WINAPI GetFileVersionInfoSizeA_GetFileVersionInfoSizeW_Stub(LPCVOID, LPDWORD)
{
    return 0;
}
DWORD WINAPI GetFileVersionInfoSizeExA_GetFileVersionInfoSizeExW_Stub(DWORD, LPCVOID, LPDWORD)
{
    return 0;
}
DWORD WINAPI VerFindFileA_VerFindFileW_VerInstallFileA_VerInstallFileW_Stub(DWORD, LPVOID, LPVOID, LPVOID, LPVOID, PVOID, LPVOID, PUINT)
{
    return 0;
}
DWORD WINAPI VerLanguageNameA_VerLanguageNameW_Stub(DWORD, LPVOID, DWORD)
{
    return 0;
}
WINBOOL WINAPI VerQueryValueA_VerQueryValueW_Stub(LPCVOID, LPCVOID, LPVOID *, PUINT)
{
    return FALSE;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static HRESULT WINAPI CoGetClassObject_Stub(REFCLSID, DWORD, LPVOID, REFIID, LPVOID *)
{
    return E_ACCESSDENIED;
}

[[nodiscard]]
static DWORD WINAPI GetEnvironmentVariableW_Stub(LPCWSTR lpName, LPWSTR lpBuffer, DWORD nSize)
{
    if (lpName && (FStrCompare(lpName, "LOCALAPPDATA") || FStrCompare(lpName, "allusersprofile")))
    {
        if (nSize >= g_iDirLen + 1U)
        {
            FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen);
            return g_iDirLen;
        }
        return g_iDirLen + 1U;
    }
    return GetEnvironmentVariableW_Real(lpName, lpBuffer, nSize);
}

[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //GetTempPathW returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
}

[[nodiscard]]
static HINTERNET WINAPI InternetOpenW_Stub(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD)
{
    return nullptr;
}

[[nodiscard]]
static NTSTATUS NTAPI NtCreateKey_Stub(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, ULONG, PUNICODE_STRING, ULONG, PULONG)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static NTSTATUS NTAPI NtSetValueKey_Stub(HANDLE, PUNICODE_STRING, ULONG, ULONG, PVOID, ULONG)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return (hKey == reinterpret_cast<HKEY>(g_iKeyDummy1) || hKey == reinterpret_cast<HKEY>(g_iKeyDummy2)) ?
                ERROR_SUCCESS : RegCloseKey_Real(hKey);
}

[[nodiscard]]
static LONG WINAPI RegOpenCurrentUser_Stub(REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyExW_Stub(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    if (lpSubKey)
    {
        if (hKey == HKEY_LOCAL_MACHINE)
        {
            if (FStrIsStartsWith(lpSubKey, "SOFTWARE\\"))
            {
                if (FStrCompare(lpSubKey + FCStrLen("SOFTWARE\\"), "Wow6432Node\\ProgramVerificationSystems\\PVS-Studio"))
                {
                    *phkResult = reinterpret_cast<HKEY>(g_iKeyDummy1);
                    return ERROR_SUCCESS;
                }
                if (FStrCompare(lpSubKey + FCStrLen("SOFTWARE\\"), "Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"))
                    return ERROR_ACCESS_DENIED;
            }
        }
        else if (hKey == HKEY_CURRENT_USER && FStrCompare(lpSubKey, "Software\\0fae5799-cec7-406e-bf47-b505e6fd695d"))
        {
            *phkResult = reinterpret_cast<HKEY>(g_iKeyDummy2);
            return ERROR_SUCCESS;
        }
    }
    return RegOpenKeyExW_Real(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

[[nodiscard]]
static LONG WINAPI RegQueryValueExW_Stub(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    if (lpData && !lpcbData)
        return ERROR_INVALID_PARAMETER;
    if (hKey == reinterpret_cast<HKEY>(g_iKeyDummy1))
    {
        if (lpValueName && FStrCompare(lpValueName, "InstallDir"))
        {
            if (lpType)
                *lpType = REG_SZ;
            if (!lpcbData)
                return ERROR_SUCCESS;
            const DWORD dwSize = static_cast<DWORD>((g_iDirLen + 1U)*sizeof(wchar_t));
            if (!lpData)
            {
                *lpcbData = dwSize;
                return ERROR_SUCCESS;
            }
            const bool bIsEnough = *lpcbData >= dwSize;
            *lpcbData = dwSize;
            if (bIsEnough)
            {
                FStrCopy(pointer_cast<wchar_t*>(lpData), g_pwDirFrom, g_iDirLen);
                return ERROR_SUCCESS;
            }
            return ERROR_MORE_DATA;
        }
        return ERROR_FILE_NOT_FOUND;
    }
    if (hKey == reinterpret_cast<HKEY>(g_iKeyDummy2))
    {
        if (lpValueName && FStrCompare(lpValueName, "PRC"))
        {
            if (lpType)
                *lpType = REG_DWORD;
            if (!lpcbData)
                return ERROR_SUCCESS;
            if (!lpData)
            {
                *lpcbData = sizeof(DWORD);
                return ERROR_SUCCESS;
            }
            const bool bIsEnough = *lpcbData >= sizeof(DWORD);
            *lpcbData = sizeof(DWORD);
            if (bIsEnough)
            {
                *pointer_cast<DWORD*>(lpData) = g_dwPrcValue;
                return ERROR_SUCCESS;
            }
            return ERROR_MORE_DATA;
        }
        return ERROR_FILE_NOT_FOUND;
    }
    return RegQueryValueExW_Real(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

[[nodiscard]]
static WINBOOL WINAPI Shell_NotifyIconW_Stub(DWORD, PNOTIFYICONDATAW)
{
    return FALSE;
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
#ifndef _WIN64
    static_assert(false, "only 64-bit");
#endif
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A") &&
                FHook(CoGetClassObject) &&
                FHookSaveOrigFunc(GetEnvironmentVariableW) &&
                FHook(GetTempPathW) &&
                FHook(InternetOpenW) &&
                FHook(NtCreateKey) &&
                FHook(NtSetValueKey) &&
                FHook(RegOpenCurrentUser) &&
                FHookSaveOrigFunc(RegCloseKey) &&
                FHookSaveOrigFunc(RegOpenKeyExW) &&
                FHookSaveOrigFunc(RegQueryValueExW) &&
                FHook(Shell_NotifyIconW) &&
                FHook(SHGetFolderPathW) &&
                FHook(SHGetKnownFolderPath) &&
                FHook(SHGetValueW) &&
                FHook(SHSetValueW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
        FMinHooksFree();
    return FALSE;
}
