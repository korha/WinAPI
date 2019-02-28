//SetWallpaper
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    UNICODE_STRING * const pusCmdLine = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine;
    const wchar_t *pwArgEnd;
    if (wchar_t * const pwArg = FGetArgFromCmdLine(pusCmdLine, &pwArgEnd))
    {
        const size_t szLen = static_cast<size_t>(pwArgEnd - pusCmdLine->Buffer);
        if (szLen < pusCmdLine->MaximumLength/sizeof(wchar_t))
        {
            pusCmdLine->Buffer[szLen] = '\0';
            SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, pwArg, SPIF_UPDATEINIFILE);
        }
    }
    RtlExitUserProcess(0);
}
