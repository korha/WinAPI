//CheckDll
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
        EX_USTRING usLib(pwArg, pwArgEnd);
        void *pDllHandle;
        const bool bOk = NT_SUCCESS(LdrLoadDll(nullptr, nullptr, &usLib, &pDllHandle));
        if (bOk)
            LdrUnloadDll(pDllHandle);

#ifdef _WIN64
        constexpr const wchar_t wAppName[] = {'C','h','e','c','k','D','l','l','6','4','\0'};
#else
        constexpr const wchar_t wAppName[] = {'C','h','e','c','k','D','l','l','3','2','\0'};
#endif
        const wchar_t *pwText = nullptr;
        const size_t szLen = static_cast<size_t>(pwArgEnd - pusCmdLine->Buffer);
        if (szLen < pusCmdLine->MaximumLength/sizeof(wchar_t))
        {
            pusCmdLine->Buffer[szLen] = '\0';
            pwText = pwArg;
        }
        else
            pwText = wAppName + FCCountOf(wAppName) - 1;
        MessageBoxTimeoutW(nullptr, pwText, wAppName, bOk ? MB_ICONINFORMATION : MB_ICONERROR, 0, INFINITE);
    }
    RtlExitUserProcess(0);
}
