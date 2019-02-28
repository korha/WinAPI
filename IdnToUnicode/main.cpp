//IdnToUnicode
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline wchar_t * FSkipProtocol(wchar_t *pwArg, const wchar_t * const pwArgEnd)
{
    wchar_t * const pwSave = pwArg;
    while (pwArg < pwArgEnd)
    {
        if (pwArg[0] == ':' && pwArg[1] == '/' && pwArg[2] == '/')
            return pwArg + FCStrLen("://");
        ++pwArg;
    }
    return pwSave;
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline const wchar_t * FGetArgEnd(const wchar_t *pwArg, const wchar_t * const pwArgEnd)
{
    while (pwArg < pwArgEnd)
    {
        if (*pwArg == '/' || *pwArg == ':' || *pwArg == ' ' || *pwArg == '\t')
            break;
        ++pwArg;
    }
    return pwArg;
}

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    const wchar_t *pwArgEnd;
    if (wchar_t *pwArg = FGetArgFromCmdLine(&FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine, &pwArgEnd))
        if ((pwArg = FSkipProtocol(pwArg, pwArgEnd - FCStrLen("://"))) < pwArgEnd)
        {
            pwArgEnd = FGetArgEnd(pwArg, pwArgEnd);
            int iLen = static_cast<int>(pwArgEnd - pwArg);
            iLen = IdnToUnicode(IDN_USE_STD3_ASCII_RULES, pwArg, iLen, pwArg, iLen);
            if (iLen > 0 && pwArgEnd - pwArg > iLen)
            {
                pwArg[iLen] = '\0';
                constexpr const wchar_t wCaption[] = {'I','d','n','T','o','U','n','i','c','o','d','e','\0'};
                MessageBoxTimeoutW(nullptr, pwArg, wCaption, MB_ICONINFORMATION, 0, INFINITE);
            }
        }
    RtlExitUserProcess(0);
}
