//AdobeFlashFullscreenPatcher
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const int g_iBytesTotal = 6912;
static constexpr const int g_iBytesReserve = 50;
static constexpr const int g_iFileSizeMin = 8*1024*1024;        //also divisible by allocation granularity for memory-mapped file
static constexpr const int g_iFileSizeMax = 64*1024*1024;
static constexpr const BYTE g_btPatch[] = {
    0x46,0x57,0x53,0x08,0x5E,0x00,0x00,0x00,0x78,0x00,0x05,0xDC,0x00,0x00,0x05,0xAA,
    0x00,0x00,0x14,0x01,0x00,0x44,0x11,0x00,0x00,0x00,0x00,0x43,0x02,0xFF,0xFF,0xFF,
    0x3F,0x03,0x34,0x00,0x00,0x00,0x96,0x07,0x00,0x00,0x5F,0x72,0x6F,0x6F,0x74,0x00,
    0x1C,0x96,0x24,0x00,0x00,0x41,0x6C,0x65,0x72,0x74,0x52,0x65,0x73,0x70,0x6F,0x6E,
    0x73,0x65,0x4F,0x76,0x65,0x72,0x6C,0x61,0x79,0x4D,0x65,0x73,0x73,0x61,0x67,0x65,
    0x00,0x00,0x63,0x6C,0x6F,0x73,0x65,0x00,0x4F,0x00,0x40
};

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    UNICODE_STRING * const pusCmdLine = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->CommandLine;
    const wchar_t *pwArgEnd;
    const wchar_t *pwError = L"Run with path to NPSWF*.dll in argument";
    if (wchar_t *pwArg = FGetArgFromCmdLine(pusCmdLine, &pwArgEnd))
        if (pwArg >= pusCmdLine->Buffer + FCStrLen("\\??\\"))
        {
            *--pwArg = '\\';
            *--pwArg = '?';
            *--pwArg = '?';
            *--pwArg = '\\';
            HANDLE hFile;
            EX_USTRING usFilePath(pwArg, pwArgEnd);
            EX_OBJECT_ATTRIBUTES objAttributes(&usFilePath);
            IO_STATUS_BLOCK ioStatusBlock;
            pwError = L"Opening file";
            if (NT_SUCCESS(NtOpenFile(&hFile, GENERIC_READ | GENERIC_WRITE, &objAttributes, &ioStatusBlock, 0, FILE_NON_DIRECTORY_FILE)))
            {
                FILE_STANDARD_INFORMATION fileStandardInfo;
                pwError = L"Invalid size of file";
                if (NT_SUCCESS(NtQueryInformationFile(hFile, &ioStatusBlock, &fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation)) &&
                        fileStandardInfo.EndOfFile.QuadPart >= g_iFileSizeMin && fileStandardInfo.EndOfFile.QuadPart <= g_iFileSizeMax)
                {
                    HANDLE hFileSection;
                    pwError = L"Create file mapping";
                    if (NT_SUCCESS(NtCreateSection(&hFileSection, STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_WRITE, nullptr, nullptr, PAGE_READWRITE, SEC_COMMIT, hFile)))
                    {
                        void *pBaseAddr;
                        LARGE_INTEGER iSectionOffset;
                        iSectionOffset.QuadPart = g_iFileSizeMin;
                        SIZE_T szViewSize = 0;
                        pwError = L"Map view of file";
                        if (NT_SUCCESS(NtMapViewOfSection(hFileSection, FGetCurrentProcess(), &pBaseAddr, 0, 0, &iSectionOffset, &szViewSize, ViewShare, 0, PAGE_READWRITE)))
                        {
                            BYTE *pbtIt = static_cast<BYTE*>(pBaseAddr);
                            const BYTE * const pbtEnd = pbtIt + (fileStandardInfo.EndOfFile.QuadPart - (g_iFileSizeMin + g_iBytesTotal + g_iBytesReserve));
                            constexpr const BYTE g_btSign[] = {0x46,0x57,0x53,0x08,0x07,0x1B};
                            pwError = L"Signature not found";
                            while (++pbtIt < pbtEnd)
                                if (FBytesCompare(pbtIt, g_btSign, sizeof(g_btSign)))
                                {
                                    FBytesZero(FBytesCopyEx(pbtIt, g_btPatch, sizeof(g_btPatch)), g_iBytesTotal - sizeof(g_btPatch));
                                    pwError = nullptr;
                                    break;
                                }
                            NtUnmapViewOfSection(FGetCurrentProcess(), pBaseAddr);
                        }
                        NtClose(hFileSection);
                    }
                }
                NtClose(hFile);
            }
        }
    MessageBoxTimeoutW(nullptr, pwError ? pwError : L"File successfully patched", L"AdobeFlashFullscreenPatcher", pwError ? MB_ICONERROR : MB_ICONINFORMATION, 0, INFINITE);
    RtlExitUserProcess(0);
}
