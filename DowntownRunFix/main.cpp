//DowntownRunFix
//Fix import table:
//Main.pro: advapi32.dll
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static ULONGLONG g_iInitStub, g_iInitReal;
static HWND g_hWndSubclass;
static WNDPROC FWindowProcReal;
static BYTE g_iShift;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return RegCloseKey(hKey);
}
LONG WINAPI RegOpenKeyExA_Stub(HKEY, LPCSTR, DWORD, REGSAM, PHKEY)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegQueryValueExA_Stub(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD)
{
    return ERROR_ACCESS_DENIED;
}
LONG WINAPI RegSetValueExA_Stub(HKEY, LPCSTR, DWORD, DWORD, CONST BYTE *, DWORD)
{
    return ERROR_ACCESS_DENIED;
}
}

//-------------------------------------------------------------------------------------------------
static WINBOOL WINAPI QueryPerformanceCounter_Stub(LARGE_INTEGER *lpPerformanceCount)
{
    //only one thread access - no need to synchronize
    ULONGLONG iCurrent = 0;
    NtQueryPerformanceCounter(pointer_cast<LARGE_INTEGER*>(&iCurrent), nullptr);
    iCurrent -= g_iInitReal;
    iCurrent >>= g_iShift;
    iCurrent += g_iInitStub;
    lpPerformanceCount->QuadPart = static_cast<LONGLONG>(iCurrent);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
static void FInitSpeedHack(const BYTE iShift)
{
    QueryPerformanceCounter_Stub(pointer_cast<LARGE_INTEGER*>(&g_iInitStub));
    NtQueryPerformanceCounter(pointer_cast<LARGE_INTEGER*>(&g_iInitReal), nullptr);
    g_iShift = iShift;
}

[[nodiscard]]
static LRESULT CALLBACK FWindowProcSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (hWnd == g_hWndSubclass && hWnd)
        switch (uMsg)
        {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case '1': FInitSpeedHack(0); return 0;        //2^0 = x1
            case '2': FInitSpeedHack(1); return 0;        //2^1 = x2
            case '4': FInitSpeedHack(2); return 0;        //2^2 = x4
            }
            break;
        }
        case WM_KEYUP:
        {
            switch (wParam)
            {
            case '1': case '2': case '4': return 0;
            }
            break;
        }
        case WM_DESTROY:
        {
            FInitSpeedHack(0);
            g_hWndSubclass = nullptr;
            break;
        }
        }
    return FWindowProcReal(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static HWND WINAPI CreateWindowExA_Stub(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    if (lpClassName)
    {
        constexpr const wchar_t wClassName[] = {'C','i','t','y','R','a','c','e','r','\0'};
        if (FStrCompare(lpClassName, wClassName))
            if (const HWND hWnd = CreateWindowExW(dwExStyle, wClassName, nullptr, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam))
                return (g_hWndSubclass = hWnd);
    }
    return nullptr;
}

[[nodiscard]]
static ATOM WINAPI RegisterClassA_Stub(CONST WNDCLASSA *lpWndClass)
{
    if (lpWndClass->lpszClassName)
    {
        constexpr const wchar_t wClassName[] = {'C','i','t','y','R','a','c','e','r','\0'};
        if (FStrCompare(lpWndClass->lpszClassName, wClassName))
        {
            FWindowProcReal = lpWndClass->lpfnWndProc;
            WNDCLASSEXW wndCl;
            wndCl.cbSize = sizeof(WNDCLASSEXW);
            wndCl.style = lpWndClass->style;
            wndCl.lpfnWndProc = FWindowProcSubclass;
            wndCl.cbClsExtra = lpWndClass->cbClsExtra;
            wndCl.cbWndExtra = lpWndClass->cbWndExtra;
            wndCl.hInstance = lpWndClass->hInstance;
            wndCl.hIcon = lpWndClass->hIcon;
            wndCl.hCursor = lpWndClass->hCursor;
            wndCl.hbrBackground = lpWndClass->hbrBackground;
            wndCl.lpszMenuName = nullptr;
            wndCl.lpszClassName = wClassName;
            wndCl.hIconSm = nullptr;
            return RegisterClassExW(&wndCl);
        }
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
#ifdef _WIN64
    static_assert(false, "only 32-bit");
#endif
    return (fdwReason == DLL_PROCESS_ATTACH &&
            FHook(CreateWindowExA) &&
            FHook(QueryPerformanceCounter) &&
            FHook(RegisterClassA) &&
            NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)));
}
