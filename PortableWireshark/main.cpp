//PortableWireshark
//Fix import table:
//WinSparkle.dll: advapi32.dll
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\DATA\\decode_as_entries");
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
LONG WINAPI RegCreateKeyExA_RegCreateKeyExW_Stub(HKEY, LPCVOID, DWORD, LPVOID, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegDeleteValueA_Stub(HKEY, LPCSTR)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyExA_RegOpenKeyExW_Stub(HKEY, LPCVOID, DWORD, REGSAM, PHKEY)
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
}

//-------------------------------------------------------------------------------------------------
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
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A"))
        {
            wchar_t wBufDataPath[g_iDirMaxLen + FCStrLen("\\DATA")];
            const wchar_t wBufData[] = {'D','A','T','A'};
            wchar_t wEnvName[] = {'W','I','R','E','S','H','A','R','K','_','A','P','P','D','A','T','A'};
            EX_USTRING usEnvName(true, wEnvName);
            EX_USTRING usEnvValue(wBufDataPath, FStrCopyEx(FStrCopyEx(wBufDataPath, pusImagePathName->Buffer, g_iDirLen + 1U), wBufData, sizeof(wBufData)/sizeof(wchar_t)));
            if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnvName, &usEnvValue)) &&
                    FHookD(RegCreateKeyExA, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                    FHookD(RegCreateKeyExW, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
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
    }
#ifdef _WIN64
    else if (fdwReason == DLL_PROCESS_DETACH)
        FMinHooksFree();
#endif
    return FALSE;
}
