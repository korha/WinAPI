//HotKeys_CapsSwitch
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C"
void FMsg(const wchar_t * const pwArg)
{
    if (pwArg)
    {
        static BYTE iState;
        if (iState == 2)
            iState = 0;
        else
        {
            INPUT input[2];
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_CAPITAL;
            input[0].ki.wScan = 0;
            input[0].ki.dwFlags = 0;
            input[0].ki.time = 0;
            input[0].ki.dwExtraInfo = 0;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_CAPITAL;
            input[1].ki.wScan = 0;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            input[1].ki.time = 0;
            input[1].ki.dwExtraInfo = 0;
            SendInput(2, input, sizeof(INPUT));
            if (iState == 0)
                if (HWND hWnd = GetForegroundWindow())
                {
                    HWND hWndParent;
                    while ((hWndParent = GetParent(hWnd)))
                        hWnd = hWndParent;
                    PostMessageW(hWnd, WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_FORWARD, 0);
                }
            ++iState;
        }
    }
}
