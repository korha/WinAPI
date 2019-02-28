//PortableAdobeFlashPlayerPlugin
//Fix import table (lib.dat):
//NPSWF*.dll: psapi.dll
/*
INFO:
Add file "Macromed\Flash\mms.cfg" with following content:
AllowUserLocalTrust=0
AssetCacheSize=0
AutoUpdateDisable=1
AVHardwareDisable=1
DisableContextMenuAdvertising=1
DisableDeviceFontEnumeration=1
DisableProductDownload=1
DisableSockets=1
FileDownloadDisable=1
FileUploadDisable=1
LegacyDomainMatching=0
LocalFileLegacyAction=0
LocalFileReadDisable=1
LocalStorageLimit=1
ProtectedMode=0
RTMFPP2PDisable=1
SilentAutoUpdateEnable=0
ThirdPartyStorage=0

*/

#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <psapi.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDllDirMaxLen = MAX_PATH - sizeof("\\Macromedia\\Flash Player\\macromedia.com\\support\\flashplayer\\sys\\#local\\settings.sol");
static const wchar_t *g_pwDllDirFrom;
static BYTE g_iDirLen;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//psapi.dll
WINBOOL WINAPI GetProcessMemoryInfo_Stub(HANDLE Process, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb)
{
    return K32GetProcessMemoryInfo(Process, ppsmemCounters, cb);
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static UINT WINAPI GetSystemWow64DirectoryW_Stub(LPWSTR lpBuffer, UINT)
{
    FStrCopy(lpBuffer, g_pwDllDirFrom, g_iDirLen);
    return g_iDirLen;
}

[[nodiscard]]
static DWORD WINAPI GetTempPathW_Stub(DWORD, LPWSTR lpBuffer)
{
    //GetTempPathW returned string ends with a backslash
    FStrCopy(lpBuffer, g_pwDllDirFrom, g_iDirLen + 1U);
    return g_iDirLen + 1U;
}

[[nodiscard]]
static NTSTATUS NTAPI NtCreateKey_Stub(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, ULONG, PUNICODE_STRING, ULONG, PULONG)
{
    return ERROR_ACCESS_DENIED;
}

[[nodiscard]]
static LONG WINAPI RegCreateKeyA_RegCreateKeyW_RegOpenKeyA_RegOpenKeyW_Stub(HKEY, LPCVOID, PHKEY)
{
    return ERROR_ACCESS_DENIED;
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

[[nodiscard]]
static HRESULT WINAPI SHGetFolderPathA_Stub(HWND, int, HANDLE, DWORD, LPSTR pszPath)
{
    FStrCopy(pszPath, g_pwDllDirFrom, g_iDirLen);
    return S_OK;
}

[[nodiscard]]
static HRESULT WINAPI SHGetFolderPathW_Stub(HWND, int, HANDLE, DWORD, LPWSTR pszPath)
{
    FStrCopy(pszPath, g_pwDllDirFrom, g_iDirLen);
    return S_OK;
}

[[nodiscard]]
static HRESULT WINAPI SHGetFolderLocation_Stub(HWND, int, HANDLE, DWORD, PIDLIST_ABSOLUTE *ppidl)
{
    wchar_t wBuf[g_iDllDirMaxLen + 1];        //alloc with '\0'
    FStrCopy(wBuf, g_pwDllDirFrom, g_iDirLen);
    return SHILCreateFromPath(wBuf, ppidl, nullptr);
}

[[nodiscard]]
static WINBOOL WINAPI SHGetSpecialFolderPathW_Stub(HWND, LPWSTR pszPath, int, WINBOOL)
{
    FStrCopy(pszPath, g_pwDllDirFrom, g_iDirLen);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const LIST_ENTRY * const pListEntryHead = &FGetPTeb()->ProcessEnvironmentBlock->Ldr->InLoadOrderModuleList;
        const LIST_ENTRY *pListEntryIt = pListEntryHead->Flink;
        const UNICODE_STRING *pusDllName = nullptr;
        while (pListEntryIt != pListEntryHead)
        {
            using U_ENTRY = EX_TEB::EX_PEB::EX_PEB_LDR_DATA::EX_LDR_DATA_TABLE_ENTRY;
            const U_ENTRY * const pLdrDataTableEntry = static_cast<const U_ENTRY*>(FGetOffset(pListEntryIt, FIELD_OFFSET(U_ENTRY, InLoadOrderLinks)));
            if (pLdrDataTableEntry->ModuleBaseAddress == hInstDll)
            {
                pusDllName = &pLdrDataTableEntry->FullDllName;
                break;
            }
            pListEntryIt = pListEntryIt->Flink;
        }
        if (pusDllName &&
                (g_iDirLen = FGetImageDirLen(pusDllName, g_iDllDirMaxLen)) >= FCStrLen("C:\\A") &&
                FHook(GetSystemWow64DirectoryW) &&
                FHook(GetTempPathW) &&
                FHook(NtCreateKey) &&
                FHookD(RegCreateKeyA, RegCreateKeyA_RegCreateKeyW_RegOpenKeyA_RegOpenKeyW_Stub) &&
                FHookD(RegCreateKeyW, RegCreateKeyA_RegCreateKeyW_RegOpenKeyA_RegOpenKeyW_Stub) &&
                FHookD(RegOpenKeyA, RegCreateKeyA_RegCreateKeyW_RegOpenKeyA_RegOpenKeyW_Stub) &&
                FHookD(RegOpenKeyW, RegCreateKeyA_RegCreateKeyW_RegOpenKeyA_RegOpenKeyW_Stub) &&
                FHookD(RegCreateKeyExA, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                FHookD(RegCreateKeyExW, RegCreateKeyExA_RegCreateKeyExW_Stub) &&
                FHookD(RegOpenKeyExA, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHookD(RegOpenKeyExW, RegOpenKeyExA_RegOpenKeyExW_Stub) &&
                FHook(SHGetFolderLocation) &&
                FHook(SHGetFolderPathA) &&
                FHook(SHGetFolderPathW) &&
                FHook(SHGetSpecialFolderPathW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDllDirFrom = pusDllName->Buffer;
            return TRUE;
        }
    }
#ifdef _WIN64
    else if (fdwReason == DLL_PROCESS_DETACH)
        FMinHooksFree();
#endif
    return TRUE;
}
