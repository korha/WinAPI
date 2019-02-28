//!@file
//!@brief This file contains helper and Windows internal functions, enums, typedefs, macroses.<br />
//!Include this file at the end of include section because it redeclare some export functions from system libraries with dllimport attribute

#ifndef HELPER_H
#define HELPER_H

#if !defined(UNICODE) || !defined(_UNICODE) || !defined(WIN32) || _WIN32_WINNT != 0xFFFF
static_assert(false, "needs to define this macroses");
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0xFFFF
#endif
#include <winternl.h>

//-------------------------------------------------------------------------------------------------
using uchar = unsigned char;
using uint = unsigned int;
using uint64 = unsigned __int64;
//!@brief Used in `ntdll!NtCreateThreadEx` function
using LPTHREAD_START_ROUTINE = DWORD (WINAPI *)(LPVOID lpThreadParameter);

//-------------------------------------------------------------------------------------------------
//!@brief Used in `ntdll!LdrGetDllHandleEx` function
#ifndef LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT
#define LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT 1
#endif

//!@brief Used in `netapi32!Netbios` function
#ifndef NRC_BADNCB
#define NRC_BADNCB 7
#endif

//!@brief Used in `shell32!SHGetKnownFolderPath` function
#ifndef REFKNOWNFOLDERID
#define REFKNOWNFOLDERID const GUID &
#endif

//!@brief Used in `EX_RTL_USER_PROCESS_PARAMETERS` structure
#ifndef RTL_MAX_DRIVE_LETTERS
#define RTL_MAX_DRIVE_LETTERS 32
#endif

//!@brief Used in `EX_RTL_USER_PROCESS_PARAMETERS` structure
#ifndef RTL_USER_PROCESS_PARAMETERS_NORMALIZED
#define RTL_USER_PROCESS_PARAMETERS_NORMALIZED 1
#endif

//!@brief Used in `ntdll!LdrFindResource_U` function
#ifndef RESOURCE_DATA_LEVEL
#define RESOURCE_DATA_LEVEL 3
#endif

//-------------------------------------------------------------------------------------------------
//!@brief Used in `ntdll!NtEnumerateKey` function
enum KEY_INFORMATION_CLASS
{
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    KeyHandleTagsInformation,
    MaxKeyInfoClass
};

//!@brief Used in `ntdll!NtQueryValueKey` function
enum KEY_VALUE_INFORMATION_CLASS
{
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass
};

//!@brief Used in `ntdll!NtQueryVirtualMemory` function
enum MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation
};

//!@brief Used in `ntdll!NtMapViewOfSection` function
enum SECTION_INHERIT
{
    ViewShare = 1,
    ViewUnmap = 2
};

//-------------------------------------------------------------------------------------------------
template <typename, typename>
struct [[nodiscard]] SIsSame final
{
    static inline constexpr bool F()
    {
        return false;
    }

    explicit SIsSame() = delete;
};

template <typename T>
struct [[nodiscard]] SIsSame<T, T> final
{
    static inline constexpr bool F()
    {
        return true;
    }

    explicit SIsSame() = delete;
};

template <typename... Ts>
struct [[nodiscard]] SIsStr final
{
public:
    static inline constexpr void F()
    {
    }

    explicit SIsStr() = delete;

private:
    template <typename T>
    struct [[nodiscard]] SIsStrHelper final
    {
        using type = T;
        static_assert(SIsSame<T, wchar_t>::F() || SIsSame<T, char>::F(), "non-string type!");

        SIsStrHelper() = delete;
        SIsStrHelper(const SIsStrHelper &) = delete;
        SIsStrHelper & operator=(const SIsStrHelper &) = delete;
    };

    using type = typename decltype((SIsStrHelper<Ts>{}, ...))::type;
};

template<typename>
struct [[nodiscard]] SIsArray final
{
    static inline constexpr bool F()
    {
        return false;
    }

    explicit SIsArray() = delete;
};

template<typename T, size_t N>
struct [[nodiscard]] SIsArray<T[N]> final
{
    static inline constexpr bool F()
    {
        return true;
    }

    explicit SIsArray() = delete;
};

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
[[nodiscard]]
static inline T1 pointer_cast(const T2 * const pSrc)
{
    return static_cast<T1>(static_cast<const void*>(pSrc));
}

