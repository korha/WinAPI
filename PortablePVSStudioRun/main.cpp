//PortablePVSStudioRun
#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\data\\System.Runtime.InteropServices.RuntimeInformation.dll");

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
#ifndef _WIN64
    static_assert(false, "only 64-bit");
#endif
    const EX_RTL_USER_PROCESS_PARAMETERS * const pProcParams = FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters;
    const UNICODE_STRING * const pusImagePathName = &pProcParams->ImagePathName;
    if (pusImagePathName->Length <= (MAX_PATH - sizeof("\\data"))*sizeof(wchar_t))
    {
        const uint iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen);
        if (iDirLen >= FCStrLen("C:\\A"))
        {
            wchar_t wImgFileName[FCStrLen("\\??\\") + MAX_PATH - 1];
            wchar_t *pwImgFileName = wImgFileName;
            *pwImgFileName++ = '\\';
            *pwImgFileName++ = '?';
            *pwImgFileName++ = '?';
            *pwImgFileName++ = '\\';
            wchar_t *pwImgFileNameEnd = FStrCopyEx(pwImgFileName, pusImagePathName->Buffer, iDirLen + 1);
            *pwImgFileNameEnd++ = 'd';
            *pwImgFileNameEnd++ = 'a';
            *pwImgFileNameEnd++ = 't';
            *pwImgFileNameEnd++ = 'a';
            const ptrdiff_t iCurDirLen = pwImgFileNameEnd + 1 - pwImgFileName;        //with '\' at end

            EX_USTRING usEnvValue(pwImgFileName, static_cast<uint>(iCurDirLen - 1));        //without '\' at end
            wchar_t wEnvAllUserProfile[] = {'A','L','L','U','S','E','R','S','P','R','O','F','I','L','E'};
            wchar_t wEnvData[] = {'L','O','C','A','L','A','P','P','D','A','T','A'};
            EX_USTRING usEnv(wEnvAllUserProfile);
            if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnv, &usEnvValue)))
            {
                usEnv.Buffer = wEnvData;
                usEnv.MaximumLength = usEnv.Length = sizeof(wEnvData);
                if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnv, &usEnvValue)))
                {
                    usEnv.Buffer = wEnvData + FCStrLen("LOCAL");
                    usEnv.MaximumLength = usEnv.Length = FCStrLen("APPDATA")*sizeof(wchar_t);
                    if (NT_SUCCESS(RtlSetEnvironmentVariable(nullptr, &usEnv, &usEnvValue)))
                    {
                        pwImgFileNameEnd = FStrCopyEx(pwImgFileNameEnd, pusImagePathName->Buffer + iDirLen, pusImagePathName->Buffer + pusImagePathName->Length/sizeof(wchar_t));

                        EX_USTRING usImgFileName(wImgFileName, pwImgFileNameEnd);

                        wchar_t wCmdLine[1 + (MAX_PATH - 1) + 1];
                        wchar_t *pwCmdLineEnd = wCmdLine;
                        *pwCmdLineEnd++ = '"';
                        pwCmdLineEnd = FStrCopyEx(pwCmdLineEnd, pwImgFileName, pwImgFileNameEnd);
                        *pwCmdLineEnd++ = '"';

                        EX_RTL_USER_PROCESS_PARAMETERS procParamsExe;
                        FBytesZero(procParamsExe);
                        procParamsExe.Flags = RTL_USER_PROCESS_PARAMETERS_NORMALIZED;

                        procParamsExe.ImagePathName.Buffer = pwImgFileName;
                        FSetSizeFromLen(&procParamsExe.ImagePathName, pwImgFileNameEnd - pwImgFileName);

                        procParamsExe.CommandLine.Buffer = wCmdLine;
                        FSetSizeFromLen(&procParamsExe.CommandLine, pwCmdLineEnd - wCmdLine);

                        procParamsExe.CurrentDirectory.DosPath.Buffer = pwImgFileName;
                        FSetSizeFromLen(&procParamsExe.CurrentDirectory.DosPath, iCurDirLen);

                        procParamsExe.Environment = pProcParams->Environment;
                        procParamsExe.EnvironmentSize = pProcParams->EnvironmentSize;

                        RTL_USER_PROCESS_INFORMATION procInfo;
                        if (NT_SUCCESS(RtlCreateUserProcess(&usImgFileName, 0, &procParamsExe, nullptr, nullptr, nullptr, FALSE, nullptr, nullptr, &procInfo)))
                        {
                            bool bOk = false;
                            constexpr const char cVersionDll[] = {'v','e','r','s','i','o','n','.','d','l','l','\0'};
                            void *pBaseAddr = nullptr;
                            const SIZE_T szBuf = static_cast<size_t>(FStrCopyEx(pwImgFileName + iCurDirLen, cVersionDll, sizeof(cVersionDll)) - pwImgFileName)*sizeof(wchar_t);
                            SIZE_T szRegion = szBuf;
                            if (NT_SUCCESS(NtAllocateVirtualMemory(procInfo.Process, &pBaseAddr, 0, &szRegion, MEM_COMMIT, PAGE_READWRITE)))
                            {
                                if (NT_SUCCESS(NtWriteVirtualMemory(procInfo.Process, pBaseAddr, pwImgFileName, szBuf, nullptr)))
                                {
                                    HANDLE hThread;
                                    if (NT_SUCCESS(NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, procInfo.Process, reinterpret_cast<LPTHREAD_START_ROUTINE>(reinterpret_cast<size_t>(LoadLibraryW)), pBaseAddr, 0, 0, 0, 0, nullptr)))
                                    {
                                        if (NT_SUCCESS(NtWaitForSingleObject(hThread, FALSE, nullptr)))
                                        {
                                            THREAD_BASIC_INFORMATION threadBasicInfo;
                                            if (NT_SUCCESS(NtQueryInformationThread(hThread, ThreadBasicInformation, &threadBasicInfo, sizeof(THREAD_BASIC_INFORMATION), nullptr)) &&
                                                    threadBasicInfo.ExitStatus)        //sic! non-zero value is ok (valid HMODULE returned from LoadLibraryW)
                                                bOk = true;
                                        }
                                        if (NtClose(hThread) != STATUS_SUCCESS)
                                            bOk = false;
                                    }
                                }
                                if (!NT_SUCCESS(NtFreeVirtualMemory(procInfo.Process, &pBaseAddr, &szRegion, MEM_RELEASE)))
                                    bOk = false;
                            }
                            if (!(NtClose(procInfo.Process) == STATUS_SUCCESS && bOk && NT_SUCCESS(NtResumeThread(procInfo.Thread, nullptr))))
                                NtTerminateThread(procInfo.Thread, 0);
                            NtClose(procInfo.Thread);
                        }
                    }
                }
            }
        }
    }
    RtlExitUserProcess(0);
}
