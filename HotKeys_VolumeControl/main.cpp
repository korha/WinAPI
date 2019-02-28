//HotKeys_VolumeControl
//#define U_MULTIMONITOR_MAX 16
#define WIN32_LEAN_AND_MEAN
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
enum class EVolume
{
    Null,
    MuteToggle,
    MuteSet,
    MuteUnset,
    Mute,
    LevelAdd,
    LevelSub,
    LevelSet
};

static constexpr const wchar_t * const g_pwWindowClass = L"nf3crme0y6fy2iysbjejva2ts9dycf40";
static constexpr const int g_iCellHeight = 144;
static constexpr const int g_iElapseTimer = 1200;
#ifdef U_MULTIMONITOR_MAX
static constexpr const uint g_iMaxNumOfMonitors = U_MULTIMONITOR_MAX;
#else
static constexpr const uint g_iMaxNumOfMonitors = 4;
#endif
static HWND g_hWnd[g_iMaxNumOfMonitors];
static HINSTANCE g_hInstDll;
static HFONT g_hFont;
static BYTE g_iVolume;
static bool g_bRegWndClass;
static bool g_bMute;

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static LRESULT CALLBACK FWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static WORD iCxN[g_iMaxNumOfMonitors], iCyN[g_iMaxNumOfMonitors];

    switch (uMsg)
    {
    case WM_CREATE: return SetLayeredWindowAttributes(hWnd, 0, 0, LWA_COLORKEY) ? 0 : -1;
    case WM_PAINT:
    {
        for (uint i = 0; i < g_iMaxNumOfMonitors; ++i)
            if (g_hWnd[i] == hWnd)
            {
                PAINTSTRUCT ps;
                if (const HDC hDc = BeginPaint(hWnd, &ps))
                {
                    if (const HDC hDcMem = CreateCompatibleDC(hDc))
                    {
                        const int iCx = iCxN[i];
                        const int iCy = iCyN[i];
                        if (const HBITMAP hBmpMem = CreateCompatibleBitmap(hDc, iCx, iCy))
                        {
                            wchar_t wBuf[FCStrLen("100%")];
                            int iLen;
                            if (g_iVolume < 10)
                            {
                                wBuf[0] = static_cast<wchar_t>('0' + g_iVolume);
                                wBuf[1] = '%';
                                iLen = FCStrLen("9%");
                            }
                            else if (g_iVolume < 100)
                            {
                                wBuf[0] = static_cast<wchar_t>('0' + g_iVolume/10);
                                wBuf[1] = static_cast<wchar_t>('0' + g_iVolume%10);
                                wBuf[2] = '%';
                                iLen = FCStrLen("99%");
                            }
                            else
                            {
                                wBuf[0] = '1';
                                wBuf[1] = '0';
                                wBuf[2] = '0';
                                wBuf[3] = '%';
                                iLen = FCStrLen("100%");
                            }
                            RECT rect = {0, iCy - (g_iCellHeight + g_iCellHeight/2), iCx, iCy};
                            SelectObject(hDcMem, hBmpMem);
                            SelectObject(hDcMem, g_hFont);
                            SetBkColor(hDcMem, RGB(1, 1, 1));
                            SetTextColor(hDcMem, g_bMute ? RGB(255, 0, 0) : RGB(0, 255, 0));
                            DrawTextW(hDcMem, wBuf, iLen, &rect, DT_CENTER);
                            BitBlt(hDc, 0, 0, iCx, iCy, hDcMem, 0, 0, SRCCOPY);
                            DeleteObject(hBmpMem);
                        }
                        DeleteDC(hDcMem);
                    }
                    EndPaint(hWnd, &ps);
                }
                break;
            }
        return 0;
    }
    case WM_SIZE:
    {
        for (uint i = 0; i < g_iMaxNumOfMonitors; ++i)
            if (g_hWnd[i] == hWnd)
            {
                iCxN[i] = LOWORD(lParam);
                iCyN[i] = HIWORD(lParam);
                break;
            }
        return 0;
    }
    case WM_TIMER:
    {
        KillTimer(hWnd, 1);
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    }
    case WM_DESTROY:
    {
        HWND *phWndIt = g_hWnd;
        const HWND * const phWndEnd = FCGetEnd(g_hWnd);
        while (phWndIt < phWndEnd)
        {
            if (*phWndIt == hWnd)
            {
                *phWndIt = nullptr;
                break;
            }
            ++phWndIt;
        }
        return 0;
    }
    }
    return NtdllDefWindowProc_W(hWnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static bool FChangeVolume(const EVolume eVolume, int iVolume)
{
    bool bOk = false;
    IMMDeviceEnumerator *immDeviceEnumerator;
    if (CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&immDeviceEnumerator)) == S_OK)
    {
        IMMDevice *immDeviceDefault;
        HRESULT hr = immDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &immDeviceDefault);
        immDeviceEnumerator->Release();
        if (hr == S_OK)
        {
            IAudioEndpointVolume *iAudioEndpointVolume;
            hr = immDeviceDefault->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void**>(&iAudioEndpointVolume));
            immDeviceDefault->Release();
            if (hr == S_OK)
            {
                FLOAT fLevel;
                WINBOOL bMute;
                if (eVolume == EVolume::MuteToggle)
                {
                    if (iAudioEndpointVolume->GetMute(&bMute) == S_OK &&
                            iAudioEndpointVolume->SetMute(bMute = !bMute, nullptr) == S_OK &&
                            iAudioEndpointVolume->GetMasterVolumeLevelScalar(&fLevel) == S_OK)
                    {
                        g_bMute = bMute;
                        g_iVolume = static_cast<BYTE>(fLevel*100.0f + 0.001f);        //add a small value for avoid incorrect truncation
                        bOk = true;
                    }
                }
                else if (eVolume == EVolume::LevelAdd || eVolume == EVolume::LevelSub)
                {
                    if (iAudioEndpointVolume->GetMasterVolumeLevelScalar(&fLevel) == S_OK)
                    {
                        const int iPrevVolume = static_cast<int>(fLevel*100.0f + 0.001f);        //add a small value for avoid incorrect truncation
                        if (eVolume == EVolume::LevelAdd)
                        {
                            iVolume += iPrevVolume;
                            if (iVolume > 100)
                                iVolume = 100;
                        }
                        else
                            iVolume = (iPrevVolume > iVolume) ? (iPrevVolume - iVolume) : 0;
                        if (iAudioEndpointVolume->SetMasterVolumeLevelScalar(static_cast<float>(iVolume)/100.0f, nullptr) == S_OK &&
                                iAudioEndpointVolume->GetMute(&bMute) == S_OK)
                        {
                            g_bMute = bMute;
                            g_iVolume = static_cast<BYTE>(iVolume);
                            bOk = true;
                        }
                    }
                }
                else if (eVolume == EVolume::LevelSet)
                {
                    if (iAudioEndpointVolume->SetMasterVolumeLevelScalar(static_cast<float>(iVolume)/100.0f, nullptr) == S_OK &&
                            iAudioEndpointVolume->GetMute(&bMute) == S_OK)
                    {
                        g_bMute = bMute;
                        g_iVolume = static_cast<BYTE>(iVolume);
                        bOk = true;
                    }
                }
                else        //if (eVolume == EVolume::MuteSet || eVolume == EVolume::MuteUnset)
                {
                    bMute = eVolume == EVolume::MuteSet;
                    if (iAudioEndpointVolume->SetMute(bMute, nullptr) == S_OK && iAudioEndpointVolume->GetMasterVolumeLevelScalar(&fLevel) == S_OK)
                    {
                        g_bMute = bMute;
                        g_iVolume = static_cast<BYTE>(fLevel*100.0f + 0.001f);        //add a small value for avoid incorrect truncation
                        bOk = true;
                    }
                }
                iAudioEndpointVolume->Release();
            }
        }
    }
    return bOk;
}

