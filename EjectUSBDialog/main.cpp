//EjectUSBDialog
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    wchar_t wBuf[sizeof("C:\\Windows\\System32\\hotplug.dll")];
    if (GetSystemDirectoryW(wBuf, sizeof("C:\\Windows\\System32")) == FCStrLen("C:\\Windows\\System32"))
        if (const HWND hWnd = GetDesktopWindow())
        {
            constexpr const char cDllName[] = {'\\','h','o','t','p','l','u','g','.','d','l','l','\0'};
            FStrCopy(wBuf + FCStrLen("C:\\Windows\\System32"), cDllName);
            Control_RunDLLW(hWnd, nullptr, wBuf, SW_SHOW);
        }
    RtlExitUserProcess(0);
}
