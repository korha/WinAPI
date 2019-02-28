//PortableYoutubeDl
//Fix import table:
//youtube-dl.exe: ahell32.dll/user32.dll
//#define U_EXTENDED_VERSION
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const LONG g_iKeyDummy = -0x00001000;
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\.cache\\youtube-dl\\youtube-sigfuncs\\js_en_US-?????????_00.00.json");
static wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//shell32.dll
WINBOOL WINAPI SHGetSpecialFolderPathW_Stub(HWND, LPWSTR pszPath, int, WINBOOL)
{
    FStrCopy(pszPath, g_pwDirFrom, g_iDirLen);
    return TRUE;
}
//user32.dll
int WINAPI MessageBoxA_Stub(HWND, LPCSTR, LPCSTR, UINT)
{
    return IDOK;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return (hKey == reinterpret_cast<HKEY>(g_iKeyDummy)) ? ERROR_SUCCESS : FRestoreOrigFunc(RegCloseKey)(hKey);
}

[[nodiscard]]
static LONG WINAPI RegEnumKeyExW_Stub(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPWSTR, LPDWORD, PFILETIME)
{
    return ERROR_NO_MORE_ITEMS;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyExW_Stub(HKEY hKey, LPCWSTR lpSubKey, DWORD, REGSAM, PHKEY phkResult)
{
    if (hKey == HKEY_CLASSES_ROOT && lpSubKey && *lpSubKey == '\0')
    {
        *phkResult = reinterpret_cast<HKEY>(g_iKeyDummy);
        return ERROR_SUCCESS;
    }
    return ERROR_ACCESS_DENIED;
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
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A"))
        {
            wchar_t wEnvName[] = {'U','S','E','R','P','R','O','F','I','L','E'};
            EX_USTRING usEnvName(true, wEnvName);
            EX_USTRING usEnvValue(pusImagePathName->Buffer, g_iDirLen);
            if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnvName, &usEnvValue)) &&
                    FHookSaveOrigFunc(RegCloseKey) &&
                    FHook(RegEnumKeyExW) &&
                    FHook(RegOpenKeyExW) &&
                    NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
            {
                g_pwDirFrom = pusImagePathName->Buffer;
                return TRUE;
            }
        }
    }
#ifdef U_EXTENDED_VERSION
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        PROCESS_BASIC_INFORMATION procBasicInfo;
        if (!NT_SUCCESS(NtQueryInformationProcess(FGetCurrentProcess(), ProcessBasicInformation, &procBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), nullptr)) ||
                procBasicInfo.ExitStatus)
        {
            const HANDLE hStdIn = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->StandardInput;
            if (hStdIn != INVALID_HANDLE_VALUE)
            {
                wchar_t wChar;
                DWORD dwBytes;
                ReadConsoleW(hStdIn, &wChar, 1, &dwBytes, nullptr);
            }
        }
    }
#endif
    return FALSE;
}
