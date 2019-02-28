//PortableLibreOffice
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include "../minhook.h"

//-----------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\LibreOffice\\4\\user\\extensions\\bundled\\registry\\com.sun.star.comp.deployment.configuration.PackageRegistryBackend\\????????????.tmp\\OptionsDialog.xcu");
static const wchar_t *g_pwDirFrom;
static BYTE g_iDirLen;

struct [[nodiscard]] SOuString final
{
    struct [[nodiscard]] SRtlUString final
    {
        const int iRefCount;
        int iLength;

        explicit SRtlUString() = delete;
    };
    SRtlUString * const psRtlUString;

    explicit SOuString() = delete;
};

#ifdef __MINGW32__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
static SOuString * __thiscall FGetGeneratorString_Stub(const void * const pThis);
#ifdef __MINGW32__
#pragma GCC diagnostic pop
#endif

static decltype(&FGetGeneratorString_Stub) FGetGeneratorString_Real;

//-----------------------------------------------------------------------------
extern "C"
{
//dbghelp.dll
WINBOOL WINAPI SymInitialize_Stub(HANDLE, PCSTR, WINBOOL)
{
    return FALSE;
}
WINBOOL WINAPI SymFromAddr_Stub(HANDLE, DWORD64, PDWORD64, PVOID)
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
static LONG WINAPI RegOpenKeyA_RegOpenKeyW_Stub(HKEY, LPCVOID, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegOpenKeyExA_RegOpenKeyExW_Stub(HKEY, LPCVOID, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static BOOLEAN NTAPI RtlTimeToTimeFields_Stub(PLARGE_INTEGER, TIME_FIELDS *TimeFields)
{
    TimeFields->Year = 2001;
    TimeFields->Month = 1;
    TimeFields->Day = 1;
    TimeFields->Hour = 0;
    TimeFields->Minute = 0;
    TimeFields->Second = 0;
    TimeFields->Milliseconds = 0;
    TimeFields->Weekday = 1;
    return TRUE;
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
#ifdef __MINGW32__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
[[nodiscard]]
static SOuString * __thiscall FGetGeneratorString_Stub(const void * const pThis)
{
    SOuString * const psOuString = FGetGeneratorString_Real(pThis);
    psOuString->psRtlUString->iLength = 0;
    return psOuString;
}
#ifdef __MINGW32__
#pragma GCC diagnostic pop
#endif

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A"))
        {
            wchar_t wDllName[g_iDirMaxLen + FCStrLen("\\mergedlo.dll")];
            EX_USTRING usDllName(wDllName, FStrCopyEx(FStrCopyEx(wDllName, g_pwDirFrom, g_iDirLen + 1U), "mergedlo.dll"));
            void *pDllHandle;
            if (NT_SUCCESS(LdrGetDllHandleEx(LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT, nullptr, nullptr, &usDllName, &pDllHandle)))
            {
                char cProcName[] = "?GetGeneratorString@DocInfoHelper@utl@@SA?AVOUString@rtl@@XZ";
                EX_ASTRING asProcName(false, cProcName);
                decltype(&FGetGeneratorString_Stub) FGetGeneratorString;
                if (NT_SUCCESS(LdrGetProcedureAddressEx(pDllHandle, &asProcName, 0, reinterpret_cast<void**>(&FGetGeneratorString), 0)) &&
                        FMinHookSaveOrigFunc(FGetGeneratorString) &&
                        FHook(GetTempPathW) &&
                        FMinHookD(RegOpenKeyA, RegOpenKeyA_RegOpenKeyW_Stub) &&
                        FMinHookD(RegOpenKeyW, RegOpenKeyA_RegOpenKeyW_Stub) &&
                        FMinHookD(RegOpenKeyExA, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                        FMinHookD(RegOpenKeyExW, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                        FHook(RtlTimeToTimeFields) &&
                        FHook(SHGetSpecialFolderPathA) &&
                        FHook(SHGetSpecialFolderPathW) &&
                        FHook(SHGetSpecialFolderLocation) &&
                        NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
                {
                    g_pwDirFrom = pusImagePathName->Buffer;
                    return TRUE;
                }
            }
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
        FMinHooksFree();
    return FALSE;
}
