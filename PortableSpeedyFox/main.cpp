//PortableSpeedyFox
//Fix import table:
//speedyfox.exe: advapi32.dll/psapi.dll/wininet.dll/winspool.drv
/*
INFO:
speedyfox.exe:

Patch resource:
Bitmap: replace with 1x1 monochrome image

Menu:
SEPARATOR: remove all
32773: remove
32775: remove

Dialog/102:
-1: 0x0/0x0/hide
152: 0x0/0x0/hide
1003: top=180
1009: 0x0/0x0/hide/""
1012: height: 155
1025: 0x0/0x0/""
Dialog/137: remove
*/

#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <shellapi.h>
#include <wininet.h>
#include <winspool.h>
#ifdef _WIN64
#include "../minhook.h"
#else
#include "../helper.h"
#endif

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\CrystalIdea Software\\SpeedyFox\\preferences.xml");
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
LONG WINAPI RegCreateKeyExW_Stub(HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegDeleteValueW_RegDeleteKeyW_Stub(HKEY, LPCWSTR)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegEnumKeyW_Stub(HKEY, DWORD, LPWSTR, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegEnumValueW_Stub(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegOpenKeyExW_Stub(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueExW_Stub(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueW_Stub(HKEY, LPCWSTR, LPWSTR, PLONG)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegSetValueExW_Stub(HKEY, LPCWSTR, DWORD, DWORD, CONST BYTE *, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
//psapi.dll
DWORD WINAPI GetModuleFileNameExW_Stub(HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
    return K32GetModuleFileNameExW(hProcess, hModule, lpFilename, nSize);
}
//wininet.dll
WINBOOL WINAPI InternetCanonicalizeUrlW_Stub(LPCWSTR, LPWSTR, LPDWORD, DWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetCloseHandle_Stub(HINTERNET)
{
    return FALSE;
}
WINBOOL WINAPI InternetCrackUrlW_Stub(LPCWSTR, DWORD, DWORD, LPURL_COMPONENTSW)
{
    return FALSE;
}
WINBOOL WINAPI InternetGetLastResponseInfoW_Stub(LPDWORD, LPWSTR, LPDWORD)
{
    return FALSE;
}
HINTERNET WINAPI InternetOpenUrlW_Stub(HINTERNET, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR)
{
    return nullptr;
}
HINTERNET WINAPI InternetOpenW_Stub(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD)
{
    return nullptr;
}
WINBOOL WINAPI InternetQueryDataAvailable_Stub(HINTERNET, LPDWORD, DWORD, DWORD_PTR)
{
    return FALSE;
}
WINBOOL WINAPI InternetQueryOptionW_Stub(HINTERNET, DWORD, LPVOID, LPDWORD)
{
    return FALSE;
}
WINBOOL WINAPI InternetReadFile_Stub(HINTERNET, LPVOID, DWORD, LPDWORD)
{
    return FALSE;
}
DWORD WINAPI InternetSetFilePointer_Stub(HINTERNET, LONG, PVOID, DWORD, DWORD_PTR)
{
    return INVALID_SET_FILE_POINTER;
}
WINBOOL WINAPI InternetSetOptionW_Stub(HINTERNET, DWORD, LPVOID, DWORD)
{
    return FALSE;
}
INTERNET_STATUS_CALLBACK WINAPI InternetSetStatusCallbackW_Stub(HINTERNET, INTERNET_STATUS_CALLBACK)
{
    return INTERNET_INVALID_STATUS_CALLBACK;
}
WINBOOL WINAPI InternetWriteFile_Stub(HINTERNET, LPCVOID, DWORD, LPDWORD)
{
    return FALSE;
}
//winspool.drv
WINBOOL WINAPI ClosePrinter_Stub(HANDLE)
{
    return FALSE;
}
LONG WINAPI DocumentPropertiesW_Stub(HWND, HANDLE, LPWSTR, PDEVMODEW, PDEVMODEW, DWORD)
{
    return -1;
}
WINBOOL WINAPI OpenPrinterW_Stub(LPWSTR, LPHANDLE, LPPRINTER_DEFAULTSW)
{
    return FALSE;
}
}

[[nodiscard]]
static HINSTANCE WINAPI ShellExecuteW_Stub(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT)
{
    return reinterpret_cast<HINSTANCE>(ERROR_FILE_NOT_FOUND);
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
#ifdef _WIN64
    static_assert(false, "only 32-bit");
#endif
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
        if ((g_iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen)) >= FCStrLen("C:\\A") &&
                FHook(ShellExecuteW) &&
                FHook(SHGetSpecialFolderPathW) &&
                NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
        {
            g_pwDirFrom = pusImagePathName->Buffer;
            return TRUE;
        }
    }
    return FALSE;
}
