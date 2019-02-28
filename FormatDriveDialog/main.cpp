//FormatDriveDialog
#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    const wchar_t *pwArgEnd;
    if (const wchar_t * const pwArg = FGetArgFromCmdLine(&FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine, &pwArgEnd))
        if (*pwArg >= 'A' && *pwArg <= 'Z')
            if (const HWND hWnd = GetDesktopWindow())
                SHFormatDrive(hWnd, static_cast<UINT>(*pwArg - 'A'), SHFMT_ID_DEFAULT, 0);
    RtlExitUserProcess(0);
}
