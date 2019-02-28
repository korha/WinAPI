//PortableExifToolRun
#define WIN32_LEAN_AND_MEAN
#ifdef WIN32_LEAN_AND_MEAN
#endif
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    EX_RTL_USER_PROCESS_PARAMETERS * const pProcParams = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters;
    const UNICODE_STRING * const pusImagePathName = &pProcParams->ImagePathName;
    if (pusImagePathName->Length <= (MAX_PATH - 1)*sizeof(wchar_t))
    {
        const wchar_t *pwIt = pusImagePathName->Buffer;
        const wchar_t * const pwEnd = pwIt + pusImagePathName->Length/sizeof(wchar_t);
        const wchar_t *pwDirEnd = nullptr;
        const wchar_t *pwDelim = nullptr;
        while (pwIt < pwEnd)
        {
            if (!(*pwIt > ' ' && *pwIt <= '~'))
            {
                pwDirEnd = nullptr;
                break;
            }
            if (*pwIt == '\\')
                pwDirEnd = pwIt;
            else if (*pwIt == '_')
                pwDelim = pwIt;
            ++pwIt;
        }
        if (pwDirEnd && pwDelim && pwDelim > pwDirEnd)
        {
            wchar_t wEnvPortableName[] = {'U','_','P','O','R','T','A','B','L','E'};
            EX_USTRING usEnvPortableName(wEnvPortableName);
            wchar_t wEnvPortableValue = '1';
            EX_USTRING usEnvPortableValue(&wEnvPortableValue, sizeof(wchar_t));
            if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnvPortableName, &usEnvPortableValue)))
            {
                wchar_t wImgFileName[FCStrLen("\\??\\") + MAX_PATH - 1];
                wImgFileName[0] = '\\';
                wImgFileName[1] = '?';
                wImgFileName[2] = '?';
                wImgFileName[3] = '\\';
                const ptrdiff_t iDistance = FStrCopyEx(wImgFileName + FCStrLen("\\??\\"), pusImagePathName->Buffer, pwDelim) - wImgFileName;

                EX_RTL_USER_PROCESS_PARAMETERS procParamsExe;
                FBytesZero(procParamsExe);
                procParamsExe.CurrentDirectory.DosPath.Buffer = wImgFileName + FCStrLen("\\??\\");
                FSetSizeFromLen(&procParamsExe.CurrentDirectory.DosPath, pwDirEnd - pusImagePathName->Buffer + 1);

                wchar_t wEnvTemp[] = {'T','E','M','P'};
                EX_USTRING usEnvTemp(wEnvTemp);
                if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnvTemp, &procParamsExe.CurrentDirectory.DosPath)))
                {
                    wchar_t wEnvTmp[] = {'T','M','P'};
                    EX_USTRING usEnvTmp(wEnvTmp);
                    if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnvTmp, &procParamsExe.CurrentDirectory.DosPath)))
                    {
                        EX_USTRING usImgFileName(wImgFileName, static_cast<uint>(iDistance));

                        procParamsExe.Flags = RTL_USER_PROCESS_PARAMETERS_NORMALIZED;

                        procParamsExe.ImagePathName.Buffer = wImgFileName + FCStrLen("\\??\\");
                        FSetSizeFromLen(&procParamsExe.ImagePathName, iDistance - static_cast<ptrdiff_t>(FCStrLen("\\??\\")));

                        FCloneUStr(&procParamsExe.CommandLine, &pProcParams->CommandLine);

                        procParamsExe.Environment = pProcParams->Environment;
                        procParamsExe.EnvironmentSize = pProcParams->EnvironmentSize;

                        RTL_USER_PROCESS_INFORMATION procInfo;
                        if (NT_SUCCESS(RtlCreateUserProcess(&usImgFileName, 0, &procParamsExe, nullptr, nullptr, nullptr, FALSE, nullptr, nullptr, &procInfo)))
                        {
                            NtResumeThread(procInfo.Thread, nullptr);
                            NtClose(procInfo.Thread);
                            NtClose(procInfo.Process);
                        }
                    }
                }
            }
        }
    }
    RtlExitUserProcess(0);
}