template <typename T1, typename T2>
[[nodiscard]]
static inline T1 pointer_cast(T2 * const pSrc)
{
    return static_cast<T1>(static_cast<void*>(pSrc));
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FAddOffsetSelf(const T1 ** const pSrc, const T2 iOffsetInBytes)
{
    *pSrc = pointer_cast<const T1*>(pointer_cast<const BYTE*>(*pSrc) + iOffsetInBytes);
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FAddOffsetSelf(T1 ** const pSrc, const T2 iOffsetInBytes)
{
    *pSrc = pointer_cast<T1*>(pointer_cast<BYTE*>(*pSrc) + iOffsetInBytes);
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline const void * FGetOffset(const T1 * const pSrc, const T2 iOffsetInBytes)
{
    return static_cast<const void*>(pointer_cast<const BYTE*>(pSrc) + iOffsetInBytes);
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void * FGetOffset(T1 * const pSrc, const T2 iOffsetInBytes)
{
    return static_cast<void*>(pointer_cast<BYTE*>(pSrc) + iOffsetInBytes);
}

//-------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]]
static inline constexpr const T & FCMin(const T &t1, const T &t2)
{
    return t1 < t2 ? t1 : t2;
}

template <typename T>
[[nodiscard]]
static inline const T & FMin(const T &t1, const T &t2)
{
    return t1 < t2 ? t1 : t2;
}

template <typename T>
[[nodiscard]]
static inline constexpr const T & FCMax(const T &t1, const T &t2)
{
    return t1 < t2 ? t2 : t1;
}

template <typename T>
[[nodiscard]]
static inline const T & FMax(const T &t1, const T &t2)
{
    return t1 < t2 ? t2 : t1;
}

template <typename T, size_t N>
[[nodiscard]]
static inline constexpr size_t FCCountOf(const T (&)[N])
{
    return N;
}

template <typename T, size_t N>
[[nodiscard]]
static inline constexpr const T * FCGetEnd(const T (&t)[N])
{
    return t + N;
}

template <typename T>
[[nodiscard]]
static inline constexpr int FCNumToDig10(T iValue)
{
    int iCount = 0;
    while (++iCount, (iValue /= 10) > 0);
    return iCount;
}

[[nodiscard]]
static inline constexpr size_t FCUpTo(const size_t szValue, const size_t szStep)
{
    return (szValue + (szStep - 1))/szStep*szStep;
}

[[nodiscard]]
static inline size_t FUpTo(const size_t szValue, const size_t szStep)
{
    return (szValue + (szStep - 1))/szStep*szStep;
}

//-------------------------------------------------------------------------------------------------
template <size_t N>
[[nodiscard]]
static inline constexpr size_t FCStrLen(const char (&)[N])
{
    return N - 1;
}

template <typename T>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline size_t FStrLen(const T * const pSrc)
{
    SIsStr<T>::F();
    const T *pSrcIt = pSrc;
    while (*pSrcIt)
        ++pSrcIt;
    return static_cast<size_t>(pSrcIt - pSrc);
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FStrCopy(T1 *pDst, const T2 *pSrc)
{
    SIsStr<T1, T2>::F();
    while ((*pDst = static_cast<T1>(*pSrc)))
        ++pDst, ++pSrc;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FStrCopy(T1 *pDst, const T2 *pSrcFrom, const T2 * const pSrcTo)
{
    SIsStr<T1, T2>::F();
    while (pSrcFrom < pSrcTo)
        *pDst++ = static_cast<T1>(*pSrcFrom++);
    *pDst = '\0';
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FStrCopy(T1 * const pDst, const T2 *pSrc, const uint iSrcLen)
{
    SIsStr<T1, T2>::F();
    return FStrCopy(pDst, pSrc, pSrc + iSrcLen);
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline T1 * FStrCopyEx(T1 *pDst, const T2 *pSrc)
{
    SIsStr<T1, T2>::F();
    while (*pSrc)
        *pDst++ = static_cast<T1>(*pSrc++);
    return pDst;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline T1 * FStrCopyEx(T1 *pDst, const T2 *pSrcFrom, const T2 * const pSrcTo)
{
    SIsStr<T1, T2>::F();
    while (pSrcFrom < pSrcTo)
        *pDst++ = static_cast<T1>(*pSrcFrom++);
    return pDst;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline T1 * FStrCopyEx(T1 * const pDst, const T2 *pSrc, const uint iSrcLen)
{
    SIsStr<T1, T2>::F();
    return FStrCopyEx(pDst, pSrc, pSrc + iSrcLen);
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FStrCopySelf(T1 **pDst, const T2 *pSrc)
{
    SIsStr<T1, T2>::F();
    while ((**pDst = *pSrc))
        ++*pDst, ++pSrc;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FStrCopySelf(T1 **pDst, const T2 *pSrcFrom, const T2 * const pSrcTo)
{
    SIsStr<T1, T2>::F();
    while (pSrcFrom < pSrcTo)
        **pDst++ = *pSrcFrom++;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FStrCopySelf(T1 ** const pDst, const T2 *pSrc, const uint iSrcLen)
{
    SIsStr<T1, T2>::F();
    return FStrCopySelf(pDst, pSrc, pSrc + iSrcLen);
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FStrCompare(const T1 *pStr1, const T2 *pStr2)
{
    SIsStr<T1, T2>::F();
    while (*pStr1 == *pStr2 && *pStr2)
        ++pStr1, ++pStr2;
    return *pStr1 == *pStr2;
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FStrCompare(const T1 *pStr1, const T2 *pStr2, const uint iSize)
{
    SIsStr<T1, T2>::F();
    const T1 * const pStr1End = pStr1 + iSize;
    while (pStr1 < pStr1End)
        if (*pStr1++ != *pStr2++)
            return false;
    return true;
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FStrIsStartsWith(const T1 *pStrFull, const T2 *pStrStart)
{
    SIsStr<T1, T2>::F();
    while (*pStrStart)
        if (*pStrFull++ != *pStrStart++)
            return false;
    return true;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FStrSkipIfStartsWithSelf(const T1 **pStrFull, const T2 *pStrStart)
{
    SIsStr<T1, T2>::F();
    const wchar_t *pwIt = *pStrFull;
    while (*pStrStart)
        if (*pwIt++ != *pStrStart++)
            return false;
    *pStrFull = pwIt;
    return true;
}

template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FStrSkipIfStartsWithSelf(T1 **pStrFull, const T2 *pStrStart)
{
    SIsStr<T1, T2>::F();
    wchar_t *pwIt = *pStrFull;
    while (*pStrStart)
        if (*pwIt++ != *pStrStart++)
            return false;
    *pStrFull = pwIt;
    return true;
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline const wchar_t * FStrStr(const T1 *pStrFull, const T2 * const pStrSub)
{
    SIsStr<T1, T2>::F();
    while (*pStrFull)
    {
        if (FStrIsStartsWith(pStrFull, pStrSub))
            return pStrFull;
        ++pStrFull;
    }
    return nullptr;
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline wchar_t * FStrStr(T1 *pStrFull, const T2 * const pStrSub)
{
    SIsStr<T1, T2>::F();
    while (*pStrFull)
    {
        if (FStrIsStartsWith(pStrFull, pStrSub))
            return pStrFull;
        ++pStrFull;
    }
    return nullptr;
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline const wchar_t * FStrChr(const T1 *pSrc, const T2 tCh)
{
    SIsStr<T1, T2>::F();
    do
    {
        if (*pSrc == tCh)
            return pSrc;
    } while (*pSrc++);
    return nullptr;
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static void FNumToStr(T1 *pBuf, T2 iValue)
{
    SIsStr<T1>::F();
    static_assert(SIsSame<T2, uint>::F() || SIsSame<T2, uint64>::F(), "");

    T1 *pIt = pBuf;
    do
    {
        *pIt++ = static_cast<T1>(iValue%10 + '0');
        iValue /= 10;
    } while (iValue > 0);
    *pIt-- = '\0';
    do
    {
        const T1 tTemp = *pIt;
        *pIt = *pBuf;
        *pBuf = tTemp;
    } while (++pBuf < --pIt);
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static T1 * FNumToStrEx(T1 *pBuf, T2 iValue)
{
    SIsStr<T1>::F();
    static_assert(SIsSame<T2, uint>::F() || SIsSame<T2, uint64>::F(), "");

    T1 *pIt = pBuf;
    do
    {
        *pIt++ = static_cast<T1>(iValue%10 + '0');
        iValue /= 10;
    } while (iValue > 0);
    T1 * const pEnd = pIt--;
    do
    {
        const T1 tTemp = *pIt;
        *pIt = *pBuf;
        *pBuf = tTemp;
    } while (++pBuf < --pIt);
    return pEnd;
}

//-------------------------------------------------------------------------------------------------
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline BYTE * FBytesCopyEx(void * const pDst, const void * const pSrcFrom, const void * const pSrcTo)
{
    BYTE *pbtDst = static_cast<BYTE*>(pDst);
    const BYTE *pbtSrcFrom = static_cast<const BYTE*>(pSrcFrom);
    const BYTE * const pbtSrcTo = static_cast<const BYTE*>(pSrcTo);
    while (pbtSrcFrom < pbtSrcTo)
        *pbtDst++ = *pbtSrcFrom++;
    return pbtDst;
}

#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline BYTE * FBytesCopyEx(void * const pDst, const void * const pSrc, const uint iSize)
{
    return FBytesCopyEx(pDst, pSrc, static_cast<const BYTE*>(pSrc) + iSize);
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FBytesCompare(const void * const p1, const void * const p2, const uint iSize)
{
    const BYTE *pbt1 = static_cast<const BYTE*>(p1);
    const BYTE *pbt2 = static_cast<const BYTE*>(p2);
    const BYTE * const pbtEnd = pbt1 + iSize;
    while (pbt1 < pbtEnd)
        if (*pbt1++ != *pbt2++)
            return false;
    return true;
}

#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FBytesZero(void * const pDstFrom, const void * const pDstTo)
{
    BYTE *pbtDst = static_cast<BYTE*>(pDstFrom);
    const BYTE * const pbtDstTo = static_cast<const BYTE*>(pDstTo);
    while (pbtDst < pbtDstTo)
        *pbtDst++ = '\0';
}

#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FBytesZero(void * const pDst, const uint iSize)
{
    FBytesZero(pDst, static_cast<BYTE*>(pDst) + iSize);
}

template <typename T>
static inline constexpr void FBytesZero(T &tBuf)
{
    if constexpr (SIsArray<T>::F())
            FBytesZero(tBuf, sizeof(T));
    else
    FBytesZero(&tBuf, sizeof(T));
}

//-------------------------------------------------------------------------------------------------
struct [[nodiscard]] KEY_BASIC_INFORMATION final
{
    LARGE_INTEGER LastWriteTime;
    ULONG TitleIndex;
    ULONG NameLength;
    //WCHAR Name[...];

    explicit KEY_BASIC_INFORMATION() = delete;
};

template <size_t N>
struct [[nodiscard]] EX_KEY_BASIC_INFORMATION final
{
    LARGE_INTEGER LastWriteTime;
    ULONG TitleIndex;
    ULONG NameLength;
    WCHAR Name[FCUpTo(N, sizeof(LARGE_INTEGER)/sizeof(WCHAR))];

    explicit EX_KEY_BASIC_INFORMATION() = default;
    EX_KEY_BASIC_INFORMATION(const EX_KEY_BASIC_INFORMATION &) = delete;
    EX_KEY_BASIC_INFORMATION & operator=(const EX_KEY_BASIC_INFORMATION &) = delete;
};

struct [[nodiscard]] KEY_VALUE_PARTIAL_INFORMATION final
{
    ULONG TitleIndex;
    ULONG Type;
    ULONG DataLength;
    //WCHAR Data[...];

    explicit KEY_VALUE_PARTIAL_INFORMATION() = delete;
};

template <size_t N>
struct [[nodiscard]] EX_KEY_VALUE_PARTIAL_INFORMATION final
{
    ULONG TitleIndex;
    ULONG Type;
    ULONG DataLength;
    WCHAR Data[FCUpTo(N, sizeof(ULONG)/sizeof(WCHAR))];

    explicit EX_KEY_VALUE_PARTIAL_INFORMATION() = default;
    EX_KEY_VALUE_PARTIAL_INFORMATION(const EX_KEY_VALUE_PARTIAL_INFORMATION &) = delete;
    EX_KEY_VALUE_PARTIAL_INFORMATION & operator=(const EX_KEY_VALUE_PARTIAL_INFORMATION &) = delete;
};

struct [[nodiscard]] THREAD_BASIC_INFORMATION final
{
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    ULONG_PTR AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;

    explicit THREAD_BASIC_INFORMATION() = default;
    THREAD_BASIC_INFORMATION(const THREAD_BASIC_INFORMATION &) = delete;
    THREAD_BASIC_INFORMATION & operator=(const THREAD_BASIC_INFORMATION &) = delete;
};

struct [[nodiscard]] TIME_FIELDS final
{
    USHORT Year;
    USHORT Month;
    USHORT Day;
    USHORT Hour;
    USHORT Minute;
    USHORT Second;
    USHORT Milliseconds;
    USHORT Weekday;

    explicit TIME_FIELDS() = delete;
};

struct [[nodiscard]] LDR_RESOURCE_INFO final
{
    ULONG_PTR Type;
    ULONG_PTR Name;
    ULONG_PTR Language;

    explicit LDR_RESOURCE_INFO() = delete;
};

struct [[nodiscard]] RTL_USER_PROCESS_INFORMATION final
{
    ULONG Length;
    HANDLE Process;
    HANDLE Thread;
    CLIENT_ID ClientId;
    struct [[nodiscard]] SECTION_IMAGE_INFORMATION final
    {
        PVOID TransferAddress;
        ULONG ZeroBits;
        SIZE_T MaximumStackSize;
        SIZE_T CommittedStackSize;
        ULONG SubSystemType;
        union
        {
            struct
            {
                USHORT SubSystemMinorVersion;
                USHORT SubSystemMajorVersion;
            } u_SubSystemVersion;
            ULONG SubSystemVersion;
        };
        union
        {
            struct
            {
                USHORT MajorOperatingSystemVersion;
                USHORT MinorOperatingSystemVersion;
            } u_OperatingSystemVersion;
            ULONG OperatingSystemVersion;
        };
        USHORT ImageCharacteristics;
        USHORT DllCharacteristics;
        USHORT Machine;
        BOOLEAN ImageContainsCode;
        union
        {
            UCHAR ImageFlags;
            struct
            {
                UCHAR ComPlusNativeReady : 1;
                UCHAR ComPlusILOnly : 1;
                UCHAR ImageDynamicallyRelocated : 1;
                UCHAR ImageMappedFlat : 1;
                UCHAR BaseBelow4gb : 1;
                UCHAR ComPlusPrefer32bit : 1;
                UCHAR Reserved : 2;
            } u_ImageFlags;
        };
        ULONG LoaderFlags;
        ULONG ImageFileSize;
        ULONG CheckSum;

        explicit SECTION_IMAGE_INFORMATION() = default;
        SECTION_IMAGE_INFORMATION(const SECTION_IMAGE_INFORMATION &) = delete;
        SECTION_IMAGE_INFORMATION & operator=(const SECTION_IMAGE_INFORMATION &) = delete;
    } ImageInformation;

    explicit RTL_USER_PROCESS_INFORMATION() = default;
    RTL_USER_PROCESS_INFORMATION(const RTL_USER_PROCESS_INFORMATION &) = delete;
    RTL_USER_PROCESS_INFORMATION & operator=(const RTL_USER_PROCESS_INFORMATION &) = delete;
};

struct [[nodiscard]] EX_RTL_USER_PROCESS_PARAMETERS final
{
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    HANDLE ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;
    struct [[nodiscard]] CURDIR final
    {
        UNICODE_STRING DosPath;
        HANDLE Handle;

        explicit CURDIR() = default;
        CURDIR(const CURDIR &) = delete;
        CURDIR & operator=(const CURDIR &) = delete;
    } CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    LPWSTR Environment;
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    struct [[nodiscard]] RTL_DRIVE_LETTER_CURDIR final
    {
        USHORT Flags;
        USHORT Length;
        ULONG TimeStamp;
        UNICODE_STRING DosPath;

        explicit RTL_DRIVE_LETTER_CURDIR() = default;
        RTL_DRIVE_LETTER_CURDIR(const RTL_DRIVE_LETTER_CURDIR &) = delete;
        RTL_DRIVE_LETTER_CURDIR & operator=(const RTL_DRIVE_LETTER_CURDIR &) = delete;
    } CurrentDirectories[RTL_MAX_DRIVE_LETTERS];
    ULONG_PTR EnvironmentSize;
    ULONG_PTR EnvironmentVersion;
    PVOID PackageDependencyData;
    ULONG ProcessGroupId;
    ULONG LoaderThreads;

    explicit EX_RTL_USER_PROCESS_PARAMETERS() = default;
    EX_RTL_USER_PROCESS_PARAMETERS(const EX_RTL_USER_PROCESS_PARAMETERS &) = delete;
    EX_RTL_USER_PROCESS_PARAMETERS & operator=(const EX_RTL_USER_PROCESS_PARAMETERS &) = delete;
};

struct [[nodiscard]] EX_TEB final
{
    NT_TIB NtTib;
    PVOID EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    struct [[nodiscard]] EX_PEB final
    {
        BOOLEAN InheritedAddressSpace;
        BOOLEAN ReadImageFileExecOptions;
        BOOLEAN BeingDebugged;
        union
        {
            BOOLEAN BitField;
            struct
            {
                BOOLEAN ImageUsesLargePages : 1;
                BOOLEAN IsProtectedProcess : 1;
                BOOLEAN IsImageDynamicallyRelocated : 1;
                BOOLEAN SkipPatchingUser32Forwarders : 1;
                BOOLEAN IsPackagedProcess : 1;
                BOOLEAN IsAppContainer : 1;
                BOOLEAN IsProtectedProcessLight : 1;
                BOOLEAN IsLongPathAwareProcess : 1;
            } u_BitField;
        };
        HANDLE Mutant;
        HINSTANCE ImageBaseAddress;
        struct [[nodiscard]] EX_PEB_LDR_DATA final
        {
            struct [[nodiscard]] EX_LDR_DATA_TABLE_ENTRY final
            {
                LIST_ENTRY InLoadOrderLinks;
                LIST_ENTRY InMemoryOrderLinks;
                LIST_ENTRY InInitializationOrderLinks;
                PVOID ModuleBaseAddress;
                PVOID EntryPoint;
                ULONG SizeOfImage;
                UNICODE_STRING FullDllName;
                //...
                explicit EX_LDR_DATA_TABLE_ENTRY() = delete;
            };
            ULONG Length;
            BOOLEAN Initialized;
            HANDLE SsHandle;
            LIST_ENTRY InLoadOrderModuleList;
            LIST_ENTRY InMemoryOrderModuleList;
            LIST_ENTRY InInitializationOrderModuleList;
            PVOID EntryInProgress;
            BOOLEAN ShutdownInProgress;
            HANDLE ShutdownThreadId;

            explicit EX_PEB_LDR_DATA() = delete;
        } *Ldr;
        EX_RTL_USER_PROCESS_PARAMETERS *ProcessParameters;
        PVOID SubSystemData;
        PVOID ProcessHeap;
        PRTL_CRITICAL_SECTION FastPebLock;
        PVOID AtlThunkSListPtr;
        PVOID IFEOKey;
        union
        {
            ULONG CrossProcessFlags;
            struct
            {
                ULONG ProcessInJob : 1;
                ULONG ProcessInitializing : 1;
                ULONG ProcessUsingVEH : 1;
                ULONG ProcessUsingVCH : 1;
                ULONG ProcessUsingFTH : 1;
                ULONG ProcessPreviouslyThrottled : 1;
                ULONG ProcessCurrentlyThrottled : 1;
                ULONG ReservedBits0 : 25;
            } u_CrossProcessFlags;
        };
#ifdef _WIN64
        ULONG iPad;
#endif
        union
        {
            PVOID KernelCallbackTable;
            PVOID UserSharedInfoPtr;
        };
        ULONG SystemReserved[1];
        ULONG AtlThunkSListPtr32;
        struct [[nodiscard]] API_SET_NAMESPACE final
        {
            ULONG Version;
            ULONG Size;
            ULONG Flags;
            ULONG Count;
            ULONG EntryOffset;
            ULONG HashOffset;
            ULONG HashFactor;

            explicit API_SET_NAMESPACE() = delete;
        } *ApiSetMap;
        ULONG TlsExpansionCounter;
        PVOID TlsBitmap;
        ULONG TlsBitmapBits[2];
        PVOID ReadOnlySharedMemoryBase;
        PVOID SharedData;
        PVOID *ReadOnlyStaticServerData;
        PVOID AnsiCodePageData;
        PVOID OemCodePageData;
        PVOID UnicodeCaseTableData;
        ULONG NumberOfProcessors;
        ULONG NtGlobalFlag;
        //...
        explicit EX_PEB() = delete;
    } *ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    //...
    explicit EX_TEB() = delete;
};

//-------------------------------------------------------------------------------------------------
struct [[nodiscard]] EX_USTRING final : public UNICODE_STRING
{
    template <size_t N>
    explicit inline constexpr EX_USTRING(const bool bFullArray, wchar_t (&pwBuf)[N]) :
        UNICODE_STRING{static_cast<USHORT>((bFullArray ? N : (N - 1))*sizeof(wchar_t)), static_cast<USHORT>((bFullArray ? N : (N - 1))*sizeof(wchar_t)), pwBuf}
    {
    }

#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    explicit inline constexpr EX_USTRING(wchar_t * const pwBuf, const uint iLen = 0) :
        UNICODE_STRING{static_cast<USHORT>(iLen*sizeof(wchar_t)), static_cast<USHORT>(iLen*sizeof(wchar_t)), pwBuf}
    {
    }

#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    explicit inline EX_USTRING(wchar_t * const pwBufFrom, const wchar_t * const pwBufTo) :
        UNICODE_STRING{static_cast<USHORT>(static_cast<size_t>(pwBufTo - pwBufFrom)*sizeof(wchar_t)), static_cast<USHORT>(static_cast<size_t>(pwBufTo - pwBufFrom)*sizeof(wchar_t)), pwBufFrom}
    {
    }

#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    explicit inline EX_USTRING(const UNICODE_STRING * const pusSrc) :
        UNICODE_STRING{pusSrc->Length, pusSrc->Length, pusSrc->Buffer}
    {
    }

    EX_USTRING(const EX_USTRING &) = delete;
    EX_USTRING & operator=(const EX_USTRING &) = delete;

    template <typename T>
#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    inline EX_USTRING * FAddStr(const T *pSrc)
    {
        SIsStr<T>::F();
        wchar_t *pwIt = Buffer + Length/sizeof(wchar_t);
        while (*pSrc)
            *pwIt++ = static_cast<wchar_t>(*pSrc++);
        MaximumLength = Length = static_cast<USHORT>(static_cast<size_t>(pwIt - Buffer)*sizeof(wchar_t));
        return this;
    }

    template <typename T>
#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    inline EX_USTRING * FAddStr(const T *pSrcFrom, const T * const pSrcTo)
    {
        SIsStr<T>::F();
        wchar_t *pwIt = Buffer + Length/sizeof(wchar_t);
        while (pSrcFrom < pSrcTo)
            *pwIt++ = static_cast<wchar_t>(*pSrcFrom++);
        MaximumLength = Length = static_cast<USHORT>(static_cast<size_t>(pwIt - Buffer)*sizeof(wchar_t));
        return this;
    }

    template <typename T>
#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    inline EX_USTRING * FAddStr(const T * const pSrc, const uint iSrcLen)
    {
        SIsStr<T>::F();
        return FAddStr(pSrc, pSrc + iSrcLen);
    }

    inline EX_USTRING * FAddChar(const wchar_t wCh)
    {
        Buffer[Length/sizeof(wchar_t)] = wCh;
        MaximumLength = Length = static_cast<USHORT>(Length + sizeof(wchar_t));
        return this;
    }

    inline EX_USTRING * FAddNull()
    {
        return FAddChar('\0');
    }

    template <typename T>
    inline EX_USTRING * FAddNum(T iValue)
    {
        static_assert(SIsSame<T, uint>::F() || SIsSame<T, uint64>::F(), "");

        wchar_t *pwBuf = Buffer + Length/sizeof(wchar_t);
        wchar_t *pwIt = pwBuf;
        do
        {
            *pwIt++ = static_cast<wchar_t>(iValue%10 + '0');
            iValue /= 10;
        } while (iValue > 0);
        MaximumLength = Length = static_cast<USHORT>(static_cast<size_t>(pwIt - Buffer)*sizeof(wchar_t));
        --pwIt;
        do
        {
            const wchar_t wTemp = *pwIt;
            *pwIt = *pwBuf;
            *pwBuf = wTemp;
        } while (++pwBuf < --pwIt);
        return this;
    }

    [[nodiscard]]
    inline uint FGetLen() const
    {
        return static_cast<uint>(Length/sizeof(wchar_t));
    }
};

struct [[nodiscard]] EX_ASTRING final : public ANSI_STRING
{
    template <size_t N>
    explicit inline constexpr EX_ASTRING(const bool bFullArray, char (&pcBuf)[N]) :
        ANSI_STRING{static_cast<USHORT>((bFullArray ? N : (N - 1))), static_cast<USHORT>((bFullArray ? N : (N - 1))), pcBuf}
    {
    }

#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    explicit inline constexpr EX_ASTRING(char * const pcBuf, const uint iLen = 0) :
        ANSI_STRING{static_cast<USHORT>(iLen), static_cast<USHORT>(iLen), pcBuf}
    {
    }

#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    explicit inline EX_ASTRING(char * const pcBufFrom, const char * const pcBufTo) :
        ANSI_STRING{static_cast<USHORT>(pcBufTo - pcBufFrom), static_cast<USHORT>(pcBufTo - pcBufFrom), pcBufFrom}
    {
    }

#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    explicit inline EX_ASTRING(const ANSI_STRING * const pasSrc) :
        ANSI_STRING{pasSrc->Length, pasSrc->Length, pasSrc->Buffer}
    {
    }

    EX_ASTRING(const EX_ASTRING &) = delete;
    EX_ASTRING & operator=(const EX_ASTRING &) = delete;

    template <typename T>
#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    inline EX_ASTRING * FAddStr(const T *pSrc)
    {
        SIsStr<T>::F();
        char *pcIt = Buffer + Length;
        while (*pSrc)
            *pcIt++ = static_cast<char>(*pSrc++);
        MaximumLength = Length = static_cast<USHORT>(pcIt - Buffer);
        return this;
    }

    template <typename T>
#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    inline EX_ASTRING * FAddStr(const T *pSrcFrom, const T * const pSrcTo)
    {
        SIsStr<T>::F();
        char *pcIt = Buffer + Length;
        while (pSrcFrom < pSrcTo)
            *pcIt++ = static_cast<char>(*pSrcFrom++);
        MaximumLength = Length = static_cast<USHORT>(pcIt - Buffer);
        return this;
    }

    template <typename T>
#ifdef __MINGW32__
    __attribute__((nonnull))
#endif
    inline EX_ASTRING * FAddStr(const T * const pSrc, const uint iSrcLen)
    {
        SIsStr<T>::F();
        return FAddStr(pSrc, pSrc + iSrcLen);
    }

    inline EX_ASTRING * FAddChar(const char cCh)
    {
        Buffer[Length] = cCh;
        ++Length;
        MaximumLength = Length;
        return this;
    }

    inline EX_ASTRING * FAddNull()
    {
        return FAddChar('\0');
    }

    template <typename T>
    inline EX_ASTRING * FAddNum(T iValue)
    {
        static_assert(SIsSame<T, uint>::F() || SIsSame<T, uint64>::F(), "");

        char *pcBuf = Buffer + Length;
        char *pcIt = pcBuf;
        do
        {
            *pcIt++ = static_cast<char>(iValue%10 + '0');
            iValue /= 10;
        } while (iValue > 0);
        MaximumLength = Length = static_cast<USHORT>(pcIt - Buffer);
        --pcIt;
        do
        {
            const char cTemp = *pcIt;
            *pcIt = *pcBuf;
            *pcBuf = cTemp;
        } while (++pcBuf < --pcIt);
        return this;
    }

    [[nodiscard]]
    inline uint FGetLen() const
    {
        return Length;
    }
};

struct [[nodiscard]] EX_OBJECT_ATTRIBUTES final : public OBJECT_ATTRIBUTES
{
    explicit inline EX_OBJECT_ATTRIBUTES(UNICODE_STRING * const pusObjName = nullptr, const bool bCaseInsensetive = false)
    {
        Length = sizeof(OBJECT_ATTRIBUTES);
        RootDirectory = nullptr;
        ObjectName = pusObjName;
        Attributes = bCaseInsensetive ? OBJ_CASE_INSENSITIVE : 0;
        SecurityDescriptor = nullptr;
        SecurityQualityOfService = nullptr;
    }

    EX_OBJECT_ATTRIBUTES(const EX_OBJECT_ATTRIBUTES &) = delete;
    EX_OBJECT_ATTRIBUTES & operator=(const EX_OBJECT_ATTRIBUTES &) = delete;
};

//-------------------------------------------------------------------------------------------------
extern "C"
{
#ifdef __llvm__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdll-attribute-on-redeclaration"
#elif defined(__MINGW32__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

//kernel32.dll
__declspec(dllimport) DWORD WINAPI K32GetModuleFileNameExW(HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
__declspec(dllimport) DWORD WINAPI K32GetProcessImageFileNameW(HANDLE hProcess, LPWSTR lpImageFileName, DWORD nSize);
#if defined(_PSAPI_H_) || defined(PSAPI_VERSION)
__declspec(dllimport) WINBOOL WINAPI K32GetProcessMemoryInfo(HANDLE Process, PROCESS_MEMORY_COUNTERS *ppsmemCounters, DWORD cb);
#endif
//ntdll.dll
__declspec(dllimport) NTSTATUS NTAPI LdrAccessResource(PVOID BaseAddress, PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry, PVOID *Resource, PULONG Size);
__declspec(dllimport) NTSTATUS NTAPI LdrDisableThreadCalloutsForDll(PVOID BaseAddress);
__declspec(dllimport) NTSTATUS NTAPI LdrFindResource_U(PVOID BaseAddress, LDR_RESOURCE_INFO *ResourceInfo, ULONG Level, PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry);
__declspec(dllimport) NTSTATUS NTAPI LdrGetDllHandleEx(ULONG Flags, PWSTR DllPath, PULONG DllCharacteristics, PUNICODE_STRING DllName, PVOID *DllHandle);
__declspec(dllimport) NTSTATUS NTAPI LdrGetProcedureAddressEx(PVOID DllHandle, PANSI_STRING ProcedureName, ULONG ProcedureNumber, PVOID *ProcedureAddress, ULONG Flags);        //superseed with LdrGetProcedureAddressForCaller
__declspec(dllimport) NTSTATUS NTAPI LdrLoadDll(PWSTR DllPath, PULONG LoadFlags, PUNICODE_STRING Name, PVOID *BaseAddress);
__declspec(dllimport) NTSTATUS NTAPI LdrUnloadDll(PVOID BaseAddress);
__declspec(dllimport) NTSTATUS NTAPI NtAdjustPrivilegesToken(HANDLE TokenHandle, BOOLEAN DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
__declspec(dllimport) NTSTATUS NTAPI NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID *UBaseAddress, ULONG_PTR ZeroBits, PSIZE_T URegionSize, ULONG AllocationType, ULONG Protect);
__declspec(dllimport) NTSTATUS NTAPI NtClose(HANDLE Handle);
__declspec(dllimport) NTSTATUS NTAPI NtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);
__declspec(dllimport) NTSTATUS NTAPI NtCreateKey(PHANDLE KeyHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG TitleIndex, PUNICODE_STRING Class, ULONG CreateOptions, PULONG Disposition);
__declspec(dllimport) NTSTATUS NTAPI NtCreateSection(PHANDLE SectionHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize, ULONG SectionPageProtection, ULONG AllocationAttributes, HANDLE FileHandle);
__declspec(dllimport) NTSTATUS NTAPI NtCreateThreadEx(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, LPTHREAD_START_ROUTINE StartAddress, PVOID Parameter, ULONG CreateFlags, SIZE_T StackZeroBits, SIZE_T StackSize, SIZE_T MaximumStackSize, PVOID AttributeList);
__declspec(dllimport) NTSTATUS NTAPI NtCreateUserProcess(PHANDLE ProcessHandle, PHANDLE ThreadHandle, ACCESS_MASK ProcessDesiredAccess, ACCESS_MASK ThreadDesiredAccess, POBJECT_ATTRIBUTES ProcessObjectAttributes, POBJECT_ATTRIBUTES ThreadObjectAttributes, ULONG ProcessFlags, ULONG ThreadFlags, EX_RTL_USER_PROCESS_PARAMETERS *ProcessParameters, PVOID CreateInfo, PVOID AttributeList);
__declspec(dllimport) NTSTATUS NTAPI NtDelayExecution(BOOLEAN Alertable, PLARGE_INTEGER DelayInterval);
__declspec(dllimport) LRESULT NTAPI NtdllDefWindowProc_W(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
__declspec(dllimport) NTSTATUS NTAPI NtEnumerateKey(HANDLE KeyHandle, ULONG Index, KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength);
__declspec(dllimport) NTSTATUS NTAPI NtFlushInstructionCache(HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T FlushSize);
__declspec(dllimport) NTSTATUS NTAPI NtFreeVirtualMemory(HANDLE ProcessHandle, PVOID *UBaseAddress, PSIZE_T URegionSize, ULONG FreeType);
__declspec(dllimport) NTSTATUS NTAPI NtMapViewOfSection(HANDLE SectionHandle, HANDLE ProcessHandle, PVOID *BaseAddress, ULONG_PTR ZeroBits, SIZE_T CommitSize, PLARGE_INTEGER SectionOffset, PSIZE_T ViewSize, SECTION_INHERIT InheritDisposition, ULONG AllocationType, ULONG Protect);
__declspec(dllimport) NTSTATUS NTAPI NtOpenFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, ULONG ShareAccess, ULONG OpenOptions);
__declspec(dllimport) NTSTATUS NTAPI NtOpenKeyEx(PHANDLE KeyHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG OpenOptions);
__declspec(dllimport) NTSTATUS NTAPI NtOpenProcess(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);
__declspec(dllimport) NTSTATUS NTAPI NtOpenProcessToken(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, PHANDLE TokenHandle);
__declspec(dllimport) NTSTATUS NTAPI NtProtectVirtualMemory(HANDLE ProcessHandle, PVOID *BaseAddress, SIZE_T *NumberOfBytesToProtect, ULONG NewAccessProtection, PULONG OldAccessProtection);
__declspec(dllimport) NTSTATUS NTAPI NtQueryDirectoryFile(HANDLE FileHandle, HANDLE EventHandle, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan);
__declspec(dllimport) NTSTATUS NTAPI NtQueryInformationFile(HANDLE hFile, PIO_STATUS_BLOCK io, PVOID ptr, ULONG len, FILE_INFORMATION_CLASS FileInformationClass);
__declspec(dllimport) NTSTATUS NTAPI NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
__declspec(dllimport) NTSTATUS NTAPI NtQueryInformationThreads(HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);
__declspec(dllimport) NTSTATUS NTAPI NtQueryPerformanceCounter(PLARGE_INTEGER PerformanceCounter, PLARGE_INTEGER PerformanceFrequency);
__declspec(dllimport) NTSTATUS NTAPI NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
__declspec(dllimport) NTSTATUS NTAPI NtQueryValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID KeyValueInformation, ULONG Length, PULONG ResultLength);
__declspec(dllimport) NTSTATUS NTAPI NtQueryVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength);
__declspec(dllimport) NTSTATUS NTAPI NtResumeProcess(HANDLE ProcessHandle);
__declspec(dllimport) NTSTATUS NTAPI NtResumeThread(HANDLE ThreadHandle, PULONG SuspendCount);
__declspec(dllimport) NTSTATUS NTAPI NtSetInformationFile(HANDLE hFile, PIO_STATUS_BLOCK io, PVOID ptr, ULONG len, FILE_INFORMATION_CLASS FileInformationClass);
__declspec(dllimport) NTSTATUS NTAPI NtSetValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName, ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize);
__declspec(dllimport) NTSTATUS NTAPI NtSuspendProcess(HANDLE ProcessHandle);
__declspec(dllimport) NTSTATUS NTAPI NtTerminateProcess(HANDLE ProcessHandle, NTSTATUS ExitStatus);
__declspec(dllimport) NTSTATUS NTAPI NtTerminateThread(HANDLE ThreadHandle, NTSTATUS ExitStatus);
__declspec(dllimport) NTSTATUS NTAPI NtUnmapViewOfSection(HANDLE ProcessHandle, PVOID BaseAddress);
__declspec(dllimport) NTSTATUS NTAPI NtWaitForSingleObject(HANDLE Handle,BOOLEAN Alertable, PLARGE_INTEGER Timeout);
__declspec(dllimport) NTSTATUS NTAPI NtWriteFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
__declspec(dllimport) NTSTATUS NTAPI NtWriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToWrite, PSIZE_T NumberOfBytesWritten);
__declspec(dllimport) PVOID NTAPI RtlAllocateHeap(PVOID HeapHandle, ULONG Flags, SIZE_T Size);
__declspec(dllimport) NTSTATUS NTAPI RtlCreateUserProcess(PUNICODE_STRING ImageFileName, ULONG Attributes, EX_RTL_USER_PROCESS_PARAMETERS *ProcessParameters, PSECURITY_DESCRIPTOR ProcessSecurityDescriptor, PSECURITY_DESCRIPTOR ThreadSecurityDescriptor, HANDLE ParentProcess, BOOLEAN InheritHandles, HANDLE DebugPort, HANDLE ExceptionPort, RTL_USER_PROCESS_INFORMATION *ProcessInfo);        //superseed with NtCreateUserProcess
__declspec(dllimport) __declspec(noreturn) VOID NTAPI RtlExitUserProcess(LONG ExitStatus);
__declspec(dllimport) NTSTATUS NTAPI RtlFindMessage(PVOID BaseAddress, ULONG Type, ULONG Language, ULONG MessageId, PMESSAGE_RESOURCE_ENTRY *MessageResourceEntry);        //superseeded with manual function
__declspec(dllimport) BOOLEAN NTAPI RtlFreeHeap(PVOID HeapHandle, ULONG Flags, PVOID HeapBase);        //since Windows 8 return LOGICAL
__declspec(dllimport) NTSTATUS NTAPI RtlQueryEnvironmentVariable_U(PWSTR Environment, PCUNICODE_STRING Name, PUNICODE_STRING Value);
__declspec(dllimport) NTSTATUS NTAPI RtlSetCurrentDirectory_U(PUNICODE_STRING Path);
__declspec(dllimport) NTSTATUS NTAPI RtlSetEnvironmentVariable(PWSTR *Environment, PUNICODE_STRING Name, PUNICODE_STRING Value);
__declspec(dllimport) VOID NTAPI RtlTimeToTimeFields(const LARGE_INTEGER *Time, TIME_FIELDS *TimeFields);
//powrprof.dll
__declspec(dllimport) BOOLEAN WINAPI SetSuspendState(BOOLEAN bHibernate, BOOLEAN bForce, BOOLEAN bWakeupEventsDisabled);
//shell32.dll
__declspec(dllimport) VOID WINAPI Control_RunDLLW(HWND hWnd, HINSTANCE hInst, LPCWSTR cmd, DWORD nCmdShow);
__declspec(dllimport) HRESULT WINAPI SHGetKnownFolderPath(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
//user32.dll
__declspec(dllimport) int WINAPI MessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds);

#if defined(__llvm__) || defined(__MINGW32__)
#pragma GCC diagnostic pop
#endif
}

//-------------------------------------------------------------------------------------------------
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FCloneUStr(UNICODE_STRING * const pusDst, const UNICODE_STRING * const pusSrc)
{
    pusDst->Length = pusSrc->Length;
    pusDst->MaximumLength = pusSrc->MaximumLength;
    pusDst->Buffer = pusSrc->Buffer;
}

[[nodiscard]]
static inline constexpr LONGLONG FCMsecsToNtInterval(const DWORD dwMsecs)
{
    return dwMsecs*-10000LL;
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline wchar_t * FGetArgFromCmdLine(const UNICODE_STRING * const pusCmdLine, const wchar_t ** const ppwArgEnd)
{
    bool bOk = false;
    wchar_t *pwArg = nullptr;
    wchar_t *pwCmdLine = pusCmdLine->Buffer;
    const size_t szLen = pusCmdLine->Length/sizeof(wchar_t);
    if (szLen >= FCStrLen("a b"))
    {
        const wchar_t *pwCmdLineEnd = pwCmdLine + szLen - 1;
        //skip app name
        if (*pwCmdLine == '"')
        {
            while (++pwCmdLine < pwCmdLineEnd)
                if (*pwCmdLine == '"')
                {
                    if (*++pwCmdLine == ' ' || *pwCmdLine == '\t')
                        bOk = true;
                    break;
                }
        }
        else if (*pwCmdLine != ' ' && *pwCmdLine != '\t')
            while (++pwCmdLine < pwCmdLineEnd && *pwCmdLine != '"')
                if (*pwCmdLine == ' ' || *pwCmdLine == '\t')
                {
                    bOk = true;
                    break;
                }
        if (bOk)
        {
            bOk = false;
            ++pwCmdLineEnd;
            //skip spacing
            while (++pwCmdLine < pwCmdLineEnd)
                if (*pwCmdLine != ' ' && *pwCmdLine != '\t')
                {
                    bOk = true;
                    break;
                }
            if (bOk)
            {
                bOk = false;
                //get 1st arg
                if (*pwCmdLine == '"')
                {
                    pwArg = pwCmdLine + 1;
                    while (++pwCmdLine < pwCmdLineEnd)
                        if (*pwCmdLine == '"')
                        {
                            if (pwArg < pwCmdLine && (pwCmdLine + 1 == pwCmdLineEnd || pwCmdLine[1] == ' ' || pwCmdLine[1] == '\t'))
                                bOk = true;
                            break;
                        }
                }
                else
                {
                    pwArg = pwCmdLine;
                    do
                    {
                        if (++pwCmdLine == pwCmdLineEnd || *pwCmdLine == ' ' || *pwCmdLine == '\t')
                        {
                            bOk = true;
                            break;
                        }
                    } while (*pwCmdLine != '"');
                }
            }
        }
    }
    return bOk ? (*ppwArgEnd = pwCmdLine, pwArg) : nullptr;
}

[[nodiscard]]
static inline HANDLE FGetCurrentProcess()
{
    return reinterpret_cast<HANDLE>(-1);
}

//!Get environment variable value from the name
//!@param [in] pwEnvAll nonnull-ptr to the environment variables strings block
//!(usually taken from the PEB of the current process)
//!@param [in] pcEnvName: nonnull-ptr to the environment variable name to search for
//!@param [in] iEnvNameLen: length of the `pcEnvName` in symblols
//!@return if the environment variable is found, a ptr to its value is returned, otherwise nullptr
//!e.g: "VAR_NAME=VAR_VALUE\0":
//!`pcEnvName` = "VAR_NAME"
//!`iEnvNameLen` = 8
//!return value: "VAR_VALUE\0"
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline const wchar_t * FGetEnvValue(const wchar_t *pwEnvAll, const char * const pcEnvName, const uint iEnvNameLen)
{
    while (*pwEnvAll)
    {
        if (FStrCompare(pwEnvAll, pcEnvName, iEnvNameLen))
        {
            const wchar_t *pwEnvValue = pwEnvAll + iEnvNameLen;
            if (*pwEnvValue == '=')
                return ++pwEnvValue;
        }
        while (*pwEnvAll++);
    }
    return nullptr;
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline BYTE FGetImageDirLen(const UNICODE_STRING * const pusImagePathName, const BYTE iDirMaxLen)
{
    if (pusImagePathName->Length <= (MAX_PATH - 1)*sizeof(wchar_t))
    {
        const wchar_t *pwIt = pusImagePathName->Buffer;
        const wchar_t * const pwEnd = pwIt + pusImagePathName->Length/sizeof(wchar_t);
        const wchar_t *pwDirEnd = nullptr;
        while (pwIt < pwEnd)
        {
            if (!(*pwIt > ' ' && *pwIt <= '~'))
                return 0;
            if (*pwIt == '\\')
                pwDirEnd = pwIt;
            ++pwIt;
        }
        if (pwDirEnd)
        {
            const uint iDirLen = static_cast<uint>(pwDirEnd - pusImagePathName->Buffer);
            if (iDirLen <= iDirMaxLen)
                return static_cast<BYTE>(iDirLen);
        }
    }
    return 0;
}

//!@return ptr to the Thread Environment Block of calling thread
[[nodiscard]]
static inline const EX_TEB * FGetPTeb()
{
    return reinterpret_cast<const EX_TEB*>(
            #ifdef _WIN64
                __readgsqword
            #else
                __readfsdword
            #endif
                (FIELD_OFFSET(NT_TIB, Self)));
}

#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline void FSetSizeFromLen(UNICODE_STRING * const pusDst, const ptrdiff_t iLen)
{
    pusDst->MaximumLength = pusDst->Length = static_cast<USHORT>(static_cast<size_t>(iLen)*sizeof(wchar_t));
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FUpdateCurrentDirectory(const UNICODE_STRING * const pusCurDirOld, wchar_t * const pwCurDirNew, const uint iCurDirNewLen)
{
    if (pusCurDirOld->Length == iCurDirNewLen*sizeof(wchar_t) && FStrCompare(pusCurDirOld->Buffer, pwCurDirNew, iCurDirNewLen))
        return true;
    EX_USTRING usNewCurDir(pwCurDirNew, iCurDirNewLen);
    return NT_SUCCESS(RtlSetCurrentDirectory_U(&usNewCurDir));
}

//-------------------------------------------------------------------------------------------------
#ifdef NDEBUG
#define U_ASSERT(expr) (static_cast<void>(false && (expr)))
#define U_ASSERT_X(expr, msg) (static_cast<void>(false && msg && (expr)))
#define U_ASSERT_IF
#else
namespace NPrivateHelper
{
template <typename T1, typename T2, typename T3>
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static void FAssert(const T1 &pcFile, const T2 &pcFunction, const T3 &pcExpression, const int iLine, const wchar_t * const pwMsg = L"RUNTIME ASSERTION!")
{
    wchar_t wLib[] = L"user32.dll";
    EX_USTRING usLib(false, wLib);
    void *pDllHandle;
    if (NT_SUCCESS(LdrLoadDll(nullptr, nullptr, &usLib, &pDllHandle)))
    {
        char cProcName[] = "MessageBoxTimeoutW";
        EX_ASTRING asProcName(false, cProcName);
        decltype(&MessageBoxTimeoutW) MessageBoxTimeoutWReal;
        if (NT_SUCCESS(LdrGetProcedureAddressEx(pDllHandle, &asProcName, 0, reinterpret_cast<void**>(&MessageBoxTimeoutWReal), 0)))
        {
            constexpr const char cProgram[] = "Program:\n";
            constexpr const char cLine[] = "\n\nLine: ";
            constexpr const char cExpression[] = "\n\nExpression:\n";
            constexpr const size_t szSize = FCStrLen(cProgram) +
                    (MAX_PATH - 1) +
                    FCStrLen(pcFile) +
                    FCStrLen(pcFunction) +
                    FCStrLen(cLine) +
                    FCNumToDig10(static_cast<uint>(-1)/2) +
                    FCStrLen(cExpression) +
                    FCStrLen(pcExpression) +
                    1;
            wchar_t wText[szSize];
            wchar_t *pwIt = FStrCopyEx(wText, cProgram);
            const UNICODE_STRING * const pusImagePathName = &FGetPTeb()->ProcessEnvironmentBlock->ProcessParameters->ImagePathName;
            if (pusImagePathName->Length <= (MAX_PATH - 1)*sizeof(wchar_t))
                pwIt = FStrCopyEx(pwIt, pusImagePathName->Buffer, static_cast<uint>(pusImagePathName->Length/sizeof(wchar_t)));
            else
                *pwIt++ = '?';
            pwIt = FStrCopyEx(FStrCopyEx(FStrCopyEx(pwIt, pcFile), pcFunction), cLine);
            if (iLine >= 0)
                pwIt = FNumToStrEx(pwIt, static_cast<uint>(iLine));
            else
                *pwIt++ = '?';
            FStrCopy(FStrCopyEx(pwIt, cExpression), pcExpression);
            MessageBoxTimeoutWReal(nullptr, wText, pwMsg, 0x10, 0, 0xFFFFFFFF);
        }
        LdrUnloadDll(pDllHandle);
    }
    RtlExitUserProcess(0);
}
} // NPrivateHelper

#define U_ASSERT(expr) ((expr) ? static_cast<void>(false) : NPrivateHelper::FAssert("\n\nFile:\n" __FILE__ "\n\nFunction:\n", __FUNCTION__, #expr, __LINE__))
#define U_ASSERT_X(expr, msg) ((expr) ? static_cast<void>(false) : NPrivateHelper::FAssert("\n\nFile:\n" __FILE__ "\n\nFunction:\n", __FUNCTION__, #expr, __LINE__, msg))
#define U_ASSERT_IF(expr) ((expr) ? true : (NPrivateHelper::FAssert("\n\nFile:\n" __FILE__ "\n\nFunction:\n", __FUNCTION__, #expr, __LINE__), false))
#endif // NDEBUG

//-------------------------------------------------------------------------------------------------
#ifndef _WIN64
namespace NPrivateHelper
{
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FIsFunc32Hotpatchable(const BYTE * const pbtFunc)
{
    //-5   : 0xCC/0x90 : int3/nop
    //-4   : 0xCC/0x90 : int3/nop
    //-3   : 0xCC/0x90 : int3/nop
    //-2   : 0xCC/0x90 : int3/nop
    //-1   : 0xCC/0x90 : int3/nop
    //+0 +1: 0x8B 0xFF : mov edi,edi
    //+2   : 0x55      : push ebp
    //+3 +4: 0x8B 0xEC : mov ebp,esp
    constexpr const BYTE btAsm[] = {0x8B,0xFF,0x55,0x8B,0xEC};
    return FBytesCompare(pbtFunc, btAsm, sizeof(btAsm)) &&
            ((pbtFunc[-1] == 0xCC && pbtFunc[-2] == 0xCC && pbtFunc[-3] == 0xCC && pbtFunc[-4] == 0xCC && pbtFunc[-5] == 0xCC) ||
            ( pbtFunc[-1] == 0x90 && pbtFunc[-2] == 0x90 && pbtFunc[-3] == 0x90 && pbtFunc[-4] == 0x90 && pbtFunc[-5] == 0x90));
}

[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static bool FHook32Patch(BYTE *pbtOld, size_t szOffset, const bool bSaveOrigFunc)
{
    if (!bSaveOrigFunc || FIsFunc32Hotpatchable(pbtOld))
    {
        constexpr const size_t szJumpNear = 1 + sizeof(size_t);
        SIZE_T szPatchFull = bSaveOrigFunc ? (pbtOld -= szJumpNear, szJumpNear + 2) : szJumpNear;
        void *pBaseAddr = pbtOld;
        ULONG iOldProtect;
        if (NT_SUCCESS(NtProtectVirtualMemory(FGetCurrentProcess(), &pBaseAddr, &szPatchFull, PAGE_EXECUTE_READWRITE, &iOldProtect)))
        {
            szOffset -= reinterpret_cast<size_t>(pbtOld) + szJumpNear;
            *pbtOld = 0xE9;        //jump near
            FBytesCopyEx(pbtOld + 1, &szOffset, sizeof(size_t));
            if (bSaveOrigFunc)
            {
                pbtOld[5] = 0xEB;        //jump short
                pbtOld[6] = 0xF9;        //-7
            }
            ULONG iUnused;
            if (NT_SUCCESS(NtProtectVirtualMemory(FGetCurrentProcess(), &pBaseAddr, &szPatchFull, iOldProtect, &iUnused)) &&
                    NT_SUCCESS(NtFlushInstructionCache(FGetCurrentProcess(), pBaseAddr, szPatchFull)))
                return true;
        }
    }
    return false;
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FHook32Create(T1 * const pTarget, T2 * const pDetour, const bool bSaveOrigFunc)
{
    return FHook32Patch(reinterpret_cast<BYTE*>(reinterpret_cast<size_t>(pTarget)), reinterpret_cast<size_t>(pDetour), bSaveOrigFunc);
}
} // NPrivateHelper

template <typename T>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline constexpr const T * FRestoreOrigFunc(T * const pFunc)
{
    return reinterpret_cast<const T*>(reinterpret_cast<size_t>(pFunc) + 2);        //skip old "mov edi,edi" replaced by "jump short -7"
}

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FHookD(T1 * const pTarget, T2 * const pDetour)
{
    return NPrivateHelper::FHook32Create(pTarget, pDetour, false);
}

#define FHook(target) (NPrivateHelper::FHook32Create((target), (target##_Stub), false))
#define FHookSaveOrigFunc(target) (NPrivateHelper::FHook32Create((target), (target##_Stub), true))
#endif // _WIN64

#endif // HELPER_H
