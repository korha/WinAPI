//StrikeoutText
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const wchar_t * const g_pwWindowClass = L"kh02c79qkipzit9jba4msv4a6yaawgv1";
static constexpr const wchar_t g_wStrikeOut = L'\x0336';
static constexpr const int g_iWidth = 650;
static constexpr const int g_iHeight = 28;
static constexpr const int g_iMaxBuf = 512;

struct [[nodiscard]] SArgs final
{
    HFONT hFont;
    HWND hWndInput;
    HWND hWndOutput;
    wchar_t wBuf[g_iMaxBuf*2 + 1];
    BYTE btPad[sizeof(HWND) - sizeof(wchar_t)];

    explicit SArgs() = default;
    SArgs(const SArgs &) = delete;
    SArgs & operator=(const SArgs &) = delete;
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LRESULT CALLBACK FWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static SArgs *psArgs;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        const CREATESTRUCT * const pCrStruct = reinterpret_cast<const CREATESTRUCT*>(lParam);
        psArgs = static_cast<SArgs*>(pCrStruct->lpCreateParams);
        if ((psArgs->hWndInput = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, g_iWidth, g_iHeight, hWnd, nullptr, pCrStruct->hInstance, nullptr)) &&
                (psArgs->hWndOutput = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0, g_iHeight, g_iWidth, g_iHeight, hWnd, nullptr, pCrStruct->hInstance, nullptr)) &&
                SetFocus(psArgs->hWndInput))
        {
            SendMessageW(psArgs->hWndInput, EM_LIMITTEXT, g_iMaxBuf, 0);
            SendMessageW(psArgs->hWndInput, WM_SETFONT, reinterpret_cast<WPARAM>(psArgs->hFont), FALSE);
            SendMessageW(psArgs->hWndOutput, WM_SETFONT, reinterpret_cast<WPARAM>(psArgs->hFont), FALSE);
            return 0;
        }
        return -1;
    }
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == EN_CHANGE && reinterpret_cast<HWND>(lParam) == psArgs->hWndInput)
        {
            wchar_t *pDst = psArgs->wBuf;
            wchar_t *pSrc = pDst + g_iMaxBuf*2;
            *pointer_cast<WORD*>(pSrc) = g_iMaxBuf + 1;        //first word contains buffer size
            const LRESULT iLength = SendMessageW(psArgs->hWndInput, EM_GETLINE, 0, reinterpret_cast<LPARAM>(pSrc));
            if (iLength >= 1 && iLength <= g_iMaxBuf)
            {
                const wchar_t * const pSrcEnd = pSrc + iLength;
                while (pSrc < pSrcEnd)
                {
                    *pDst++ = g_wStrikeOut;
                    *pDst++ = *pSrc++;
                }
            }
            *pDst = '\0';
            SendMessageW(psArgs->hWndOutput, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(psArgs->wBuf));
        }
        return 0;
    }
    case WM_QUERYENDSESSION:
    {
        PostMessageW(hWnd, WM_CLOSE, 0, 0);
        return FALSE;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    }
    return NtdllDefWindowProc_W(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    NONCLIENTMETRICSW nonClientMetrics;
    nonClientMetrics.cbSize = sizeof(NONCLIENTMETRICSW);
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &nonClientMetrics, 0))
    {
        SArgs sArgs;
        nonClientMetrics.lfMessageFont.lfHeight = -g_iHeight/2;
        if ((sArgs.hFont = CreateFontIndirectW(&nonClientMetrics.lfMessageFont)))
        {
            WNDCLASSEXW wndCl;
            if ((wndCl.hCursor = static_cast<HCURSOR>(LoadImageW(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED))))
            {
                wndCl.cbSize = sizeof(WNDCLASSEXW);
                wndCl.style = 0;
                wndCl.lpfnWndProc = FWindowProc;
                wndCl.cbClsExtra = 0;
                wndCl.cbWndExtra = 0;
                wndCl.hInstance = FGetPTeb()->ProcessEnvironmentBlock->ImageBaseAddress;
                wndCl.hIcon = nullptr;
                //wndCl.hCursor;
                wndCl.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
                wndCl.lpszMenuName = nullptr;
                wndCl.lpszClassName = g_pwWindowClass;
                wndCl.hIconSm = nullptr;
                if (RegisterClassExW(&wndCl))
                {
                    RECT rect = {0, 0, g_iWidth, g_iHeight*2};
                    if (AdjustWindowRectEx(&rect, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX), FALSE, 0))
                    {
                        const LONG iWidth = rect.right - rect.left;
                        const LONG iHeight = rect.bottom - rect.top;
                        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &rect, 0))
                        {
                            if (const HWND hWnd = CreateWindowExW(0, g_pwWindowClass, L"StrikeoutText", (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
                                                                  (rect.left + rect.right - iWidth)/2,
                                                                  (rect.top + rect.bottom - iHeight)/2,
                                                                  iWidth,
                                                                  iHeight,
                                                                  nullptr, nullptr, wndCl.hInstance, &sArgs))
                            {
                                MSG msg;
                                while (GetMessageW(&msg, nullptr, 0, 0) > 0)
                                    if (!IsDialogMessageW(hWnd, &msg))
                                        DispatchMessageW(&msg);
                            }
                        }
                    }
                    UnregisterClassW(g_pwWindowClass, wndCl.hInstance);
                }
            }
            DeleteObject(sArgs.hFont);
        }
    }
    RtlExitUserProcess(0);
}
