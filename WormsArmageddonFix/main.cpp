//WormsArmageddonFix
#define WIN32_LEAN_AND_MEAN
#include <ddraw.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
enum class ESpeed : unsigned char {eDefault, eFast, eSlow};

static constexpr const int g_iMaxClassName = sizeof("Afx:00000000:0:00000000:00000000:00000000");
static constexpr const int g_iMaxWindowName = sizeof("Worms Armageddon");
static ULONGLONG g_iInitStub, g_iInitReal;
static ESpeed g_eSpeedHack;
static HWND g_hWndSubclass;
static WNDPROC FWindowProcReal;

//-------------------------------------------------------------------------------------------------
extern "C"
{
//ddraw.dll
HRESULT WINAPI DirectDrawCreate_Stub(GUID *, LPDIRECTDRAW *, IUnknown *)
{
    return DDERR_INVALIDPARAMS;
}
HRESULT WINAPI DirectDrawCreateEx_Stub(GUID *, LPVOID *, REFIID, IUnknown *)
{
    return DDERR_INVALIDPARAMS;
}
HRESULT WINAPI DirectDrawEnumerateExA_Stub(LPDDENUMCALLBACKEXA, LPVOID, DWORD)
{
    return DDERR_INVALIDPARAMS;
}
}

//-------------------------------------------------------------------------------------------------
static WINBOOL WINAPI QueryPerformanceCounter_Stub(LARGE_INTEGER *lpPerformanceCount)
{
    //only one thread access - no need to synchronize
    ULONGLONG iCurrent = 0;
    NtQueryPerformanceCounter(pointer_cast<LARGE_INTEGER*>(&iCurrent), nullptr);
    iCurrent -= g_iInitReal;
    if (g_eSpeedHack == ESpeed::eFast)
        iCurrent <<= 3;
    else if (g_eSpeedHack == ESpeed::eSlow)
        iCurrent >>= 1;
    iCurrent += g_iInitStub;
    lpPerformanceCount->QuadPart = static_cast<LONGLONG>(iCurrent);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
static void FInitSpeedHack(const ESpeed eSpeedHack)
{
    QueryPerformanceCounter_Stub(pointer_cast<LARGE_INTEGER*>(&g_iInitStub));
    NtQueryPerformanceCounter(pointer_cast<LARGE_INTEGER*>(&g_iInitReal), nullptr);
    g_eSpeedHack = eSpeedHack;
}

//-------------------------------------------------------------------------------------------------
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
            case 'W': wParam = VK_UP; break;
            case 'A': wParam = VK_LEFT; break;
            case 'S': wParam = VK_DOWN; break;
            case 'D': wParam = VK_RIGHT; break;
            case VK_OEM_COMMA: FInitSpeedHack(ESpeed::eSlow); return 0;
            case VK_OEM_PERIOD: FInitSpeedHack(ESpeed::eFast); return 0;
            case VK_OEM_2: FInitSpeedHack(ESpeed::eDefault); return 0;
            }
            break;
        }
        case WM_KEYUP:
        {
            switch (wParam)
            {
            case 'W': wParam = VK_UP; break;
            case 'A': wParam = VK_LEFT; break;
            case 'S': wParam = VK_DOWN; break;
            case 'D': wParam = VK_RIGHT; break;
            case VK_OEM_COMMA:
            case VK_OEM_PERIOD:
            case VK_OEM_2:
                return 0;
            }
            break;
        }
        case WM_DESTROY:
        {
            FInitSpeedHack(ESpeed::eDefault);
            g_hWndSubclass = nullptr;
            break;
        }
        }
    return FWindowProcReal(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static wchar_t * FFill(wchar_t *pwDst, const wchar_t * const pwDstEnd, const char *pcSrc)
{
    do
    {
        if (pwDst >= pwDstEnd)
            return nullptr;
    } while ((*pwDst++ = static_cast<wchar_t>(*pcSrc++)));
    return pwDst;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static HWND WINAPI CreateWindowExA_Stub(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    const wchar_t *pwClassName = nullptr;
    const wchar_t *pwWindowName = nullptr;
    wchar_t wBuf[g_iMaxClassName + g_iMaxWindowName];
    wchar_t *pwIt = wBuf;
    if (lpClassName)
    {
        pwClassName = pwIt;
        if (!(pwIt = FFill(pwIt, FCGetEnd(wBuf), lpClassName)))
            return nullptr;
    }
    if (lpWindowName)
    {
        pwWindowName = pwIt;
        if (!FFill(pwIt, FCGetEnd(wBuf), lpWindowName))
            return nullptr;
    }
    const HWND hWnd = CreateWindowExW(dwExStyle, pwClassName, pwWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    if (hWnd && lpClassName && lpWindowName)
    {
        constexpr const char cClassName[] = {'W','o','r','m','s','2','D','\0'};
        if (FStrCompare(lpClassName, cClassName) && FStrCompare(lpWindowName, cClassName))
            g_hWndSubclass = hWnd;
    }
    return hWnd;
}

[[nodiscard]]
static ATOM WINAPI RegisterClassA_Stub(CONST WNDCLASSA *lpWndClass)
{
    wchar_t wBuf[g_iMaxClassName];
    WNDCLASSEXW wndCl;
    wndCl.lpfnWndProc = lpWndClass->lpfnWndProc;
    if (lpWndClass->lpszClassName)
    {
        if (!FFill(wBuf, FCGetEnd(wBuf), lpWndClass->lpszClassName))
            return 0;
        wndCl.lpszClassName = wBuf;
        constexpr const char cClassName[] = {'W','o','r','m','s','2','D','\0'};
        if (FStrCompare(lpWndClass->lpszClassName, cClassName))
        {
            FWindowProcReal = lpWndClass->lpfnWndProc;
            wndCl.lpfnWndProc = FWindowProcSubclass;
        }
    }
    else
        wndCl.lpszClassName = nullptr;
    wndCl.cbSize = sizeof(WNDCLASSEXW);
    wndCl.style = lpWndClass->style;
    wndCl.cbClsExtra = lpWndClass->cbClsExtra;
    wndCl.cbWndExtra = lpWndClass->cbWndExtra;
    wndCl.hInstance = lpWndClass->hInstance;
    wndCl.hIcon = lpWndClass->hIcon;
    wndCl.hCursor = lpWndClass->hCursor;
    wndCl.hbrBackground = lpWndClass->hbrBackground;
    wndCl.lpszMenuName = nullptr;
    wndCl.hIconSm = nullptr;
    return RegisterClassExW(&wndCl);
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
