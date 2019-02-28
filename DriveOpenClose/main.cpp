//DriveOpenClose
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    const wchar_t *pwArgEnd;
    if (const wchar_t * const pwArg = FGetArgFromCmdLine(&FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine, &pwArgEnd))
        if (pwArgEnd == pwArg + FCStrLen("+A") &&
                pwArg[1] >= 'A' && pwArg[1] <= 'Z')
        {
            if (*pwArg == '+')
            {
                const wchar_t wCommand[] = {'s','e','t',' ','c','d','a','u','d','i','o','!',static_cast<wchar_t>(pwArg[1] + ('a' - 'A')),':',' ','d','o','o','r',' ','o','p','e','n',' ','w','a','i','t','\0'};
                mciSendStringW(wCommand, nullptr, 0, nullptr);
            }
            else if (*pwArg == '-')
            {
                const wchar_t wCommand[] = {'s','e','t',' ','c','d','a','u','d','i','o','!',static_cast<wchar_t>(pwArg[1] + ('a' - 'A')),':',' ','d','o','o','r',' ','c','l','o','s','e','d',' ','w','a','i','t','\0'};
                mciSendStringW(wCommand, nullptr, 0, nullptr);
            }
        }
    RtlExitUserProcess(0);
}
