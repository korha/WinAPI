//Tank-O-BoxFix
//Fix import table:
//Tank-o-box.exe: advapi32.dll/dinput8.dll/shell32.dll
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const int g_iCodePageWesternEuropean = 1251;
static constexpr const int g_iDefaultWidth = 1024;
static constexpr const int g_iDefaultHeight = 768;
static HWND g_hWndSubclass;
static WNDPROC FWindowProcReal;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
LONG WINAPI RegCloseKey_Stub(HKEY hKey)
{
    return RegCloseKey(hKey);
}
LONG WINAPI RegCreateKeyExA_Stub(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD)
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
//dinput8.dll
HRESULT WINAPI DirectInput8Create_Stub(HINSTANCE, DWORD, REFIID, LPVOID *, LPVOID)
{
    return S_FALSE;
}
//shell32.dll
WINBOOL WINAPI SHGetPathFromIDListA_Stub(PCIDLIST_ABSOLUTE, LPSTR)
{
    return FALSE;
}
HRESULT WINAPI SHGetSpecialFolderLocation_Stub(HWND, int, PIDLIST_ABSOLUTE *)
{
    return S_FALSE;
}
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LRESULT CALLBACK FWindowProcSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (hWnd == g_hWndSubclass && hWnd)
    {
        static LONG iCxBorders, iCyBorders;

        switch (uMsg)
        {
        case WM_SIZING:
        {
            if (const LONG iStyle = GetWindowLongPtrW(hWnd, GWL_STYLE))
                if (const LONG iExStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE))
                {
                    RECT rectAdjust = {0, 0, g_iDefaultWidth, g_iDefaultHeight};
                    if (AdjustWindowRectEx(&rectAdjust, static_cast<DWORD>(iStyle), FALSE, static_cast<DWORD>(iExStyle)))
                    {
                        iCxBorders = rectAdjust.right - rectAdjust.left - g_iDefaultWidth;
                        iCyBorders = rectAdjust.bottom - rectAdjust.top - g_iDefaultHeight;
                        RECT * const rect = reinterpret_cast<RECT*>(lParam);
                        if (wParam == WMSZ_TOP || wParam == WMSZ_BOTTOM)
                            rect->right = rect->left + (rect->bottom - rect->top - iCyBorders)*4/3 + iCxBorders;
                        else if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
                            rect->top = rect->bottom - (rect->right - rect->left - iCxBorders)*3/4 - iCyBorders;
                        else
                            rect->bottom = rect->top + (rect->right - rect->left - iCxBorders)*3/4 + iCyBorders;
                    }
                }
            return TRUE;
        }
        case WM_GETMINMAXINFO:
        {
            if (const int iCx = GetSystemMetrics(SM_CXVIRTUALSCREEN))
                if (const int iCy = GetSystemMetrics(SM_CYVIRTUALSCREEN))
                {
                    POINT * const point = &reinterpret_cast<MINMAXINFO*>(lParam)->ptMaxTrackSize;
                    point->x = (iCy - iCyBorders)*4/3 + iCxBorders;
                    point->y = (iCx - iCxBorders)*3/4 + iCyBorders;
                }
            return 0;
        }
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            if (wParam == VK_SPACE)
                wParam = VK_TAB;
            else if (wParam == VK_SHIFT)
                wParam = VK_CONTROL;
            break;
        }
        case WM_DESTROY:
        {
            g_hWndSubclass = nullptr;
            break;
        }
        }
    }
    return FWindowProcReal(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static HWND WINAPI CreateWindowExA_Stub(DWORD dwExStyle, LPCSTR, LPCSTR, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    constexpr const wchar_t wClassName[] = {'l','i','b','a',':',':','W','i','n','d','o','w','s','C','r','e','a','t','o','r','\0'};
    constexpr const wchar_t wWindowName[] = {'L','o','a','d','i','n','g','.','.','.','\0'};
    return (g_hWndSubclass = CreateWindowExW(dwExStyle, wClassName, wWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam));
}

[[nodiscard]]
static int WINAPI MultiByteToWideChar_Stub(UINT, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    return FRestoreOrigFunc(MultiByteToWideChar)(g_iCodePageWesternEuropean, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

[[nodiscard]]
static ATOM WINAPI RegisterClassA_Stub(CONST WNDCLASSA *lpWndClass)
{
    if (lpWndClass->lpszClassName)
    {
        constexpr const wchar_t wClassName[] = {'l','i','b','a',':',':','W','i','n','d','o','w','s','C','r','e','a','t','o','r','\0'};
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
            FHookSaveOrigFunc(MultiByteToWideChar) &&
            FHook(RegisterClassA) &&
            NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)));
}
