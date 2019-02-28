//QtCreatorPathFix
#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#include <windows.h>
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
static constexpr const BYTE g_iDirMaxLen = MAX_PATH - sizeof("\\qtcreator\\bin\\QtProject\\qtcreator\\toolchains.xml");
static constexpr const char cXmlBegin[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE QtCreator";
static constexpr const size_t g_szFileMaxSize = sizeof(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE QtCreatorToolChains>\n"
            "<qtcreator>\n"
            "<data>\n"
            "<variable>ToolChain.0</variable>\n"
            "<valuemap type=\"QVariantMap\">\n"
            "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">C:\\???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????\\mingw32posix_dwarf\\bin\\g++.exe</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw32posix_dwarf</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{a0518c5d-6e98-4de0-aead-a112bc1f07f7}</value>\n"
            "</valuemap>\n"
            "</data>\n"
            "<data>\n"
            "<variable>ToolChain.1</variable>\n"
            "<valuemap type=\"QVariantMap\">\n"
            "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">C:\\???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????\\mingw32win_sjlj\\bin\\g++.exe</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw32win_sjlj</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{513d00e3-d11b-40b2-95b8-a1a438c90673}</value>\n"
            "</valuemap>\n"
            "</data>\n"
            "<data>\n"
            "<variable>ToolChain.2</variable>\n"
            "<valuemap type=\"QVariantMap\">\n"
            "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">C:\\???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????\\mingw64posix_seh\\bin\\g++.exe</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw64posix_seh</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{734abf4e-7eb9-4b76-85e6-7f3a3ddae852}</value>\n"
            "</valuemap>\n"
            "</data>\n"
            "<data>\n"
            "<variable>ToolChain.3</variable>\n"
            "<valuemap type=\"QVariantMap\">\n"
            "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">C:\\???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????\\mingw64win_sjlj\\bin\\g++.exe</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw64win_sjlj</value>\n"
            "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{94418bbe-adca-4780-a2a7-f550db914cb1}</value>\n"
            "</valuemap>\n"
            "</data>\n"
            "<data>\n"
            "<variable>ToolChain.Count</variable>\n"
            "<value type=\"int\">4</value>\n"
            "</data>\n"
            "<data>\n"
            "<variable>Version</variable>\n"
            "<value type=\"int\">1</value>\n"
            "</data>\n"
            "</qtcreator>");

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static ULONG FFillToolchains(char * const pcDst, const wchar_t * const pwDirFrom, const uint iLen)
{
    return EX_ASTRING(pcDst, FCStrLen(cXmlBegin)).
            FAddStr("ToolChains>\n"
                    "<qtcreator>\n"
                    "<data>\n"
                    "<variable>ToolChain.0</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw32posix_dwarf\\bin\\g++.exe</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw32posix_dwarf</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{a0518c5d-6e98-4de0-aead-a112bc1f07f7}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>ToolChain.1</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw32win_sjlj\\bin\\g++.exe</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw32win_sjlj</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{513d00e3-d11b-40b2-95b8-a1a438c90673}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>ToolChain.2</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw64posix_seh\\bin\\g++.exe</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw64posix_seh</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{734abf4e-7eb9-4b76-85e6-7f3a3ddae852}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>ToolChain.3</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.GccToolChain.Path\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw64win_sjlj\\bin\\g++.exe</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.DisplayName\">mingw64win_sjlj</value>\n"
                    "<value type=\"QString\" key=\"ProjectExplorer.ToolChain.Id\">ProjectExplorer.ToolChain.Mingw:{94418bbe-adca-4780-a2a7-f550db914cb1}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>ToolChain.Count</variable>\n"
                    "<value type=\"int\">4</value>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>Version</variable>\n"
                    "<value type=\"int\">1</value>\n"
                    "</data>\n"
                    "</qtcreator>")->Length;
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static ULONG FFillDebuggers(char * const pcDst, const wchar_t * const pwDirFrom, const uint iLen)
{
    return EX_ASTRING(pcDst, FCStrLen(cXmlBegin)).
            FAddStr("Debuggers>\n"
                    "<qtcreator>\n"
                    "<data>\n"
                    "<variable>DebuggerItem.0</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"Binary\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw32posix_dwarf\\bin\\gdb.exe</value>\n"
                    "<value type=\"QString\" key=\"DisplayName\">gdb32posix_dwarf</value>\n"
                    "<value type=\"QString\" key=\"Id\">{f30a0c74-06f2-4880-bd84-f11c4eb1dc2d}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>DebuggerItem.1</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"Binary\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw32win_sjlj\\bin\\gdb.exe</value>\n"
                    "<value type=\"QString\" key=\"DisplayName\">gdb32win_sjlj</value>\n"
                    "<value type=\"QString\" key=\"Id\">{83ee107b-01df-4bd4-bbe8-ac5e925857ff}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>DebuggerItem.2</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"Binary\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw64posix_seh\\bin\\gdb.exe</value>\n"
                    "<value type=\"QString\" key=\"DisplayName\">gdb64posix_seh</value>\n"
                    "<value type=\"QString\" key=\"Id\">{58edefa1-728f-491d-b2b2-4d9f0e0c5d18}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>DebuggerItem.3</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"QString\" key=\"Binary\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("mingw64win_sjlj\\bin\\gdb.exe</value>\n"
                    "<value type=\"QString\" key=\"DisplayName\">gdb64win_sjlj</value>\n"
                    "<value type=\"QString\" key=\"Id\">{fecc20d4-a7da-4dc2-bb39-5a6fe47f11b7}</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>DebuggerItem.Count</variable>\n"
                    "<value type=\"int\">4</value>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>Version</variable>\n"
                    "<value type=\"int\">1</value>\n"
                    "</data>\n"
                    "</qtcreator>")->Length;
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static ULONG FFillQtVersion(char * const pcDst, const wchar_t * const pwDirFrom, const uint iLen)
{
    return EX_ASTRING(pcDst, FCStrLen(cXmlBegin)).
            FAddStr("QtVersions>\n"
                    "<qtcreator>\n"
                    "<data>\n"
                    "<variable>QtVersion.0</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"int\" key=\"Id\">8</value>\n"
                    "<value type=\"QString\" key=\"Name\">qmake32</value>\n"
                    "<value type=\"QString\" key=\"QMakePath\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("qmake32\\qmake.exe</value>\n"
                    "<value type=\"QString\" key=\"QtVersion.Type\">Qt4ProjectManager.QtVersion.Desktop</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>QtVersion.1</variable>\n"
                    "<valuemap type=\"QVariantMap\">\n"
                    "<value type=\"int\" key=\"Id\">9</value>\n"
                    "<value type=\"QString\" key=\"Name\">qmake64</value>\n"
                    "<value type=\"QString\" key=\"QMakePath\">")->
            FAddStr(pwDirFrom, iLen)->
            FAddStr("qmake64\\qmake.exe</value>\n"
                    "<value type=\"QString\" key=\"QtVersion.Type\">Qt4ProjectManager.QtVersion.Desktop</value>\n"
                    "</valuemap>\n"
                    "</data>\n"
                    "<data>\n"
                    "<variable>Version</variable>\n"
                    "<value type=\"int\">1</value>\n"
                    "</data>\n"
                    "</qtcreator>")->Length;
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static bool FWriteFile(OBJECT_ATTRIBUTES * const pobjAttributes, char * const pcFileData, const ULONG iFileSize)
{
    bool bOk = false;
    HANDLE hFile;
    IO_STATUS_BLOCK ioStatusBlock;
    if (NtCreateFile(&hFile, FILE_READ_ATTRIBUTES | GENERIC_WRITE | SYNCHRONIZE, pobjAttributes, &ioStatusBlock, nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, nullptr, 0) == STATUS_SUCCESS)
    {
        if (NtWriteFile(hFile, nullptr, nullptr, nullptr, &ioStatusBlock, pcFileData, iFileSize, nullptr, nullptr) == STATUS_SUCCESS)
            bOk = true;
        if (NtClose(hFile) != STATUS_SUCCESS)
            bOk = false;
    }
    return bOk;
}

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    bool bOk = false;
    const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
    uint iDirLen = FGetImageDirLen(pusImagePathName, g_iDirMaxLen, false);
    if (iDirLen >= FCStrLen("C:\\A"))
    {
        ++iDirLen;        //include trailing slash
        char cFileData[g_szFileMaxSize];
        FStrCopyEx(cFileData, cXmlBegin);

        wchar_t wFilePath[FCStrLen("\\??\\") + MAX_PATH - 1];
        EX_USTRING usDirPath(wFilePath);
        usDirPath.FAddStr("\\??\\")->FAddStr(pusImagePathName->Buffer, iDirLen)->FAddStr("qtcreator\\bin\\QtProject\\qtcreator\\");

        EX_USTRING usFilePathToolchains(&usDirPath);
        usFilePathToolchains.FAddStr("toolchains.xml");

        EX_OBJECT_ATTRIBUTES objAttributes(&usFilePathToolchains);
        if (FWriteFile(&objAttributes, cFileData, FFillToolchains(cFileData, pusImagePathName->Buffer, iDirLen)))
        {
            usDirPath.FAddStr("debuggers.xml");
            objAttributes.ObjectName = &usDirPath;
            if (FWriteFile(&objAttributes, cFileData, FFillDebuggers(cFileData, pusImagePathName->Buffer, iDirLen)))
            {
                static_assert(sizeof("debuggers") == sizeof("qtversion"), "");
                FStrCopyEx(usDirPath.Buffer + usDirPath.Length/sizeof(wchar_t) - FCStrLen("qtversion.xml"), "qtversion");
                if (FWriteFile(&objAttributes, cFileData, FFillQtVersion(cFileData, pusImagePathName->Buffer, iDirLen)))
                    bOk = true;
            }
        }
    }
    MessageBoxTimeoutW(nullptr, bOk ? L"Paths successfully fixed" : L"Error occured", L"QtCreatorPathFix", bOk ? MB_ICONINFORMATION : MB_ICONERROR, 0, INFINITE);
    RtlExitUserProcess(0);
}
