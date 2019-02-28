//HotKeys_WindowControl
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
enum class EAction : unsigned char {Terminate, Suspend, Resume};

//-------------------------------------------------------------------------------------------------
static void FQueryProcess(const EAction eAction)
{
    if (const HWND hWnd = GetForegroundWindow())
    {
        DWORD dwPid = ASFW_ANY;
        GetWindowThreadProcessId(hWnd, &dwPid);
        if (dwPid != ASFW_ANY)
        {
            HANDLE hProcess;
            EX_OBJECT_ATTRIBUTES objAttributes;
            CLIENT_ID clientId = {reinterpret_cast<HANDLE>(dwPid), nullptr};
            if (NT_SUCCESS(NtOpenProcess(&hProcess, eAction == EAction::Terminate ? PROCESS_TERMINATE : PROCESS_SUSPEND_RESUME, &objAttributes, &clientId)))
            {
                if (eAction == EAction::Suspend)
                    NtSuspendProcess(hProcess);
                else if (eAction == EAction::Resume)
                    NtResumeProcess(hProcess);
                else
                    NtTerminateProcess(hProcess, 0);
                NtClose(hProcess);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
extern "C"
void FMsg(const wchar_t *pwMsg)
{
    if (pwMsg && *pwMsg == '/')
    {
        ++pwMsg;
        if (FStrCompare(pwMsg, "suspend"))
            FQueryProcess(EAction::Suspend);
        else if (FStrCompare(pwMsg, "resume"))
            FQueryProcess(EAction::Resume);
        else if (FStrCompare(pwMsg, "minimize"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                ShowWindow(hWnd, SW_MINIMIZE);
        }
        else if (FStrCompare(pwMsg, "maximize"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                ShowWindow(hWnd, SW_MAXIMIZE);
        }
        else if (FStrCompare(pwMsg, "restore"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                ShowWindow(hWnd, SW_RESTORE);
        }
        else if (FStrCompare(pwMsg, "toggle-maximize"))
        {
            if (const HWND hWnd = GetForegroundWindow())
            {
                WINDOWPLACEMENT wndpl;
                wndpl.length = sizeof(WINDOWPLACEMENT);
                if (GetWindowPlacement(hWnd, &wndpl))
                    ShowWindow(hWnd, (wndpl.showCmd == SW_MAXIMIZE) ? SW_RESTORE : SW_MAXIMIZE);
            }
        }
        else if (FStrCompare(pwMsg, "topmost"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        else if (FStrCompare(pwMsg, "notopmost"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        else if (FStrCompare(pwMsg, "toggle-topmost"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                SetWindowPos(hWnd, (GetWindowLongPtrW(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        else if (FStrCompare(pwMsg, "caption-off"))
        {
            if (const HWND hWnd = GetForegroundWindow())
            {
                SetWindowLongPtrW(hWnd, GWL_STYLE, GetWindowLongPtrW(hWnd, GWL_STYLE) & ~WS_CAPTION);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        else if (FStrCompare(pwMsg, "caption-on"))
        {
            if (const HWND hWnd = GetForegroundWindow())
            {
                SetWindowLongPtrW(hWnd, GWL_STYLE, GetWindowLongPtrW(hWnd, GWL_STYLE) | WS_CAPTION);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        else if (FStrCompare(pwMsg, "toggle-caption"))
        {
            if (const HWND hWnd = GetForegroundWindow())
            {
                SetWindowLongPtrW(hWnd, GWL_STYLE, GetWindowLongPtrW(hWnd, GWL_STYLE) ^ WS_CAPTION);
                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        else if (FStrCompare(pwMsg, "resize"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                SetWindowLongPtrW(hWnd, GWL_STYLE, GetWindowLongPtrW(hWnd, GWL_STYLE) | WS_SIZEBOX);
        }
        else if (FStrCompare(pwMsg, "close"))
        {
            if (const HWND hWnd = GetForegroundWindow())
                PostMessageW(hWnd, WM_CLOSE, 0, 0);
        }
        else if (FStrCompare(pwMsg, "terminate"))
            FQueryProcess(EAction::Terminate);
        else if (FStrIsStartsWith(pwMsg, "opacity"))
        {
            pwMsg += FCStrLen("opacity");
            if ((*pwMsg == '+' || *pwMsg == '-' || *pwMsg == '=') &&
                    (pwMsg[1] >= '0' && pwMsg[1] <= '2') &&
                    (pwMsg[2] >= '0' && pwMsg[2] <= '9') &&
                    (pwMsg[3] >= '0' && pwMsg[3] <= '9') &&
                    pwMsg[4] == '\0')
            {
                int iOpacity = (pwMsg[1] - '0')*100 + (pwMsg[2] - '0')*10 + (pwMsg[3] - '0');
                if (iOpacity <= 255)
                    if (const HWND hWnd = GetForegroundWindow())
                    {
                        DWORD dwFlags;
                        BYTE btAlpha;
                        if (*pwMsg == '+')
                        {
                            if (GetLayeredWindowAttributes(hWnd, nullptr, &btAlpha, &dwFlags) && (dwFlags & LWA_ALPHA))
                            {
                                iOpacity += btAlpha;
                                if (iOpacity > 255)
                                    iOpacity = 255;
                                SetLayeredWindowAttributes(hWnd, 0, static_cast<BYTE>(iOpacity), LWA_ALPHA);
                            }
                        }
                        else
                        {
                            if (*pwMsg == '-')
                            {
                                if (GetLayeredWindowAttributes(hWnd, nullptr, &btAlpha, &dwFlags) && (dwFlags & LWA_ALPHA))
                                    iOpacity = (btAlpha > iOpacity) ? (btAlpha - iOpacity) : 0;
                                else
                                {
                                    SetWindowLongPtrW(hWnd, GWL_EXSTYLE, GetWindowLongPtrW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
                                    iOpacity = 255 - iOpacity;
                                }
                            }
                            else if (!(GetLayeredWindowAttributes(hWnd, nullptr, nullptr, &dwFlags) && (dwFlags & LWA_ALPHA)))
                                SetWindowLongPtrW(hWnd, GWL_EXSTYLE, GetWindowLongPtrW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
                            SetLayeredWindowAttributes(hWnd, 0, static_cast<BYTE>(iOpacity), LWA_ALPHA);
                        }
                    }
            }
        }
    }
}
