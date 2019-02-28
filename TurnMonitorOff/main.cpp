//TurnMonitorOff
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    PostMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
    RtlExitUserProcess(0);
}
