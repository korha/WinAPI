//ExitWindows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const DWORD g_iReason = SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED;

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static bool FGetPrivilege()
{
    bool bOk = false;
    HANDLE hToken;
    if (NT_SUCCESS(NtOpenProcessToken(FGetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)))
    {
        TOKEN_PRIVILEGES tokenPrv;
        if (LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &tokenPrv.Privileges[0].Luid))
        {
            tokenPrv.PrivilegeCount = 1;
            tokenPrv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if (NT_SUCCESS(NtAdjustPrivilegesToken(hToken, FALSE, &tokenPrv, 0, nullptr, nullptr)))
                bOk = true;
        }
        NtClose(hToken);
    }
    return bOk;
}

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    const wchar_t *pwArgEnd;
    if (const wchar_t * const pwArg = FGetArgFromCmdLine(&FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine, &pwArgEnd))
        switch (pwArgEnd - pwArg)
        {
        case FCStrLen("/lock"):
            if (FStrIsStartsWith(pwArg, "/lock"))
                LockWorkStation();
            break;

        case FCStrLen("/sleep"):
            if (FStrIsStartsWith(pwArg, "/sleep") && FGetPrivilege())
                SetSuspendState(FALSE, FALSE, TRUE);
            break;

        case FCStrLen("/reboot"):
            if (FStrIsStartsWith(pwArg, "/reboot"))
            {
                if (FGetPrivilege())
                    ExitWindowsEx(EWX_REBOOT, g_iReason);
            }
            else if (FStrIsStartsWith(pwArg, "/logoff"))
            {
                if (FGetPrivilege())
                    ExitWindowsEx(EWX_LOGOFF, g_iReason);
            }
            else if (FStrIsStartsWith(pwArg, "/hybrid"))
            {
                if (FGetPrivilege())
                    ExitWindowsEx(EWX_SHUTDOWN | EWX_HYBRID_SHUTDOWN, g_iReason);
            }
            break;

        case FCStrLen("/shutdown"):
            if (FStrIsStartsWith(pwArg, "/shutdown") && FGetPrivilege())
                ExitWindowsEx(EWX_SHUTDOWN, g_iReason);
            break;

        case FCStrLen("/hibernate"):
            if (FStrIsStartsWith(pwArg, "/hibernate") && FGetPrivilege())
                SetSuspendState(TRUE, FALSE, TRUE);
            break;

        case FCStrLen("/reboot-force"):
            if (FStrIsStartsWith(pwArg, "/reboot-force"))
            {
                if (FGetPrivilege())
                    ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, g_iReason);
            }
            if (FStrIsStartsWith(pwArg, "/logoff-force"))
            {
                if (FGetPrivilege())
                    ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, g_iReason);
            }
            if (FStrIsStartsWith(pwArg, "/hybrid-force"))
            {
                if (FGetPrivilege())
                    ExitWindowsEx(EWX_SHUTDOWN | EWX_HYBRID_SHUTDOWN | EWX_FORCEIFHUNG, g_iReason);
            }
            break;

        case FCStrLen("/shutdown-force"):
            if (FStrIsStartsWith(pwArg, "/shutdown-force") && FGetPrivilege())
                ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCEIFHUNG, g_iReason);
            break;

        case FCStrLen("/sleep-allow-awake"):
            if (FStrIsStartsWith(pwArg, "/sleep-allow-awake") && FGetPrivilege())
                SetSuspendState(FALSE, FALSE, FALSE);
            break;

        case FCStrLen("/hibernate-allow-awake"):
            if (FStrIsStartsWith(pwArg, "/hibernate-allow-awake") && FGetPrivilege())
                SetSuspendState(TRUE, FALSE, FALSE);
            break;
        }
    RtlExitUserProcess(0);
}
