//ComodoUpdateCheckFix
#define WIN32_LEAN_AND_MEAN
#ifdef WIN32_LEAN_AND_MEAN
#endif
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C"
{
//dbghelp.dll
PVOID WINAPI ImageDirectoryEntryToData_Stub(PVOID, BOOLEAN, USHORT, PULONG)
{
    return nullptr;
}
BOOL WINAPI MiniDumpWriteDump_Stub(HANDLE, DWORD, HANDLE, int, PVOID, PVOID, PVOID)
{
    return FALSE;
}
}

//-------------------------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
        LdrDisableThreadCalloutsForDll(hInstDll);
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        HANDLE hKey;
#ifdef _WIN64
        //wKey stored in .text section
        wchar_t wKey[] = {'\\','R','e','g','i','s','t','r','y','\\','M','a','c','h','i','n','e','\\','S','O','F','T','W','A','R','E','\\','C','O','M','O','D','O','\\','C','I','S','\\','D','a','t','a'};
        EX_USTRING usKey(true, wKey);
#else
        //wKey stored in .rdata instead of .data section
        wchar_t wKey[] = L"\\Registry\\Machine\\SOFTWARE\\COMODO\\CIS\\Data";
        EX_USTRING usKey(false, wKey);
#endif
        EX_OBJECT_ATTRIBUTES objAttributes(&usKey, true);
        if (NT_SUCCESS(NtOpenKeyEx(&hKey, KEY_SET_VALUE, &objAttributes, 0)))
        {
            wchar_t wValue[] = {'A','v','D','b','C','h','e','c','k','D','a','t','e'};
            EX_USTRING usValue(true, wValue);
            DWORD64 iValue = 0xFFFFFFFFU;        //sic!
            NtSetValueKey(hKey, &usValue, 0, REG_QWORD, &iValue, sizeof(DWORD64));
            NtClose(hKey);
        }
    }
    return TRUE;
}
