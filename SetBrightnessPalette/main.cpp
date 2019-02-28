//SetBrightness
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    const wchar_t *pwArgEnd;
    if (const wchar_t * const pwArg = FGetArgFromCmdLine(&FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine, &pwArgEnd))
        if (pwArgEnd == pwArg + FCStrLen("+127") &&
                (pwArg[0] == '-' || pwArg[0] == '+') &&
                pwArg[1] >= '0' && pwArg[1] <= '1' &&
                pwArg[2] >= '0' && pwArg[2] <= '9' &&
                pwArg[3] >= '0' && pwArg[3] <= '9')
        {
            int iPreLevel = (pwArg[1] - '0')*100 + (pwArg[2] - '0')*10 + (pwArg[3] - '0');
            if (*pwArg == '-')
                iPreLevel = -iPreLevel;
            if (iPreLevel >= -128 && iPreLevel <= 127)
            {
                const uint iLevel = static_cast<uint>(iPreLevel + 256);
                WORD wGamma[3*256];
                for (uint i = 0; i < 256; ++i)
                {
                    uint iValue = i*iLevel;
                    if (iValue > 65535)
                        iValue = 65535;
                    wGamma[i] = static_cast<WORD>(iValue);
                }
                FBytesCopyEx(wGamma + 1*256, wGamma, sizeof(WORD)*256);
                FBytesCopyEx(wGamma + 2*256, wGamma, sizeof(WORD)*256);
                if (const HDC hDc = GetDC(nullptr))
                {
                    SetDeviceGammaRamp(hDc, wGamma);
                    ReleaseDC(nullptr, hDc);
                }
            }
        }
    RtlExitUserProcess(0);
}
