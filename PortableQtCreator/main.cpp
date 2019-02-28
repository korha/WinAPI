//PortableQtCreator
//Fix import table:
//Qt5Core.dll: shell32.dll
#include <shlobj.h>
#include <userenv.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\QtProject\\qtcreator\\generic-highlighter\\valgrind-suppression.xml");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
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
static LONG WINAPI RegCreateKeyExA_RegCreateKeyExW_Stub(HKEY, LPCVOID, DWORD, LPVOID, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
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
        const EX_RTL_USER_PROCESS_PARAMETERS * const pProcParam = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters;
        const UNICODE_STRING * const pusImagePathName = &pProcParam->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C\\A") &&
                FUpdateCurrentDirectory(&pProcParam->CurrentDirectory.DosPath, pusImagePathName->Buffer, g_iDirLen + 1U) &&
                FHook(GetTempPathW) &&
                FHook(GetUserProfileDirectoryW) &&
                FHookD(RegCreateKeyExA, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                FHookD(RegCreateKeyExW, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                FHookD(RegOpenKeyExA, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHookD(RegOpenKeyExW, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHook(SHGetKnownFolderPath) &&
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