[[nodiscard]]
static HWND FGetWnd(const uint iIndex)
{
    HWND * const phWnd = g_hWnd + iIndex;
    if (!*phWnd)
    {
        if (!g_bRegWndClass)
        {
            WNDCLASSEXW wndCl;
            wndCl.cbSize = sizeof(WNDCLASSEXW);
            wndCl.style = 0;
            wndCl.lpfnWndProc = FWindowProc;
            wndCl.cbClsExtra = 0;
            wndCl.cbWndExtra = 0;
            wndCl.hInstance = g_hInstDll;
            wndCl.hIcon = nullptr;
            wndCl.hCursor = nullptr;
            wndCl.hbrBackground = nullptr;
            wndCl.lpszMenuName = nullptr;
            wndCl.lpszClassName = g_pwWindowClass;
            wndCl.hIconSm = nullptr;
            if (RegisterClassExW(&wndCl))
                g_bRegWndClass = true;
        }
        if (g_bRegWndClass &&
                (g_hFont || (g_hFont = CreateFontW(g_iCellHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Tahoma"))))
            *phWnd = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST, g_pwWindowClass, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, g_hInstDll, nullptr);
    }
    return *phWnd;
}

[[nodiscard]]
static WINBOOL CALLBACK FMonitorProc(HMONITOR, HDC, LPRECT lpfnEnum, LPARAM dwData)
{
    uint * const piIndex = reinterpret_cast<uint*>(dwData);
    if (const HWND hWnd = FGetWnd(*piIndex))
    {
        if (SetWindowPos(hWnd, HWND_TOP, lpfnEnum->left, lpfnEnum->top, 0, 0, SWP_NOSIZE | SWP_NOZORDER) &&
                SetTimer(hWnd, 1, g_iElapseTimer, nullptr))
        {
            InvalidateRect(hWnd, nullptr, FALSE);
            ShowWindow(hWnd, SW_SHOWMAXIMIZED);
        }
        return ++*piIndex < g_iMaxNumOfMonitors;
    }
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
extern "C"
{
void FMsg(const wchar_t *pwMsg)
{
    if (pwMsg)
    {
        EVolume eVolume = EVolume::Null;
        int iValue = 0;
        if (*pwMsg == '/')
        {
            ++pwMsg;
            if (FStrCompare(pwMsg, "toggle-mute"))
                eVolume = EVolume::MuteToggle;
            else if (FStrCompare(pwMsg, "mute"))
                eVolume = EVolume::MuteSet;
            else if (FStrCompare(pwMsg, "unmute"))
                eVolume = EVolume::MuteUnset;
        }
        else if (*pwMsg &&
                 ((pwMsg[1] == '0' && pwMsg[2] >= '0' && pwMsg[2] <= '9' && pwMsg[3] >= '0' && pwMsg[3] <= '9') ||
                  (pwMsg[1] == '1' && pwMsg[2] == '0' && pwMsg[3] == '0')) &&
                 pwMsg[4] == '\0')
        {
            iValue = (pwMsg[1] - '0')*100 + (pwMsg[2] - '0')*10 + (pwMsg[3] - '0');
            if (*pwMsg == '+')
                eVolume = EVolume::LevelAdd;
            else if (*pwMsg == '-')
                eVolume = EVolume::LevelSub;
            else if (*pwMsg == '=')
                eVolume = EVolume::LevelSet;
        }
        if (eVolume != EVolume::Null && FChangeVolume(eVolume, iValue))
        {
            uint iIndex = 0;
            EnumDisplayMonitors(nullptr, nullptr, FMonitorProc, reinterpret_cast<LPARAM>(&iIndex));
        }
    }
    else
    {
        HWND *phWndIt = g_hWnd;
        const HWND * const phWndEnd = FCGetEnd(g_hWnd);
        while (phWndIt < phWndEnd)
        {
            if (*phWndIt)
                SendMessageW(*phWndIt, WM_CLOSE, 0, 0);
            ++phWndIt;
        }
        if (g_bRegWndClass)
        {
            UnregisterClassW(g_pwWindowClass, g_hInstDll);
            g_bRegWndClass = false;
        }
        if (g_hFont)
        {
            DeleteObject(g_hFont);
            g_hFont = nullptr;
        }
    }
}

void FIsNeedCom()
{
}

BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
    static_assert(g_iMaxNumOfMonitors >= 1 && g_iMaxNumOfMonitors <= 256, "unsupported numbers of monitors");
    if (fdwReason == DLL_PROCESS_ATTACH)
        g_hInstDll = hInstDll;
    return TRUE;
}
}
