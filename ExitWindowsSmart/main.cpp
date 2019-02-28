//ExitWindowsSmart
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <ntstatus.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const DWORD g_dwReason = SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED;

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FIsProgramManagerWindow(const HWND hWnd, wchar_t * const pwBuf, const LRESULT iLen)
{
    return iLen == static_cast<LRESULT>(FCStrLen("Program Manager")) &&
            FStrCompare(pwBuf, "Program Manager") &&
            GetClassNameW(hWnd, pwBuf, sizeof("Progman\0")) == static_cast<int>(FCStrLen("Progman")) &&
            FStrCompare(pwBuf, "Progman");
}

[[nodiscard]]
static WINBOOL CALLBACK FEnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    if (IsWindowVisible(hWnd))
    {
        const LRESULT iLen = SendMessageW(hWnd, WM_GETTEXT, sizeof("Program Manager\0"), lParam);
        if (iLen > 0 && !FIsProgramManagerWindow(hWnd, reinterpret_cast<wchar_t*>(lParam), iLen))
            PostMessageW(hWnd, WM_CLOSE, 0, 0);
    }
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
static void FResetInputLanguage()
{
    if (HWND hWnd = GetForegroundWindow())
    {
        HWND hWndParent;
        while ((hWndParent = GetParent(hWnd)))
            hWnd = hWndParent;
        PostMessageW(hWnd, WM_INPUTLANGCHANGEREQUEST, 0, reinterpret_cast<LPARAM>(LoadKeyboardLayoutW(L"00000409", KLF_NOTELLSHELL)));
    }
}

//-------------------------------------------------------------------------------------------------
static void FVolumeMute()
{
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
    {
        IMMDeviceEnumerator *immDeviceEnumerator;
        if (CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&immDeviceEnumerator)) == S_OK)
        {
            IMMDevice *immDeviceDefault;
            HRESULT hr = immDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &immDeviceDefault);
            immDeviceEnumerator->Release();
            if (hr == S_OK)
            {
                IAudioEndpointVolume *iAudioEndpointVolume;
                hr = immDeviceDefault->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, reinterpret_cast<void**>(&iAudioEndpointVolume));
                immDeviceDefault->Release();
                if (hr == S_OK)
                {
                    iAudioEndpointVolume->SetMute(TRUE, nullptr);
                    iAudioEndpointVolume->SetMasterVolumeLevelScalar(0, nullptr);
                    iAudioEndpointVolume->Release();
                }
            }
        }
        CoUninitialize();
    }
}

//-------------------------------------------------------------------------------------------------
static void FResetBrightnessPalette()
{
    WORD wGamma[3*256];
    for (uint i = 0; i < 256; ++i)
        wGamma[i] = static_cast<WORD>(i*256);
    FBytesCopyEx(wGamma + 1*256, wGamma, sizeof(WORD)*256);
    FBytesCopyEx(wGamma + 2*256, wGamma, sizeof(WORD)*256);
    if (const HDC hDc = GetDC(nullptr))
    {
        SetDeviceGammaRamp(hDc, wGamma);
        ReleaseDC(nullptr, hDc);
    }
}

