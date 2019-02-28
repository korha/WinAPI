//PortableqBittorrent
//Fix import table:
//qbittorrent.exe: mpr.dll
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <userenv.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\qBittorrent\\data\\BT_backup\\0123456789abcdefghijklmnopqrstuvwxyz????.fastresume.0");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//mpr.dll
DWORD WINAPI WNetGetUniversalNameW_Stub(LPCWSTR, DWORD, LPVOID, LPDWORD)
{
    return ERROR_NO_NETWORK;
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
static WINBOOL WINAPI GetUserProfileDirectoryW_Stub(HANDLE, LPWSTR, LPDWORD)
{
    return FALSE;
}

[[nodiscard]]
static LONG WINAPI RegCreateKeyExW_Stub(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyExW_Stub(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
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
        const EX_RTL_USER_PROCESS_PARAMETERS * const pProcParam = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters;
        const UNICODE_STRING * const pusImagePathName = &pProcParam->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A"))
        {
            wchar_t wEnvName[] = {'Q','B','T','_','P','O','R','T','A','B','L','E'};
            EX_USTRING usEnvName(wEnvName);
            wchar_t wEnvValue = '1';
            EX_USTRING usEnvValue(&wEnvValue, sizeof(wchar_t));
            if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnvName, &usEnvValue)) &&
                    FUpdateCurrentDirectory(&pProcParam->CurrentDirectory.DosPath, pusImagePathName->Buffer, g_iDirLen + 1U) &&
                    FHook(GetTempPathW) &&
                    FHook(GetUserProfileDirectoryW) &&
                    FHook(RegCreateKeyExW) &&
                    FHook(RegOpenKeyExW) &&
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
