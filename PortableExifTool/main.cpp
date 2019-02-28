//PortableExifTool
//Fix import table:
//exiftool.exe: advapi32.dll
//#define U_EXTENDED_VERSION
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\par-41646d696e\\cache-exiftool-??.??\\inc\\lib\\auto\\Math\\BigInt\\FastCalc\\FastCalc.dll");

//-------------------------------------------------------------------------------------------------
extern "C"
{
//advapi32.dll
WINBOOL WINAPI GetUserNameA_Stub(LPSTR, LPDWORD)
{
    return FALSE;
}
}

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
#ifdef _WIN64
    static_assert(false, "only 32-bit");
#endif
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        const EX_RTL_USER_PROCESS_PARAMETERS * const pProcParams = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters;
        if (FGetImageDirLen(&pProcParams->ImagePathName, g_iDirMaxLen) >= FCStrLen("C:\\A"))
        {
            constexpr const char cEnvName[] = {'U','_','P','O','R','T','A','B','L','E'};
            if (const wchar_t * const pwEnvValue = FGetEnvValue(pProcParams->Environment, cEnvName, sizeof(cEnvName)))
                if (*pwEnvValue == '1' && pwEnvValue[1] == '\0' &&
                        NT_SUCCESS(LdrDisableThreadCalloutsForDll(hInstDll)))
                    return TRUE;
        }
    }
#ifdef U_EXTENDED_VERSION
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        const HANDLE hStdIn = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->StandardInput;
        if (hStdIn != INVALID_HANDLE_VALUE)
        {
            wchar_t wChar;
            DWORD dwBytes;
            ReadConsoleW(hStdIn, &wChar, 1, &dwBytes, nullptr);
        }
    }
#endif
    return FALSE;
}