//-------------------------------------------------------------------------------------------------
static void FRandomize(const bool bIsShutdown)
{
    constexpr const uint iNetworkUuidLen = FCStrLen("{01234567-89AB-CDEF-0123-456789ABCDEF}");
    const BYTE * const pbtVal = pointer_cast<const BYTE*>("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    BYTE btRandom[FCStrLen("0123456789AB")*2 + FCStrLen("0123456")];
    bool bRandomInit = false;

    HANDLE hKeyList;
    wchar_t wKey[FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\{00000000-0000-0000-0000-000000000000}\\Connection")];
    FStrCopyEx(wKey, "\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\{00000000-0000-0000-0000-000000000000}\\Connection");
    EX_USTRING usKey(wKey, FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}")*sizeof(wchar_t));
    EX_OBJECT_ATTRIBUTES objAttributes(&usKey, true);
    if (NT_SUCCESS(NtOpenKeyEx(&hKeyList, KEY_ENUMERATE_SUB_KEYS, &objAttributes, 0)))
    {
        wchar_t wValueName[] = L"Name";
        EX_USTRING usValueName(wValueName, true);
        usKey.MaximumLength = usKey.Length = iNetworkUuidLen*sizeof(wchar_t);
        EX_KEY_VALUE_PARTIAL_INFORMATION<iNetworkUuidLen> keyValuePartialInfo;
        wchar_t wUuidWlan0[iNetworkUuidLen];
        wchar_t wUuidEth0[iNetworkUuidLen];
        *wUuidWlan0 = '\0';
        *wUuidEth0 = '\0';

        ULONG iIndex = 0;
        EX_KEY_BASIC_INFORMATION<iNetworkUuidLen> keyBasicInfo;
        ULONG iResultLength;
        while (NtEnumerateKey(hKeyList, iIndex++, KeyBasicInformation, &keyBasicInfo, sizeof(KEY_BASIC_INFORMATION) + iNetworkUuidLen*sizeof(wchar_t), &iResultLength) == STATUS_SUCCESS)
            if (keyBasicInfo.NameLength == iNetworkUuidLen*sizeof(wchar_t))
            {
                FStrCopyEx(wKey + FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\"), keyBasicInfo.Name, iNetworkUuidLen);
                HANDLE hKey;
                usKey.MaximumLength = usKey.Length = sizeof(wKey);
                if (NT_SUCCESS(NtOpenKeyEx(&hKey, KEY_READ, &objAttributes, 0)))
                {
                    if (NtQueryValueKey(hKey, &usValueName, KeyValuePartialInformation, &keyValuePartialInfo, sizeof(KEY_VALUE_PARTIAL_INFORMATION) + FCMax(sizeof(L"wlan0"), sizeof(L"eth0")), &iResultLength) == STATUS_SUCCESS &&
                            keyValuePartialInfo.Type == REG_SZ)
                    {
                        if (!*wUuidWlan0 && keyValuePartialInfo.DataLength == sizeof(L"wlan0") && FStrCompare(keyValuePartialInfo.Data, "wlan0"))
                            FStrCopyEx(wUuidWlan0, keyBasicInfo.Name, iNetworkUuidLen);
                        else if (!*wUuidEth0 && keyValuePartialInfo.DataLength == sizeof(L"eth0") && FStrCompare(keyValuePartialInfo.Data, "eth0"))
                            FStrCopyEx(wUuidEth0, keyBasicInfo.Name, iNetworkUuidLen);
                    }
                    NtClose(hKey);
                    if (*wUuidWlan0 && *wUuidEth0)
                        break;
                }
            }
        NtClose(hKeyList);
        if ((*wUuidWlan0 || *wUuidEth0) &&
                BCryptGenRandom(nullptr, btRandom, static_cast<ULONG>(bIsShutdown ? (FCStrLen("0123456789AB")*2 + FCStrLen("0123456")) : FCStrLen("0123456789AB")*2), BCRYPT_USE_SYSTEM_PREFERRED_RNG) == STATUS_SUCCESS)
        {
            bRandomInit = true;
            *btRandom = static_cast<BYTE>((*btRandom | 2U) & ~1U);        //Windows WLAN adapter MAC address restriction
            wchar_t wMacs[12*2];
            const BYTE *pbtSrc = btRandom;
            wchar_t *pwDst = wMacs;
            const wchar_t * const pwEnd = wMacs + sizeof(wMacs)/sizeof(wchar_t);
            while (pwDst < pwEnd)
            {
                *pwDst++ = pbtVal[*pbtSrc >> 4];
                *pwDst++ = pbtVal[*pbtSrc++ & 0xF];
            }
            FStrCopyEx(FStrCopyEx(FStrCopyEx(wKey + FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\"), "Class"), wKey + FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Network"), FCStrLen("\\") + iNetworkUuidLen), "\\");
            usKey.MaximumLength = usKey.Length = FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}")*sizeof(wchar_t);
            if (NT_SUCCESS(NtOpenKeyEx(&hKeyList, KEY_ENUMERATE_SUB_KEYS, &objAttributes, 0)))
            {
                iIndex = 0;
                wchar_t * const wIt = wKey + FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}\\");
                usKey.MaximumLength = usKey.Length = FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}\\0000")*sizeof(wchar_t);
                wchar_t wValueNetCfgInstanceId[] = L"NetCfgInstanceId";
                EX_USTRING usValueNetCfgInstanceId(wValueNetCfgInstanceId, true);
                wchar_t wValueNetworkAddress[] = L"NetworkAddress";
                EX_USTRING usValueNetworkAddress(wValueNetworkAddress, true);
                while (NtEnumerateKey(hKeyList, iIndex++, KeyBasicInformation, &keyBasicInfo, sizeof(KEY_BASIC_INFORMATION) + FCStrLen("0123")*sizeof(wchar_t), &iResultLength) == STATUS_SUCCESS)
                    if (keyBasicInfo.NameLength == FCStrLen("0123")*sizeof(wchar_t))
                    {
                        FStrCopyEx(wIt, keyBasicInfo.Name, FCStrLen("0123"));
                        HANDLE hKey;
                        if (NT_SUCCESS(NtOpenKeyEx(&hKey, KEY_READ | KEY_SET_VALUE, &objAttributes, 0)))
                        {
                            if (NtQueryValueKey(hKey, &usValueNetCfgInstanceId, KeyValuePartialInformation, &keyValuePartialInfo, sizeof(KEY_VALUE_PARTIAL_INFORMATION) + (iNetworkUuidLen + 1)*sizeof(wchar_t), &iResultLength) == STATUS_SUCCESS &&
                                    keyValuePartialInfo.Type == REG_SZ)
                            {
                                if (*wUuidWlan0 && FStrCompare(keyValuePartialInfo.Data, wUuidWlan0, iNetworkUuidLen))
                                {
                                    NtSetValueKey(hKey, &usValueNetworkAddress, 0, REG_SZ, wMacs, sizeof(wMacs)/2);
                                    *wUuidWlan0 = '\0';
                                }
                                else if (*wUuidEth0 && FStrCompare(keyValuePartialInfo.Data, wUuidEth0, iNetworkUuidLen))
                                {
                                    NtSetValueKey(hKey, &usValueNetworkAddress, 0, REG_SZ, wMacs + sizeof(wMacs)/2/sizeof(wchar_t), sizeof(wMacs)/2);
                                    *wUuidEth0 = '\0';
                                }
                            }
                            NtClose(hKey);
                            if (!*wUuidWlan0 && !*wUuidEth0)
                                break;
                        }
                    }
                NtClose(hKeyList);
            }
        }
    }
    if (bIsShutdown)
    {
        if (bRandomInit || BCryptGenRandom(nullptr, btRandom + FCStrLen("0123456789AB")*2, FCStrLen("0123456"), BCRYPT_USE_SYSTEM_PREFERRED_RNG) == STATUS_SUCCESS)
        {
            wchar_t wBuf[sizeof("Program Manager\0")];
            const BYTE *pbtSrc = btRandom + FCStrLen("0123456");
            wchar_t *pwDst = FStrCopyEx(wBuf, "DESKTOP-");
            const wchar_t * const pwEnd = pwDst + FCStrLen("0123456");
            while (pwDst < pwEnd)
                *pwDst++ = pbtVal[*pbtSrc++ % FCStrLen("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")];

            FSetSizeFromLen(&usKey, FStrCopyEx(wKey + FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\"), "ComputerName\\ComputerName") - wKey);
            HANDLE hKey;
            if (NT_SUCCESS(NtOpenKeyEx(&hKey, KEY_READ | KEY_SET_VALUE, &objAttributes, 0)))
            {
                wchar_t wValue[] = L"ComputerName";
                EX_USTRING usValue(wValue, true);
                NtSetValueKey(hKey, &usValue, 0, REG_SZ, wBuf, FCStrLen("DESKTOP-0123456")*sizeof(wchar_t));
                NtClose(hKey);
            }
            FSetSizeFromLen(&usKey, FStrCopyEx(wKey + FCStrLen("\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\"), "Services\\Tcpip\\Parameters") - wKey);
            if (NT_SUCCESS(NtOpenKeyEx(&hKey, KEY_READ | KEY_SET_VALUE, &objAttributes, 0)))
            {
                wchar_t wValue[] = L"NV Hostname";
                EX_USTRING usValue(wValue, true);
                NtSetValueKey(hKey, &usValue, 0, REG_SZ, wBuf, FCStrLen("DESKTOP-0123456")*sizeof(wchar_t));
                NtClose(hKey);
            }
        }
        EnumChildWindows(nullptr, FEnumWindowsProc, reinterpret_cast<LPARAM>(wKey));
        LARGE_INTEGER iDelayInterval;
        iDelayInterval.QuadPart = FCMsecsToNtInterval(2200);
        NtDelayExecution(FALSE, &iDelayInterval);
    }
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static bool FMake(const bool bIsShutdown)
{
    FResetInputLanguage();
    FVolumeMute();
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
        if (bOk)
        {
            FResetBrightnessPalette();
            FRandomize(bIsShutdown);
        }
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
            {
                FResetInputLanguage();
                FVolumeMute();
                LockWorkStation();
            }
            break;

        case FCStrLen("/sleep"):
            if (FStrIsStartsWith(pwArg, "/sleep") && FMake(false))
                SetSuspendState(FALSE, FALSE, TRUE);
            break;

        case FCStrLen("/reboot"):
            if (FStrIsStartsWith(pwArg, "/reboot"))
            {
                if (FMake(true))
                    ExitWindowsEx(EWX_REBOOT, g_dwReason);
            }
            else if (FStrIsStartsWith(pwArg, "/logoff"))
            {
                if (FMake(false))
                    ExitWindowsEx(EWX_LOGOFF, g_dwReason);
            }
            else if (FStrIsStartsWith(pwArg, "/hybrid"))
            {
                if (FMake(true))
                    ExitWindowsEx(EWX_SHUTDOWN | EWX_HYBRID_SHUTDOWN, g_dwReason);
            }
            break;

        case FCStrLen("/shutdown"):
            if (FStrIsStartsWith(pwArg, "/shutdown") && FMake(true))
                ExitWindowsEx(EWX_SHUTDOWN, g_dwReason);
            break;

        case FCStrLen("/hibernate"):
            if (FStrIsStartsWith(pwArg, "/hibernate") && FMake(false))
                SetSuspendState(TRUE, FALSE, TRUE);
            break;

        case FCStrLen("/reboot-force"):
            if (FStrIsStartsWith(pwArg, "/reboot-force"))
            {
                if (FMake(true))
                    ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, g_dwReason);
            }
            else if (FStrIsStartsWith(pwArg, "/logoff-force"))
            {
                if (FMake(false))
                    ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, g_dwReason);
            }
            else if (FStrIsStartsWith(pwArg, "/hybrid-force"))
            {
                if (FMake(true))
                    ExitWindowsEx(EWX_SHUTDOWN | EWX_HYBRID_SHUTDOWN | EWX_FORCEIFHUNG, g_dwReason);
            }
            break;

        case FCStrLen("/shutdown-force"):
            if (FStrIsStartsWith(pwArg, "/shutdown-force") && FMake(true))
                ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCEIFHUNG, g_dwReason);
            break;

        case FCStrLen("/sleep-allow-awake"):
            if (FStrIsStartsWith(pwArg, "/sleep-allow-awake") && FMake(false))
                SetSuspendState(FALSE, FALSE, FALSE);
            break;

        case FCStrLen("/hibernate-allow-awake"):
            if (FStrIsStartsWith(pwArg, "/hibernate-allow-awake") && FMake(false))
                SetSuspendState(TRUE, FALSE, FALSE);
            break;
        }
    RtlExitUserProcess(0);
}
