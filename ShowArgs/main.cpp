//ShowArgs
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    UNICODE_STRING * const pusCmdLine = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine;
    const size_t szLen = pusCmdLine->Length/sizeof(wchar_t);
    if (szLen < pusCmdLine->MaximumLength/sizeof(wchar_t))
    {
        wchar_t * const pwEnd = pusCmdLine->Buffer + szLen;
        *pwEnd = '\0';
        MessageBoxTimeoutW(nullptr, pusCmdLine->Buffer, pwEnd, MB_OK, 0, INFINITE);
    }
    RtlExitUserProcess(0);
}
