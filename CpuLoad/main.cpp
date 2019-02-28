//CpuLoad
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const wchar_t * const g_pwWindowClass = L"aux1yvno84pa877dtypi4h7stsx8uwg2";
static constexpr const int g_iWidth = 244;
static constexpr const int g_iHeight = 75;
static constexpr const int g_iMaxNumOfProcessors = 64;

struct [[nodiscard]] SArgs final
{
    HICON hIconStart, hIconStop;
    HWND hWndEdit, hWndUpDown, hWndBtn;
    HFONT hFont;
    HANDLE hThreads[g_iMaxNumOfProcessors];
    BYTE iMaxNumOfThreads;
    BYTE iNumOfThreads;
    bool bActive;
    BYTE btPad[sizeof(HANDLE) - (sizeof(BYTE) + sizeof(BYTE) + sizeof(bool))];

    explicit SArgs() = default;
    SArgs(const SArgs &) = delete;
    SArgs & operator=(const SArgs &) = delete;
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static DWORD WINAPI FThreadProc(void *pbActive)
{
    while (*static_cast<const volatile bool*>(pbActive));
    return 0;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LRESULT CALLBACK FWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    enum
    {
        eBtn = 1
    };

    static SArgs *psArgs;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        const CREATESTRUCT * const pCrStruct = reinterpret_cast<const CREATESTRUCT*>(lParam);
        psArgs = static_cast<SArgs*>(pCrStruct->lpCreateParams);
        if (const HWND hWndStatic = CreateWindowExW(0, WC_STATICW, L"Threads:", WS_CHILD | WS_VISIBLE, 65, 14, 52, 14, hWnd, nullptr, pCrStruct->hInstance, nullptr))
            if ((psArgs->hWndEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_NUMBER, 118, 11, 41, 20, hWnd, nullptr, pCrStruct->hInstance, nullptr)) &&
                    (psArgs->hWndUpDown = CreateWindowExW(0, UPDOWN_CLASSW, nullptr, WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS, 0, 0, 0, 0, hWnd, nullptr, pCrStruct->hInstance, nullptr)) &&
                    (psArgs->hWndBtn = CreateWindowExW(0, WC_BUTTONW, L"Start", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 84, 40, 75, 22, hWnd, reinterpret_cast<HMENU>(eBtn), pCrStruct->hInstance, nullptr)) &&
                    SetFocus(psArgs->hWndBtn))
            {
                SendMessageW(psArgs->hWndUpDown, UDM_SETRANGE, 0, MAKELPARAM(psArgs->iMaxNumOfThreads, 1));
                SendMessageW(psArgs->hWndUpDown, UDM_SETPOS, 0, 1);

                SendMessageW(hWndStatic, WM_SETFONT, reinterpret_cast<WPARAM>(psArgs->hFont), FALSE);
                SendMessageW(psArgs->hWndEdit, WM_SETFONT, reinterpret_cast<WPARAM>(psArgs->hFont), FALSE);
                SendMessageW(psArgs->hWndUpDown, WM_SETFONT, reinterpret_cast<WPARAM>(psArgs->hFont), FALSE);
                SendMessageW(psArgs->hWndBtn, WM_SETFONT, reinterpret_cast<WPARAM>(psArgs->hFont), FALSE);

                SendMessageW(psArgs->hWndBtn, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(psArgs->hIconStart));

                psArgs->bActive = false;
                return 0;
            }
        return -1;
    }
    case WM_COMMAND:
    {
        if (wParam == eBtn)
        {
            if (psArgs->bActive)
            {
                psArgs->bActive = false;
                SendMessageW(psArgs->hWndBtn, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(L"Start"));
                SendMessageW(psArgs->hWndBtn, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(psArgs->hIconStart));
                for (const HANDLE *phIt = psArgs->hThreads, * const phEnd = phIt + psArgs->iNumOfThreads; phIt < phEnd; ++phIt)
                    if (*phIt)
                    {
                        NtWaitForSingleObject(*phIt, FALSE, nullptr);
                        NtClose(*phIt);
                    }
                EnableWindow(psArgs->hWndEdit, TRUE);
                EnableWindow(psArgs->hWndUpDown, TRUE);
            }
            else
            {
                const LRESULT iNewNumOfThreads = SendMessageW(psArgs->hWndUpDown, UDM_GETPOS, 0, 0);
                if (iNewNumOfThreads > 0 && iNewNumOfThreads <= psArgs->iMaxNumOfThreads)
                {
                    EnableWindow(psArgs->hWndUpDown, FALSE);
                    EnableWindow(psArgs->hWndEdit, FALSE);
                    SendMessageW(psArgs->hWndBtn, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(L"Stop"));
                    SendMessageW(psArgs->hWndBtn, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(psArgs->hIconStop));
                    psArgs->iNumOfThreads = static_cast<BYTE>(iNewNumOfThreads);
                    psArgs->bActive = true;
                    for (HANDLE *phIt = psArgs->hThreads, * const phEnd = phIt + psArgs->iNumOfThreads; phIt < phEnd; ++phIt)
                        if (!NT_SUCCESS(NtCreateThreadEx(phIt, THREAD_ALL_ACCESS, nullptr, FGetCurrentProcess(), FThreadProc, &psArgs->bActive, 0, 0, 0, 0, nullptr)))
                            *phIt = nullptr;
                }
                else
                    SendMessageW(psArgs->hWndUpDown, UDM_SETPOS, 0, 1);
            }
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
        if (psArgs->bActive)
        {
            psArgs->bActive = false;
            for (const HANDLE *phIt = psArgs->hThreads, * const phEnd = phIt + psArgs->iNumOfThreads; phIt < phEnd; ++phIt)
                if (*phIt)
                {
                    NtWaitForSingleObject(*phIt, FALSE, nullptr);
                    NtClose(*phIt);
                }
        }
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
    const EX_TEB::EX_PEB * const pPeb = FGetPTeb()->ProcessEnvironmentBlock;
    const ULONG iNumberOfProcessors = pPeb->NumberOfProcessors;
    if (iNumberOfProcessors && iNumberOfProcessors <= g_iMaxNumOfProcessors)
    {
        NONCLIENTMETRICSW nonClientMetrics;
        nonClientMetrics.cbSize = sizeof(NONCLIENTMETRICSW);
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &nonClientMetrics, 0))
        {
            SArgs sArgs;
            if ((sArgs.hFont = CreateFontIndirectW(&nonClientMetrics.lfMessageFont)))
            {
                WNDCLASSEXW wndCl;
                if ((wndCl.hCursor = static_cast<HCURSOR>(LoadImageW(nullptr, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED))))
                {
                    wndCl.hInstance = pPeb->ImageBaseAddress;
                    if ((wndCl.hIcon = static_cast<HICON>(LoadImageW(wndCl.hInstance, L"IDI_ICON1", IMAGE_ICON, 32, 32, 0))))
                    {
                        if ((wndCl.hIconSm = static_cast<HICON>(LoadImageW(wndCl.hInstance, L"IDI_ICON1", IMAGE_ICON, 16, 16, 0))))
                        {
                            if ((sArgs.hIconStart = static_cast<HICON>(LoadImageW(wndCl.hInstance, L"IDI_ICON2", IMAGE_ICON, 16, 16, 0))))
                            {
                                if ((sArgs.hIconStop = static_cast<HICON>(LoadImageW(wndCl.hInstance, L"IDI_ICON3", IMAGE_ICON, 16, 16, 0))))
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
                                        RECT rect = {0, 0, g_iWidth, g_iHeight};
                                        if (AdjustWindowRectEx(&rect, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX), FALSE, 0))
                                        {
                                            const LONG iWidth = rect.right - rect.left;
                                            const LONG iHeight = rect.bottom - rect.top;
                                            if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &rect, 0))
                                            {
                                                sArgs.iMaxNumOfThreads = static_cast<BYTE>(iNumberOfProcessors);
                                                if (const HWND hWnd = CreateWindowExW(0, g_pwWindowClass, L"CpuLoad", (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_MAXIMIZEBOX | WS_SIZEBOX),
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
                                    DestroyIcon(sArgs.hIconStop);
                                }
                                DestroyIcon(sArgs.hIconStart);
                            }
                            DestroyIcon(wndCl.hIconSm);
                        }
                        DestroyIcon(wndCl.hIcon);
                    }
                }
                DeleteObject(sArgs.hFont);
            }
        }
    }
    RtlExitUserProcess(0);
}
