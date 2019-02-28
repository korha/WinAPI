//KeyBlock
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const wchar_t * const g_pwWindowClass = L"41tsntgwmfuqe9s9ykaxyg1ayxn1h69j";

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LRESULT CALLBACK FWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_QUERYENDSESSION)
    {
        PostMessageW(hWnd, WM_CLOSE, 0, 0);
        return FALSE;
    }
    if (uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return NtdllDefWindowProc_W(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LRESULT CALLBACK FLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return nCode == HC_ACTION ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    WNDCLASSEXW wndCl;
    if ((wndCl.hCursor = static_cast<HCURSOR>(LoadImageW(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED))))
    {
        wndCl.hInstance = FGetPTeb()->ProcessEnvironmentBlock->ImageBaseAddress;
        if ((wndCl.hIcon = static_cast<HICON>(LoadImageW(wndCl.hInstance, L"IDI_ICON1", IMAGE_ICON, 32, 32, 0))))
        {
            if ((wndCl.hIconSm = static_cast<HICON>(LoadImageW(wndCl.hInstance, L"IDI_ICON1", IMAGE_ICON, 16, 16, 0))))
            {
                wndCl.cbSize = sizeof(WNDCLASSEXW);
                wndCl.style = 0;
                wndCl.lpfnWndProc = FWindowProc;
                wndCl.cbClsExtra = 0;
                wndCl.cbWndExtra = 0;
                //wndCl.hInstance;
                //wndCl.hIcon;
                //wndCl.hCursor;
                wndCl.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
                wndCl.lpszMenuName = nullptr;
                wndCl.lpszClassName = g_pwWindowClass;
                //wndCl.hIconSm;
                if (RegisterClassExW(&wndCl))
                {
                    RECT rect;
                    if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &rect, 0))
                    {
                        const LONG iWidth = (rect.right - rect.left)/6;
                        const LONG iHeight = (rect.bottom - rect.top)/9;
                        if (CreateWindowExW(0, g_pwWindowClass, L"KeyBlock", (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
                                            (rect.left + rect.right - iWidth)/2,
                                            (rect.top + rect.bottom - iHeight)/2,
                                            iWidth,
                                            iHeight,
                                            nullptr, nullptr, wndCl.hInstance, nullptr))
                            if (const HHOOK hHook = SetWindowsHookExW(WH_KEYBOARD_LL, FLowLevelKeyboardProc, wndCl.hInstance, 0))
                            {
                                MSG msg;
                                while (GetMessageW(&msg, nullptr, 0, 0) > 0)
                                    DispatchMessageW(&msg);
                                UnhookWindowsHookEx(hHook);
                            }
                    }
                    UnregisterClassW(g_pwWindowClass, wndCl.hInstance);
                }
                DestroyIcon(wndCl.hIconSm);
            }
            DestroyIcon(wndCl.hIcon);
        }
    }
    RtlExitUserProcess(0);
}
