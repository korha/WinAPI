/*
LICENSE:
Portions of this software using MinHook library v1.3.3 (8 Jan 2017)

================================================================================
Portions of this software are Copyright (c) 2009-2017, Tsuda Kageyu.
================================================================================
MinHook - The Minimalistic API Hooking Library for x64/x86
Copyright (C) 2009-2017 Tsuda Kageyu.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

================================================================================
Portions of this software are Copyright (c) 2008-2009, Vyacheslav Patkov.
================================================================================
Hacker Disassembler Engine 32 C
Copyright (c) 2008-2009, Vyacheslav Patkov.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-------------------------------------------------------------------------------
Hacker Disassembler Engine 64 C
Copyright (c) 2008-2009, Vyacheslav Patkov.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MINHOOK_H
#define MINHOOK_H

#include "helper.h"

//-------------------------------------------------------------------------------------------------
namespace NPrivateMinHook
{
struct [[nodiscard]] SHookEntry final
{
    size_t pTarget;
    size_t pDetour;
    void ** const ppOriginal;
    size_t pTrampoline;

    SHookEntry() = delete;
    SHookEntry(const SHookEntry &) = delete;
    SHookEntry & operator=(const SHookEntry &) = delete;
};

struct [[nodiscard]] SHde final
{
    UINT8 len;
    UINT8 p_rep;
    UINT8 p_lock;
    UINT8 p_seg;
    UINT8 p_66;
    UINT8 p_67;
#ifdef _WIN64
    UINT8 rex;
    UINT8 rex_w;
    UINT8 rex_r;
    UINT8 rex_x;
    UINT8 rex_b;
#endif
    UINT8 opcode;
    UINT8 opcode2;
    UINT8 modrm;
    UINT8 modrm_mod;
    UINT8 modrm_reg;
    UINT8 modrm_rm;
    UINT8 sib;
    UINT8 sib_scale;
    UINT8 sib_index;
    UINT8 sib_base;
    union
    {
        UINT8 imm8;
        UINT16 imm16;
        UINT32 imm32;
#ifdef _WIN64
        UINT64 imm64;
#endif
    } imm;
    union
    {
        UINT8 disp8;
        UINT16 disp16;
        UINT32 disp32;
    } disp;
    UINT32 flags;

    explicit SHde() = default;
    SHde(const SHde &) = delete;
    SHde & operator=(const SHde &) = delete;
#ifdef _WIN64
}__attribute__((packed));
#else
};
#endif

#ifdef _WIN64
static constexpr const int g_iMemSlotSize = 64;
#else
static constexpr const int g_iMemSlotSize = 32;
#endif

struct [[nodiscard]] SMemoryBlock final
{
    struct [[nodiscard]] SMemorySlot final
    {
        union
        {
            SMemorySlot *pMemSlotNext;
            BYTE btBuf[g_iMemSlotSize];
        };

        explicit SMemorySlot() = delete;
    };
    SMemoryBlock *pMemBlockNext;
    SMemorySlot *pMemSlotFree;

    explicit SMemoryBlock() = delete;
};

static SMemoryBlock *g_pMemBlocks;        //static variable

#ifdef _WIN64
static constexpr const ULONG g_iAllocationGranularity = 0x10000;

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static size_t FMinHookFindPrevFreeRegion(size_t szAddr, const size_t iMinAddr)
{
    szAddr -= szAddr%g_iAllocationGranularity;
    szAddr -= g_iAllocationGranularity;
    while (szAddr >= iMinAddr)
    {
        MEMORY_BASIC_INFORMATION memBasicInfo;
        if (!NT_SUCCESS(NtQueryVirtualMemory(FGetCurrentProcess(), reinterpret_cast<void*>(szAddr), MemoryBasicInformation, &memBasicInfo, sizeof(MEMORY_BASIC_INFORMATION), nullptr)))
            break;
        if (memBasicInfo.State == MEM_FREE)
            return szAddr;
        if (reinterpret_cast<size_t>(memBasicInfo.AllocationBase) < g_iAllocationGranularity)
            break;
        szAddr = reinterpret_cast<size_t>(memBasicInfo.AllocationBase) - g_iAllocationGranularity;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static size_t FMinHookFindNextFreeRegion(size_t szAddr, const size_t iMaxAddr)
{
    szAddr -= szAddr%g_iAllocationGranularity;
    szAddr += g_iAllocationGranularity;
    while (szAddr <= iMaxAddr)
    {
        MEMORY_BASIC_INFORMATION memBasicInfo;
        if (!NT_SUCCESS(NtQueryVirtualMemory(FGetCurrentProcess(), reinterpret_cast<void*>(szAddr), MemoryBasicInformation, &memBasicInfo, sizeof(MEMORY_BASIC_INFORMATION), nullptr)))
            break;
        if (memBasicInfo.State == MEM_FREE)
            return szAddr;
        szAddr = reinterpret_cast<size_t>(memBasicInfo.BaseAddress) + memBasicInfo.RegionSize;
        szAddr += g_iAllocationGranularity - 1;
        szAddr -= szAddr%g_iAllocationGranularity;
    }
    return 0;
}
#endif // _WIN64

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static SMemoryBlock * FMinHookGetMemoryBlock(
        #ifdef _WIN64
        const SHookEntry * const pHookEntry
        #endif
        )
{
    static constexpr const int g_iMemBlockSize = 4096;
    SMemoryBlock *pMemBlock;
#ifdef _WIN64
    static constexpr const int g_iMaxMemoryRange = 1024*1024*1024;
    size_t szMinAddr = 0x10000;
    size_t szMaxAddr = 0x7FFFFFFEFFFF;
    if (pHookEntry->pTarget > g_iMaxMemoryRange && szMinAddr < pHookEntry->pTarget - g_iMaxMemoryRange)
        szMinAddr = pHookEntry->pTarget - g_iMaxMemoryRange;
    if (szMaxAddr > pHookEntry->pTarget + g_iMaxMemoryRange)
        szMaxAddr = pHookEntry->pTarget + g_iMaxMemoryRange;
    szMaxAddr -= g_iMemBlockSize - 1;
#endif
    for (pMemBlock = g_pMemBlocks; pMemBlock; pMemBlock = pMemBlock->pMemBlockNext)
    {
#ifdef _WIN64
        if (reinterpret_cast<size_t>(pMemBlock) < szMinAddr || reinterpret_cast<size_t>(pMemBlock) >= szMaxAddr)
            continue;
#endif
        if (pMemBlock->pMemSlotFree)
            return pMemBlock;
    }
#ifdef _WIN64
    size_t szAddr = pHookEntry->pTarget;
    while (szAddr >= szMinAddr)
    {
        if (!(szAddr = FMinHookFindPrevFreeRegion(szAddr, szMinAddr)))
            break;
        void *pBaseAddr = reinterpret_cast<void*>(szAddr);
        SIZE_T szRegion = g_iMemBlockSize;
        if (NT_SUCCESS(NtAllocateVirtualMemory(FGetCurrentProcess(), &pBaseAddr, 0, &szRegion, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
            pMemBlock = static_cast<SMemoryBlock*>(pBaseAddr);
        else
            pMemBlock = nullptr;
        if (pMemBlock)
            break;
    }
    if (!pMemBlock)
    {
        szAddr = pHookEntry->pTarget;
        while (szAddr <= szMaxAddr)
        {
            if (!(szAddr = FMinHookFindNextFreeRegion(szAddr, szMaxAddr)))
                break;
            void *pBaseAddr = reinterpret_cast<void*>(szAddr);
            SIZE_T szRegion = g_iMemBlockSize;
            if (NT_SUCCESS(NtAllocateVirtualMemory(FGetCurrentProcess(), &pBaseAddr, 0, &szRegion, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
                pMemBlock = static_cast<SMemoryBlock*>(pBaseAddr);
            else
                pMemBlock = nullptr;
            if (pMemBlock)
                break;
        }
    }
#else
    void *pBaseAddr = nullptr;
    SIZE_T szRegion = g_iMemBlockSize;
    if (NT_SUCCESS(NtAllocateVirtualMemory(FGetCurrentProcess(), &pBaseAddr, 0, &szRegion, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)))
        pMemBlock = static_cast<SMemoryBlock*>(pBaseAddr);
    else
        pMemBlock = nullptr;
#endif // _WIN64
    if (pMemBlock)
    {
        SMemoryBlock::SMemorySlot *pMemSlot = pointer_cast<SMemoryBlock::SMemorySlot*>(pMemBlock) + 1;
        pMemBlock->pMemSlotFree = nullptr;
        do
        {
            pMemSlot->pMemSlotNext = pMemBlock->pMemSlotFree;
            pMemBlock->pMemSlotFree = pMemSlot++;
        } while (reinterpret_cast<size_t>(pMemSlot) - reinterpret_cast<size_t>(pMemBlock) <= g_iMemBlockSize - g_iMemSlotSize);
        pMemBlock->pMemBlockNext = g_pMemBlocks;
        g_pMemBlocks = pMemBlock;
    }
    return pMemBlock;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
static bool FMinHookIsExecutableAddress(const size_t szAddr)
{
    void *pAddress = reinterpret_cast<void*>(szAddr);
    MEMORY_BASIC_INFORMATION memBasicInfo;
    return NT_SUCCESS(NtQueryVirtualMemory(FGetCurrentProcess(), pAddress, MemoryBasicInformation, &memBasicInfo, sizeof(MEMORY_BASIC_INFORMATION), nullptr)) &&
            memBasicInfo.State == MEM_COMMIT &&
            (memBasicInfo.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static UINT FMinHookDisasm(const void * const pCode, SHde * const pHde)
{
    static constexpr const int g_iFModrm = 0x1;
    static constexpr const int g_iFSib = 0x2;
    static constexpr const int g_iFImm8 = 0x4;
    static constexpr const int g_iFImm16 = 0x8;
    static constexpr const int g_iFImm32 = 0x10;
    static constexpr const int g_iFError = 0x1000;
    static constexpr const int g_iFErrorOpcode = 0x2000;
    static constexpr const int g_iFErrorLength = 0x4000;
    static constexpr const int g_iFErrorLock = 0x8000;
    static constexpr const int g_iFErrorOperand = 0x10000;
    static constexpr const int g_iCModrm = 0x1;
    static constexpr const int g_iCImm8 = 0x2;
    static constexpr const int g_iCImm16 = 0x4;
    static constexpr const int g_iCImmP66 = 0x10;
    static constexpr const int g_iCRel8 = 0x20;
    static constexpr const int g_iCRel32 = 0x40;
    static constexpr const int g_iCGroup = 0x80;
    static constexpr const int g_iCError = 0xFF;
    static constexpr const int g_iPreNone = 0x1;
    static constexpr const int g_iPreF2 = 0x2;
    static constexpr const int g_iPreF3 = 0x4;
    static constexpr const int g_iPre66 = 0x8;
    static constexpr const int g_iPre67 = 0x10;
    static constexpr const int g_iPreLock = 0x20;
    static constexpr const int g_iPreSeg = 0x40;
    static constexpr const int g_iDeltaOpcodes = 0x4A;
    UINT8 iC = 0, iCflags, iOpcode, iPref = 0;
    const UINT8 *piP = static_cast<const UINT8*>(pCode);
    UINT8 iMMod, iMReg, iMRm, iDispSize = 0;
    FBytesZero(pHde, sizeof(SHde));
    for (UINT8 i = 16; i; --i)
        switch (iC = *piP++)
        {
        case 0xF3:
            pHde->p_rep = iC;
            iPref |= g_iPreF3;
            break;
        case 0xF2:
            pHde->p_rep = iC;
            iPref |= g_iPreF2;
            break;
        case 0xF0:
            pHde->p_lock = iC;
            iPref |= g_iPreLock;
            break;
        case 0x26:
        case 0x2E:
        case 0x36:
        case 0x3E:
        case 0x64:
        case 0x65:
            pHde->p_seg = iC;
            iPref |= g_iPreSeg;
            break;
        case 0x66:
            pHde->p_66 = iC;
            iPref |= g_iPre66;
            break;
        case 0x67:
            pHde->p_67 = iC;
            iPref |= g_iPre67;
            break;
        default:
            goto pref_done;
        }
pref_done:
    pHde->flags = static_cast<UINT32>(iPref) << 23;
    if (!iPref)
        iPref |= g_iPreNone;
#ifdef _WIN64
    static constexpr const int g_iFImm64 = 0x20;
    static constexpr const int g_iFDisp8 = 0x40;
    static constexpr const int g_iFDisp16 = 0x80;
    static constexpr const int g_iFDisp32 = 0x100;
    static constexpr const int g_iFRelative = 0x200;
    static constexpr const int g_iFPrefixRex = 0x40000000;
    static constexpr const int g_iDeltaFpuReg = 0xFD;
    static constexpr const int g_iDeltaFpuModrm = 0x104;
    static constexpr const int g_iDeltaPrefixes = 0x13C;
    static constexpr const int g_iDeltaOpLockOk = 0x1AE;
    static constexpr const int g_iDeltaOp2LockOk = 0x1C6;
    static constexpr const int g_iDeltaOpOnlyMem = 0x1D8;
    static constexpr const int g_iDeltaOp2OnlyMem = 0x1E7;
    static constexpr const BYTE btHdeTable[] = {
        0xA5,0xAA,0xA5,0xB8,0xA5,0xAA,0xA5,0xAA,0xA5,0xB8,0xA5,0xB8,0xA5,0xB8,0xA5,0xB8,
        0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xAC,0xC0,0xCC,0xC0,0xA1,0xA1,0xA1,0xA1,
        0xB1,0xA5,0xA5,0xA6,0xC0,0xC0,0xD7,0xDA,0xE0,0xC0,0xE4,0xC0,0xEA,0xEA,0xE0,0xE0,
        0x98,0xC8,0xEE,0xF1,0xA5,0xD3,0xA5,0xA5,0xA1,0xEA,0x9E,0xC0,0xC0,0xC2,0xC0,0xE6,
        0x03,0x7F,0x11,0x7F,0x01,0x7F,0x01,0x3F,0x01,0x01,0xAB,0x8B,0x90,0x64,0x5B,0x5B,
        0x5B,0x5B,0x5B,0x92,0x5B,0x5B,0x76,0x90,0x92,0x92,0x5B,0x5B,0x5B,0x5B,0x5B,0x5B,
        0x5B,0x5B,0x5B,0x5B,0x5B,0x5B,0x6A,0x73,0x90,0x5B,0x52,0x52,0x52,0x52,0x5B,0x5B,
        0x5B,0x5B,0x77,0x7C,0x77,0x85,0x5B,0x5B,0x70,0x5B,0x7A,0xAF,0x76,0x76,0x5B,0x5B,
        0x5B,0x5B,0x5B,0x5B,0x5B,0x5B,0x5B,0x5B,0x5B,0x86,0x01,0x03,0x01,0x04,0x03,0xD5,
        0x03,0xD5,0x03,0xCC,0x01,0xBC,0x03,0xF0,0x03,0x03,0x04,0x00,0x50,0x50,0x50,0x50,
        0xFF,0x20,0x20,0x20,0x20,0x01,0x01,0x01,0x01,0xC4,0x02,0x10,0xFF,0xFF,0xFF,0x01,
        0x00,0x03,0x11,0xFF,0x03,0xC4,0xC6,0xC8,0x02,0x10,0x00,0xFF,0xCC,0x01,0x01,0x01,
        0x00,0x00,0x00,0x00,0x01,0x01,0x03,0x01,0xFF,0xFF,0xC0,0xC2,0x10,0x11,0x02,0x03,
        0x01,0x01,0x01,0xFF,0xFF,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
        0x10,0x10,0x10,0x10,0x02,0x10,0x00,0x00,0xC6,0xC8,0x02,0x02,0x02,0x02,0x06,0x00,
        0x04,0x00,0x02,0xFF,0x00,0xC0,0xC2,0x01,0x01,0x03,0x03,0x03,0xCA,0x40,0x00,0x0A,
        0x00,0x04,0x00,0x00,0x00,0x00,0x7F,0x00,0x33,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
        0xFF,0xBF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0xBF,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0xFF,0x40,0x40,0x40,0x40,
        0x41,0x49,0x40,0x40,0x40,0x40,0x4C,0x42,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
        0x4F,0x44,0x53,0x40,0x40,0x40,0x44,0x57,0x43,0x5C,0x40,0x60,0x40,0x40,0x40,0x40,
        0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x64,0x66,0x6E,0x6B,0x40,0x40,
        0x6A,0x46,0x40,0x40,0x44,0x46,0x40,0x40,0x5B,0x44,0x40,0x40,0x00,0x00,0x00,0x00,
        0x06,0x06,0x06,0x06,0x01,0x06,0x06,0x02,0x06,0x06,0x00,0x06,0x00,0x0A,0x0A,0x00,
        0x00,0x00,0x02,0x07,0x07,0x06,0x02,0x0D,0x06,0x06,0x06,0x0E,0x05,0x05,0x02,0x02,
        0x00,0x00,0x04,0x04,0x04,0x04,0x05,0x06,0x06,0x06,0x00,0x00,0x00,0x0E,0x00,0x00,
        0x08,0x00,0x10,0x00,0x18,0x00,0x20,0x00,0x28,0x00,0x30,0x00,0x80,0x01,0x82,0x01,
        0x86,0x00,0xF6,0xCF,0xFE,0x3F,0xAB,0x00,0xB0,0x00,0xB1,0x00,0xB3,0x00,0xBA,0xF8,
        0xBB,0x00,0xC0,0x00,0xC1,0x00,0xC7,0xBF,0x62,0xFF,0x00,0x8D,0xFF,0x00,0xC4,0xFF,
        0x00,0xC5,0xFF,0x00,0xFF,0xFF,0xEB,0x01,0xFF,0x0E,0x12,0x08,0x00,0x13,0x09,0x00,
        0x16,0x08,0x00,0x17,0x09,0x00,0x2B,0x09,0x00,0xAE,0xFF,0x07,0xB2,0xFF,0x00,0xB4,
        0xFF,0x00,0xB5,0xFF,0x00,0xC3,0x01,0x00,0xC7,0xFF,0xBF,0xE7,0x08,0x00,0xF0,0x02,
        0x00
    };
    const UINT8 *piHt = btHdeTable;
    UINT8 iOp64 = 0;
    if ((iC & 0xF0) == 0x40)
    {
        pHde->flags |= g_iFPrefixRex;
        if ((pHde->rex_w = (iC & 0xF) >> 3) && (*piP & 0xF8) == 0xB8)
            ++iOp64;
        pHde->rex_r = (iC & 7) >> 2;
        pHde->rex_x = (iC & 3) >> 1;
        pHde->rex_b = iC & 1;
        if (((iC = *piP++) & 0xF0) == 0x40)
        {
            iOpcode = iC;
            goto error_opcode;
        }
    }
    if ((pHde->opcode = iC) == 0xF)
    {
        pHde->opcode2 = iC = *piP++;
        piHt += g_iDeltaOpcodes;
    }
    else if (iC >= 0xA0 && iC <= 0xA3)
    {
        ++iOp64;
        if (iPref & g_iPre67)
            iPref |= g_iPre66;
        else
            iPref = static_cast<UINT8>(iPref & ~static_cast<UINT>(g_iPre66));
    }
    iOpcode = iC;
    iCflags = piHt[piHt[iOpcode/4] + (iOpcode%4)];
    if (iCflags == g_iCError)
    {
error_opcode:
        pHde->flags |= g_iFError | g_iFErrorOpcode;
        iCflags = 0;
        if ((iOpcode & -3) == 0x24)
            ++iCflags;
    }
    UINT8 iX = 0;
    if (iCflags & g_iCGroup)
    {
        const UINT16 iT = *pointer_cast<const UINT16*>(piHt + (iCflags & 0x7F));
        iCflags = static_cast<UINT8>(iT);
        iX = static_cast<UINT8>(iT >> 8);
    }
    if (pHde->opcode2)
    {
        piHt = btHdeTable + g_iDeltaPrefixes;
        if (piHt[piHt[iOpcode/4] + (iOpcode%4)] & iPref)
            pHde->flags |= g_iFError | g_iFErrorOpcode;
    }
    if (iCflags & g_iCModrm)
    {
        pHde->flags |= g_iFModrm;
        pHde->modrm = iC = *piP++;
        pHde->modrm_mod = iMMod = static_cast<UINT8>(iC >> 6);
        pHde->modrm_rm = iMRm = iC & 7;
        pHde->modrm_reg = iMReg = (iC & 0x3F) >> 3;
        if (iX && ((iX << iMReg) & 0x80))
            pHde->flags |= g_iFError | g_iFErrorOpcode;
        if (!pHde->opcode2 && iOpcode >= 0xD9 && iOpcode <= 0xDF)
        {
            UINT8 iT = static_cast<UINT8>(iOpcode - 0xD9);
            if (iMMod == 3)
            {
                piHt = btHdeTable + g_iDeltaFpuModrm + iT*8;
                iT = static_cast<UINT8>(piHt[iMReg] << iMRm);
            }
            else
            {
                piHt = btHdeTable + g_iDeltaFpuReg;
                iT = static_cast<UINT8>(piHt[iT] << iMReg);
            }
            if (iT & 0x80)
                pHde->flags |= g_iFError | g_iFErrorOpcode;
        }
        if (iPref & g_iPreLock)
        {
            if (iMMod == 3)
                pHde->flags |= g_iFError | g_iFErrorLock;
            else
            {
                const UINT8 *piTableEnd;
                UINT8 iOp = iOpcode;
                if (pHde->opcode2)
                {
                    piHt = btHdeTable + g_iDeltaOp2LockOk;
                    piTableEnd = piHt + (g_iDeltaOpOnlyMem - g_iDeltaOp2LockOk);
                }
                else
                {
                    piHt = btHdeTable + g_iDeltaOpLockOk;
                    piTableEnd = piHt + (g_iDeltaOp2LockOk - g_iDeltaOpLockOk);
                    iOp = static_cast<UINT8>(iOp & -2);
                }
                for (; piHt != piTableEnd; ++piHt)
                    if (*piHt++ == iOp)
                    {
                        if (!((*piHt << iMReg) & 0x80))
                            goto no_lock_error;
                        else
                            break;
                    }
                pHde->flags |= g_iFError | g_iFErrorLock;
no_lock_error:
                ;
            }
        }
        if (pHde->opcode2)
        {
            switch (iOpcode)
            {
            case 0x20:
            case 0x22:
                iMMod = 3;
                if (iMReg > 4 || iMReg == 1)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x21:
            case 0x23:
                iMMod = 3;
                if (iMReg == 4 || iMReg == 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }
        else
        {
            switch (iOpcode)
            {
            case 0x8C:
                if (iMReg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x8E:
                if (iMReg == 1 || iMReg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }
        if (iMMod == 3)
        {
            const UINT8 *piTableEnd;
            if (pHde->opcode2)
            {
                piHt = btHdeTable + g_iDeltaOp2OnlyMem;
                piTableEnd = piHt + (sizeof(btHdeTable) - g_iDeltaOp2OnlyMem);
            }
            else
            {
                piHt = btHdeTable + g_iDeltaOpOnlyMem;
                piTableEnd = piHt + (g_iDeltaOp2OnlyMem - g_iDeltaOpOnlyMem);
            }
            for (; piHt != piTableEnd; piHt += 2)
                if (*piHt++ == iOpcode)
                {
                    if (*piHt++ & iPref && !((*piHt << iMReg) & 0x80))
                        goto error_operand;
                    else
                        break;
                }
            goto no_error_operand;
        }
        else if (pHde->opcode2)
        {
            switch (iOpcode)
            {
            case 0x50:
            case 0xD7:
            case 0xF7:
                if (iPref & (g_iPreNone | g_iPre66))
                    goto error_operand;
                break;
            case 0xD6:
                if (iPref & (g_iPreF2 | g_iPreF3))
                    goto error_operand;
                break;
            case 0xC5:
                goto error_operand;
            }
            goto no_error_operand;
        }
        else
            goto no_error_operand;
error_operand:
        pHde->flags |= g_iFError | g_iFErrorOperand;
no_error_operand:
        iC = *piP++;
        if (iMReg <= 1)
        {
            if (iOpcode == 0xF6)
                iCflags |= g_iCImm8;
            else if (iOpcode == 0xF7)
                iCflags |= g_iCImmP66;
        }
        switch (iMMod)
        {
        case 0:
            if (iPref & g_iPre67)
            {
                if (iMRm == 6)
                    iDispSize = 2;
            }
            else if (iMRm == 5)
                iDispSize = 4;
            break;
        case 1:
            iDispSize = 1;
            break;
        case 2:
            iDispSize = 2;
            if (!(iPref & g_iPre67))
                iDispSize = static_cast<UINT8>(iDispSize << 1);
        }
        if (iMMod != 3 && iMRm == 4)
        {
            pHde->flags |= g_iFSib;
            ++piP;
            pHde->sib = iC;
            pHde->sib_scale = static_cast<UINT8>(iC >> 6);
            pHde->sib_index = (iC & 0x3F) >> 3;
            if ((pHde->sib_base = iC & 7) == 5 && !(iMMod & 1))
                iDispSize = 4;
        }
        --piP;
        switch (iDispSize)
        {
        case 1:
            pHde->flags |= g_iFDisp8;
            pHde->disp.disp8 = *piP;
            break;
        case 2:
            pHde->flags |= g_iFDisp16;
            pHde->disp.disp16 = *pointer_cast<const UINT16*>(piP);
            break;
        case 4:
            pHde->flags |= g_iFDisp32;
            pHde->disp.disp32 = *pointer_cast<const UINT32*>(piP);
        }
        piP += iDispSize;
    }
    else if (iPref & g_iPreLock)
        pHde->flags |= g_iFError | g_iFErrorLock;
    if (iCflags & g_iCImmP66)
    {
        if (iCflags & g_iCRel32)
        {
            if (iPref & g_iPre66)
            {
                pHde->flags |= g_iFImm16 | g_iFRelative;
                pHde->imm.imm16 = *pointer_cast<const UINT16*>(piP);
                piP += 2;
                goto disasm_done;
            }
            goto rel32_ok;
        }
        if (iOp64)
        {
            pHde->flags |= g_iFImm64;
            pHde->imm.imm64 = *pointer_cast<const UINT64*>(piP);
            piP += 8;
        }
        else if (!(iPref & g_iPre66))
        {
            pHde->flags |= g_iFImm32;
            pHde->imm.imm32 = *pointer_cast<const UINT32*>(piP);
            piP += 4;
        }
        else
            goto imm16_ok;
    }
    if (iCflags & g_iCImm16)
    {
imm16_ok:
        pHde->flags |= g_iFImm16;
        pHde->imm.imm16 = *pointer_cast<const UINT16*>(piP);
        piP += 2;
    }
#else
    static constexpr const int g_iFDisp8 = 0x20;
    static constexpr const int g_iFDisp16 = 0x40;
    static constexpr const int g_iFDisp32 = 0x80;
    static constexpr const int g_iFRelative = 0x100;
    static constexpr const int g_iF2Imm16 = 0x800;
    static constexpr const int g_iDeltaFpuReg = 0xF1;
    static constexpr const int g_iDeltaFpuModrm = 0xF8;
    static constexpr const int g_iDeltaPrefixes = 0x130;
    static constexpr const int g_iDeltaOpLockOk = 0x1A1;
    static constexpr const int g_iDeltaOp2LockOk = 0x1B9;
    static constexpr const int g_iDeltaOpOnlyMem = 0x1CB;
    static constexpr const int g_iDeltaOp2OnlyMem = 0x1DA;
    static constexpr const BYTE btHdeTable[] = {
        0xA3,0xA8,0xA3,0xA8,0xA3,0xA8,0xA3,0xA8,0xA3,0xA8,0xA3,0xA8,0xA3,0xA8,0xA3,0xA8,
        0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAC,0xAA,0xB2,0xAA,0x9F,0x9F,0x9F,0x9F,
        0xB5,0xA3,0xA3,0xA4,0xAA,0xAA,0xBA,0xAA,0x96,0xAA,0xA8,0xAA,0xC3,0xC3,0x96,0x96,
        0xB7,0xAE,0xD6,0xBD,0xA3,0xC5,0xA3,0xA3,0x9F,0xC3,0x9C,0xAA,0xAA,0xAC,0xAA,0xBF,
        0x03,0x7F,0x11,0x7F,0x01,0x7F,0x01,0x3F,0x01,0x01,0x90,0x82,0x7D,0x97,0x59,0x59,
        0x59,0x59,0x59,0x7F,0x59,0x59,0x60,0x7D,0x7F,0x7F,0x59,0x59,0x59,0x59,0x59,0x59,
        0x59,0x59,0x59,0x59,0x59,0x59,0x9A,0x88,0x7D,0x59,0x50,0x50,0x50,0x50,0x59,0x59,
        0x59,0x59,0x61,0x94,0x61,0x9E,0x59,0x59,0x85,0x59,0x92,0xA3,0x60,0x60,0x59,0x59,
        0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x9F,0x01,0x03,0x01,0x04,0x03,0xD5,
        0x03,0xCC,0x01,0xBC,0x03,0xF0,0x10,0x10,0x10,0x10,0x50,0x50,0x50,0x50,0x14,0x20,
        0x20,0x20,0x20,0x01,0x01,0x01,0x01,0xC4,0x02,0x10,0x00,0x00,0x00,0x00,0x01,0x01,
        0xC0,0xC2,0x10,0x11,0x02,0x03,0x11,0x03,0x03,0x04,0x00,0x00,0x14,0x00,0x02,0x00,
        0x00,0xC6,0xC8,0x02,0x02,0x02,0x02,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,
        0xFF,0xCA,0x01,0x01,0x01,0x00,0x06,0x00,0x04,0x00,0xC0,0xC2,0x01,0x01,0x03,0x01,
        0xFF,0xFF,0x01,0x00,0x03,0xC4,0xC4,0xC6,0x03,0x01,0x01,0x01,0xFF,0x03,0x03,0x03,
        0xC8,0x40,0x00,0x0A,0x00,0x04,0x00,0x00,0x00,0x00,0x7F,0x00,0x33,0x01,0x00,0x00,
        0x00,0x00,0x00,0x00,0xFF,0xBF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0xFF,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
        0x00,0x00,0x00,0xBF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0xFF,
        0x4A,0x4A,0x4A,0x4A,0x4B,0x52,0x4A,0x4A,0x4A,0x4A,0x4F,0x4C,0x4A,0x4A,0x4A,0x4A,
        0x4A,0x4A,0x4A,0x4A,0x55,0x45,0x40,0x4A,0x4A,0x4A,0x45,0x59,0x4D,0x46,0x4A,0x5D,
        0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x4A,0x61,0x63,
        0x67,0x4E,0x4A,0x4A,0x6B,0x6D,0x4A,0x4A,0x45,0x6D,0x4A,0x4A,0x44,0x45,0x4A,0x4A,
        0x00,0x00,0x00,0x02,0x0D,0x06,0x06,0x06,0x06,0x0E,0x00,0x00,0x00,0x00,0x06,0x06,
        0x06,0x00,0x06,0x06,0x02,0x06,0x00,0x0A,0x0A,0x07,0x07,0x06,0x02,0x05,0x05,0x02,
        0x02,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x0E,0x05,0x06,0x06,0x06,0x01,
        0x06,0x00,0x00,0x08,0x00,0x10,0x00,0x18,0x00,0x20,0x00,0x28,0x00,0x30,0x00,0x80,
        0x01,0x82,0x01,0x86,0x00,0xF6,0xCF,0xFE,0x3F,0xAB,0x00,0xB0,0x00,0xB1,0x00,0xB3,
        0x00,0xBA,0xF8,0xBB,0x00,0xC0,0x00,0xC1,0x00,0xC7,0xBF,0x62,0xFF,0x00,0x8D,0xFF,
        0x00,0xC4,0xFF,0x00,0xC5,0xFF,0x00,0xFF,0xFF,0xEB,0x01,0xFF,0x0E,0x12,0x08,0x00,
        0x13,0x09,0x00,0x16,0x08,0x00,0x17,0x09,0x00,0x2B,0x09,0x00,0xAE,0xFF,0x07,0xB2,
        0xFF,0x00,0xB4,0xFF,0x00,0xB5,0xFF,0x00,0xC3,0x01,0x00,0xC7,0xFF,0xBF,0xE7,0x08,
        0x00,0xF0,0x02,0x00
    };
    const UINT8 *piHt = btHdeTable;
    if ((pHde->opcode = iC) == 0xF)
    {
        pHde->opcode2 = iC = *piP++;
        piHt += g_iDeltaOpcodes;
    }
    else if (iC >= 0xA0 && iC <= 0xA3)
    {
        if (iPref & g_iPre67)
            iPref |= g_iPre66;
        else
            iPref = static_cast<UINT8>(iPref & ~static_cast<UINT>(g_iPre66));
    }
    iOpcode = iC;
    iCflags = piHt[piHt[iOpcode/4] + (iOpcode%4)];
    if (iCflags == g_iCError)
    {
        pHde->flags |= g_iFError | g_iFErrorOpcode;
        iCflags = 0;
        if ((iOpcode & -3) == 0x24)
            ++iCflags;
    }
    UINT8 iX = 0;
    if (iCflags & g_iCGroup)
    {
        const UINT16 t = *pointer_cast<const UINT16*>(piHt + (iCflags & 0x7F));
        iCflags = static_cast<UINT8>(t);
        iX = static_cast<UINT8>(t >> 8);
    }
    if (pHde->opcode2)
    {
        piHt = btHdeTable + g_iDeltaPrefixes;
        if (piHt[piHt[iOpcode/4] + (iOpcode%4)] & iPref)
            pHde->flags |= g_iFError | g_iFErrorOpcode;
    }
    if (iCflags & g_iCModrm)
    {
        pHde->flags |= g_iFModrm;
        pHde->modrm = iC = *piP++;
        pHde->modrm_mod = iMMod = static_cast<UINT8>(iC >> 6);
        pHde->modrm_rm = iMRm = iC & 7;
        pHde->modrm_reg = iMReg = (iC & 0x3F) >> 3;
        if (iX && ((iX << iMReg) & 0x80))
            pHde->flags |= g_iFError | g_iFErrorOpcode;
        if (!pHde->opcode2 && iOpcode >= 0xD9 && iOpcode <= 0xDF)
        {
            UINT8 iT = static_cast<UINT8>(iOpcode - 0xD9);
            if (iMMod == 3)
            {
                piHt = btHdeTable + g_iDeltaFpuModrm + iT*8;
                iT = static_cast<UINT8>(piHt[iMReg] << iMRm);
            }
            else
            {
                piHt = btHdeTable + g_iDeltaFpuReg;
                iT = static_cast<UINT8>(piHt[iT] << iMReg);
            }
            if (iT & 0x80)
                pHde->flags |= g_iFError | g_iFErrorOpcode;
        }
        if (iPref & g_iPreLock)
        {
            if (iMMod == 3)
                pHde->flags |= g_iFError | g_iFErrorLock;
            else
            {
                const UINT8 *piTableEnd;
                UINT8 iOp = iOpcode;
                if (pHde->opcode2)
                {
                    piHt = btHdeTable + g_iDeltaOp2LockOk;
                    piTableEnd = piHt + (g_iDeltaOpOnlyMem - g_iDeltaOp2LockOk);
                }
                else
                {
                    piHt = btHdeTable + g_iDeltaOpLockOk;
                    piTableEnd = piHt + (g_iDeltaOp2LockOk - g_iDeltaOpLockOk);
                    iOp = static_cast<UINT8>(iOp & -2);
                }
                for (; piHt != piTableEnd; ++piHt)
                    if (*piHt++ == iOp)
                    {
                        if (!((*piHt << iMReg) & 0x80))
                            goto no_lock_error;
                        else
                            break;
                    }
                pHde->flags |= g_iFError | g_iFErrorLock;
no_lock_error:
                ;
            }
        }
        if (pHde->opcode2)
        {
            switch (iOpcode)
            {
            case 0x20:
            case 0x22:
                iMMod = 3;
                if (iMReg > 4 || iMReg == 1)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x21:
            case 0x23:
                iMMod = 3;
                if (iMReg == 4 || iMReg == 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }
        else
        {
            switch (iOpcode)
            {
            case 0x8C:
                if (iMReg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x8E:
                if (iMReg == 1 || iMReg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }

        if (iMMod == 3)
        {
            const UINT8 *piTableEnd;
            if (pHde->opcode2)
            {
                piHt = btHdeTable + g_iDeltaOp2OnlyMem;
                piTableEnd = piHt + (sizeof(btHdeTable) - g_iDeltaOp2OnlyMem);
            }
            else
            {
                piHt = btHdeTable + g_iDeltaOpOnlyMem;
                piTableEnd = piHt + (g_iDeltaOp2OnlyMem - g_iDeltaOpOnlyMem);
            }
            for (; piHt != piTableEnd; piHt += 2)
                if (*piHt++ == iOpcode)
                {
                    if (*piHt++ & iPref && !((*piHt << iMReg) & 0x80))
                        goto error_operand;
                    else
                        break;
                }
            goto no_error_operand;
        }
        else if (pHde->opcode2)
        {
            switch (iOpcode)
            {
            case 0x50:
            case 0xD7:
            case 0xF7:
                if (iPref & (g_iPreNone | g_iPre66))
                    goto error_operand;
                break;
            case 0xD6:
                if (iPref & (g_iPreF2 | g_iPreF3))
                    goto error_operand;
                break;
            case 0xC5:
                goto error_operand;
            }
            goto no_error_operand;
        }
        else
            goto no_error_operand;
error_operand:
        pHde->flags |= g_iFError | g_iFErrorOperand;
no_error_operand:
        iC = *piP++;
        if (iMReg <= 1)
        {
            if (iOpcode == 0xF6)
                iCflags |= g_iCImm8;
            else if (iOpcode == 0xF7)
                iCflags |= g_iCImmP66;
        }
        switch (iMMod)
        {
        case 0:
            if (iPref & g_iPre67)
            {
                if (iMRm == 6)
                    iDispSize = 2;
            }
            else if (iMRm == 5)
                iDispSize = 4;
            break;
        case 1:
            iDispSize = 1;
            break;
        case 2:
            iDispSize = 2;
            if (!(iPref & g_iPre67))
                iDispSize = static_cast<UINT8>(iDispSize << 1);
        }
        if (iMMod != 3 && iMRm == 4 && !(iPref & g_iPre67))
        {
            pHde->flags |= g_iFSib;
            ++piP;
            pHde->sib = iC;
            pHde->sib_scale = static_cast<UINT8>(iC >> 6);
            pHde->sib_index = (iC & 0x3F) >> 3;
            if ((pHde->sib_base = iC & 7) == 5 && !(iMMod & 1))
                iDispSize = 4;
        }
        --piP;
        switch (iDispSize)
        {
        case 1:
            pHde->flags |= g_iFDisp8;
            pHde->disp.disp8 = *piP;
            break;
        case 2:
            pHde->flags |= g_iFDisp16;
            pHde->disp.disp16 = *pointer_cast<const UINT16*>(piP);
            break;
        case 4:
            pHde->flags |= g_iFDisp32;
            pHde->disp.disp32 = *pointer_cast<const UINT32*>(piP);
        }
        piP += iDispSize;
    }
    else if (iPref & g_iPreLock)
        pHde->flags |= g_iFError | g_iFErrorLock;
    if (iCflags & g_iCImmP66)
    {
        if (iCflags & g_iCRel32)
        {
            if (iPref & g_iPre66)
            {
                pHde->flags |= g_iFImm16 | g_iFRelative;
                pHde->imm.imm16 = *pointer_cast<const UINT16*>(piP);
                piP += 2;
                goto disasm_done;
            }
            goto rel32_ok;
        }
        if (iPref & g_iPre66)
        {
            pHde->flags |= g_iFImm16;
            pHde->imm.imm16 = *pointer_cast<const UINT16*>(piP);
            piP += 2;
        }
        else
        {
            pHde->flags |= g_iFImm32;
            pHde->imm.imm32 = *pointer_cast<const UINT32*>(piP);
            piP += 4;
        }
    }
    if (iCflags & g_iCImm16)
    {
        if (pHde->flags & g_iFImm32)
        {
            pHde->flags |= g_iFImm16;
            pHde->disp.disp16 = *pointer_cast<const UINT16*>(piP);
        }
        else if (pHde->flags & g_iFImm16)
        {
            pHde->flags |= g_iF2Imm16;
            pHde->disp.disp16 = *pointer_cast<const UINT16*>(piP);
        }
        else
        {
            pHde->flags |= g_iFImm16;
            pHde->imm.imm16 = *pointer_cast<const UINT16*>(piP);
        }
        piP += 2;
    }
#endif // _WIN64
    if (iCflags & g_iCImm8)
    {
        pHde->flags |= g_iFImm8;
        pHde->imm.imm8 = *piP++;
    }
    if (iCflags & g_iCRel32)
    {
rel32_ok:
        pHde->flags |= g_iFImm32 | g_iFRelative;
        pHde->imm.imm32 = *pointer_cast<const UINT32*>(piP);
        piP += 4;
    }
    else if (iCflags & g_iCRel8)
    {
        pHde->flags |= g_iFImm8 | g_iFRelative;
        pHde->imm.imm8 = *piP++;
    }
disasm_done:
    if ((pHde->len = static_cast<UINT8>(piP - static_cast<const UINT8*>(pCode))) > 15)
    {
        pHde->flags |= g_iFError | g_iFErrorLength;
        pHde->len = 15;
    }
    return pHde->len;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static bool FMinHookIsCodePadding(const BYTE * const pbtInst, const UINT iSize)
{
    if (*pbtInst != 0x00 && *pbtInst != 0x90 && *pbtInst != 0xCC)
        return false;
    const BYTE *pbtIt = pbtInst;
    const BYTE * const pbtItEnd = pbtInst + iSize;
    while (++pbtIt < pbtItEnd)
        if (*pbtIt != *pbtInst)
            return false;
    return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static bool FMinHookCreateTrampoline(SHookEntry * const pHookEntry)
{
    static constexpr const int g_iFError = 0x1000;
    static constexpr const int g_iTargetBoundaries = 8;

    struct [[nodiscard]] SJmpRelShort final
    {
        UINT8 opcode;
        UINT8 operand;

        explicit SJmpRelShort() = delete;
    };//__attribute__((packed));
#ifdef _WIN64
    struct [[nodiscard]] SJmpRel final
    {
        UINT8 opcode;
        UINT32 operand;

        explicit SJmpRel() = delete;
    }__attribute__((packed));
    struct [[nodiscard]] SJmpAbs final
    {
        UINT8 opcode0;
        UINT8 opcode1;
        UINT32 dummy;
        UINT64 address;

        SJmpAbs() = default;
        SJmpAbs(const SJmpAbs &) = delete;
        SJmpAbs & operator=(const SJmpAbs &) = delete;
    }__attribute__((packed)) sJmp = {0xFF,0x25,0x00,0x00};
    struct [[nodiscard]] SCall final
    {
        UINT8 opcode0;
        UINT8 opcode1;
        UINT32 dummy0;
        UINT8 dummy1;
        UINT8 dummy2;
        UINT64 address;

        SCall() = default;
        SCall(const SCall &) = delete;
        SCall & operator=(const SCall &) = delete;
    }__attribute__((packed)) sCall = {0xFF,0x15,0x02,0xEB,0x08,0x00};
    struct [[nodiscard]] SJcc final
    {
        UINT8 opcode;
        UINT8 dummy0;
        UINT8 dummy1;
        UINT8 dummy2;
        UINT32 dummy3;
        UINT64 address;

        SJcc() = default;
        SJcc(const SJcc &) = delete;
        SJcc & operator=(const SJcc &) = delete;
    } sJcc = {0x70,0x0E,0xFF,0x25,0x00,0x00};//__attribute__((packed));

    static constexpr const int g_iTrampolineMaxSize = g_iMemSlotSize - sizeof(SJmpAbs);
    UINT8 iInstBuf[16];
#else
    struct [[nodiscard]] SJmpRel final
    {
        UINT8 opcode;
        UINT32 operand;

        SJmpRel() = default;
        SJmpRel(const SJmpRel &) = delete;
        SJmpRel & operator=(const SJmpRel &) = delete;
    }__attribute__((packed)) sJmp = {0xE9,0x00};
    struct [[nodiscard]] SCall final
    {
        UINT8 opcode;
        UINT32 operand;

        SCall() = default;
        SCall(const SCall &) = delete;
        SCall & operator=(const SCall &) = delete;
    }__attribute__((packed)) sCall = {0xE8,0x00};
    struct [[nodiscard]] SJcc final
    {
        UINT8 opcode0;
        UINT8 opcode1;
        UINT32 operand;

        SJcc() = default;
        SJcc(const SJcc &) = delete;
        SJcc & operator=(const SJcc &) = delete;
    }__attribute__((packed)) sJcc = {0x0F,0x80,0x00};

    static constexpr const int g_iTrampolineMaxSize = g_iMemSlotSize;
#endif // _WIN64
    size_t szJmpDest = 0;
    UINT iNumOfIp = 0;
    UINT8 iOldPos = 0;
    UINT8 iNewPos = 0;
    bool bFinished = false;
    bool bPatchAbove = false;
    do
    {
        SHde sHde;
        const size_t iOldInst = pHookEntry->pTarget + iOldPos;
        UINT iCopySize = FMinHookDisasm(reinterpret_cast<const void*>(iOldInst), &sHde);
        if (sHde.flags & g_iFError)
            return false;
        const size_t iNewInst = pHookEntry->pTrampoline + iNewPos;
        void *pCopySrc = reinterpret_cast<void*>(iOldInst);
        if (iOldPos >= sizeof(SJmpRel))
        {
#ifdef _WIN64
            sJmp.address = iOldInst;
#else
            sJmp.operand = iOldInst - (iNewInst + sizeof(sJmp));
#endif
            pCopySrc = &sJmp;
            iCopySize = sizeof(sJmp);
            bFinished = true;
        }
#ifdef _WIN64
        else if ((sHde.modrm & 0xC7) == 0x05)
        {
            FBytesCopyEx(iInstBuf, reinterpret_cast<const void*>(iOldInst), iCopySize);
            pCopySrc = iInstBuf;
            *pointer_cast<UINT32*>(iInstBuf + sHde.len - ((sHde.flags & 0x3C) >> 2) - 4) =
                    static_cast<UINT32>((iOldInst + sHde.len + static_cast<UINT32>(static_cast<INT32>(sHde.disp.disp32))) - (iNewInst + sHde.len));
            if (sHde.opcode == 0xFF && sHde.modrm_reg == 4)
                bFinished = true;
        }
#endif
        else if (sHde.opcode == 0xE8)
        {
            const size_t szDest = iOldInst + sHde.len + static_cast<UINT32>(static_cast<INT32>(sHde.imm.imm32));
#ifdef _WIN64
            sCall.address = szDest;
#else
            sCall.operand = szDest - (iNewInst + sizeof(SCall));
#endif
            pCopySrc = &sCall;
            iCopySize = sizeof(SCall);
        }
        else if ((sHde.opcode & 0xFD) == 0xE9)
        {
            size_t szDest = iOldInst + sHde.len;
            if (sHde.opcode == 0xEB)
                szDest += static_cast<size_t>(static_cast<INT8>(sHde.imm.imm8));
            else
                szDest += static_cast<size_t>(static_cast<INT32>(sHde.imm.imm32));
            if (pHookEntry->pTarget <= szDest && szDest < pHookEntry->pTarget + sizeof(SJmpRel))
            {
                if (szJmpDest < szDest)
                    szJmpDest = szDest;
            }
            else
            {
#ifdef _WIN64
                sJmp.address = szDest;
#else
                sJmp.operand = szDest - (iNewInst + sizeof(sJmp));
#endif
                pCopySrc = &sJmp;
                iCopySize = sizeof(sJmp);
                bFinished = (iOldInst >= szJmpDest);
            }
        }
        else if ((sHde.opcode & 0xF0) == 0x70 || (sHde.opcode & 0xFC) == 0xE0 || (sHde.opcode2 & 0xF0) == 0x80)
        {
            size_t szDest = iOldInst + sHde.len;
            if ((sHde.opcode & 0xF0) == 0x70 || (sHde.opcode & 0xFC) == 0xE0)
                szDest += static_cast<size_t>(static_cast<INT8>(sHde.imm.imm8));
            else
                szDest += static_cast<size_t>(static_cast<INT32>(sHde.imm.imm32));
            if (pHookEntry->pTarget <= szDest && szDest < pHookEntry->pTarget + sizeof(SJmpRel))
            {
                if (szJmpDest < szDest)
                    szJmpDest = szDest;
            }
            else if ((sHde.opcode & 0xFC) == 0xE0)
                return false;
            else
            {
                const UINT8 iCond = (sHde.opcode != 0x0F ? sHde.opcode : sHde.opcode2) & 0x0F;
#ifdef _WIN64
                sJcc.opcode = 0x71 ^ iCond;
                sJcc.address = szDest;
#else
                sJcc.opcode1 = 0x80 | iCond;
                sJcc.operand = szDest - (iNewInst + sizeof(SJcc));
#endif
                pCopySrc = &sJcc;
                iCopySize = sizeof(SJcc);
            }
        }
        else if ((sHde.opcode & 0xFE) == 0xC2)
            bFinished = (iOldInst >= szJmpDest);
        if (iOldInst < szJmpDest && iCopySize != sHde.len)
            return false;
        if ((iNewPos + iCopySize) > g_iTrampolineMaxSize)
            return false;
        if (iNumOfIp >= g_iTargetBoundaries)
            return false;
        ++iNumOfIp;
        FBytesCopyEx(reinterpret_cast<BYTE*>(pHookEntry->pTrampoline) + iNewPos, pCopySrc, iCopySize);
        iNewPos = static_cast<UINT8>(iNewPos + iCopySize);
        iOldPos = static_cast<UINT8>(iOldPos + sHde.len);
    } while (!bFinished);
    if (iOldPos < sizeof(SJmpRel) && !FMinHookIsCodePadding(reinterpret_cast<const BYTE*>(pHookEntry->pTarget) + iOldPos, static_cast<UINT>(static_cast<UINT16>(sizeof(SJmpRel)) - iOldPos)))
    {
        if (iOldPos < sizeof(SJmpRelShort) && !FMinHookIsCodePadding(reinterpret_cast<const BYTE*>(pHookEntry->pTarget) + iOldPos, static_cast<UINT>(static_cast<UINT16>(sizeof(SJmpRelShort)) - iOldPos)))
            return false;
        if (!FMinHookIsExecutableAddress(pHookEntry->pTarget - sizeof(SJmpRel)))
            return false;
        if (!FMinHookIsCodePadding(reinterpret_cast<const BYTE*>(pHookEntry->pTarget) - sizeof(SJmpRel), sizeof(SJmpRel)))
            return false;
        bPatchAbove = true;
    }
#ifdef _WIN64
    sJmp.address = pHookEntry->pDetour;
    pHookEntry->pDetour = pHookEntry->pTrampoline + iNewPos;
    FBytesCopyEx(reinterpret_cast<void*>(pHookEntry->pDetour), &sJmp, sizeof(sJmp));
#endif
    if (pHookEntry->ppOriginal)
        *pHookEntry->ppOriginal = reinterpret_cast<void**>(pHookEntry->pTrampoline);
    BYTE *pbtPatchTarget = reinterpret_cast<BYTE*>(pHookEntry->pTarget);
    SIZE_T szPatchSize = sizeof(SJmpRel);
    if (bPatchAbove)
    {
        pbtPatchTarget -= sizeof(SJmpRel);
        szPatchSize += sizeof(SJmpRelShort);
    }
    void *pBaseAddr = pbtPatchTarget;
    ULONG iOldProtect;
    if (NT_SUCCESS(NtProtectVirtualMemory(FGetCurrentProcess(), &pBaseAddr, &szPatchSize, PAGE_EXECUTE_READWRITE, &iOldProtect)))
    {
        SJmpRel * const pJmpRel = pointer_cast<SJmpRel*>(pbtPatchTarget);
        pJmpRel->opcode = 0xE9;
        pJmpRel->operand = static_cast<UINT32>(reinterpret_cast<const BYTE*>(pHookEntry->pDetour) - (pbtPatchTarget + sizeof(SJmpRel)));
        if (bPatchAbove)
        {
            SJmpRelShort * const pJmpRelShort = reinterpret_cast<SJmpRelShort*>(pHookEntry->pTarget);
            pJmpRelShort->opcode = 0xEB;
            pJmpRelShort->operand = static_cast<UINT8>(-(sizeof(SJmpRelShort) + sizeof(SJmpRel)));
        }
        ULONG iUnused;
        if (NT_SUCCESS(NtProtectVirtualMemory(FGetCurrentProcess(), &pBaseAddr, &szPatchSize, iOldProtect, &iUnused)) &&
                NT_SUCCESS(NtFlushInstructionCache(FGetCurrentProcess(), pBaseAddr, szPatchSize)))
            return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static bool FMinHookNew(SHookEntry * const pHookEntry)
{
    if (FMinHookIsExecutableAddress(pHookEntry->pTarget) && FMinHookIsExecutableAddress(pHookEntry->pDetour))
        if (SMemoryBlock * const pMemBlock = FMinHookGetMemoryBlock(
            #ifdef _WIN64
                    pHookEntry
            #endif
                    ))
        {
            SMemoryBlock::SMemorySlot * const pMemSlot = pMemBlock->pMemSlotFree;
            pMemBlock->pMemSlotFree = pMemSlot->pMemSlotNext;
            pHookEntry->pTrampoline = reinterpret_cast<size_t>(pMemSlot);
            if (FMinHookCreateTrampoline(pHookEntry))
                return true;
        }
    return false;
}

//-------------------------------------------------------------------------------------------------
template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull (1, 2)))
#endif
static inline bool FMinHookCreate(T1 * const pTarget, T2 * const pDetour, T1 ** const ppOriginal = nullptr)
{
    SHookEntry sHookEntry = {
        reinterpret_cast<size_t>(pTarget),
        reinterpret_cast<size_t>(pDetour),
        reinterpret_cast<void**>(reinterpret_cast<size_t>(ppOriginal)),
        0};
    return FMinHookNew(&sHookEntry);
}
} // NPrivateMinHook

//-------------------------------------------------------------------------------------------------
#define FMinHook(target) (NPrivateMinHook::FMinHookCreate((target), (target##_Stub)))
#define FMinHookSaveOrigFunc(target) (NPrivateMinHook::FMinHookCreate((target), (target##_Stub), (&target##_Real)))

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FMinHookD(T1 * const pTarget, T2 * const pDetour)
{
    return NPrivateMinHook::FMinHookCreate(pTarget, pDetour);
}

#ifdef _WIN64
#define FHook(target) (NPrivateMinHook::FMinHookCreate((target), (target##_Stub)))
#define FHookSaveOrigFunc(target) (NPrivateMinHook::FMinHookCreate((target), (target##_Stub), (&target##_Real)))

template <typename T1, typename T2>
[[nodiscard]]
#ifdef __MINGW32__
__attribute__((nonnull))
#endif
static inline bool FHookD(T1 * const pTarget, T2 * const pDetour)
{
    return NPrivateMinHook::FMinHookCreate(pTarget, pDetour);
}
#endif // _WIN64

//-------------------------------------------------------------------------------------------------
static void FMinHooksFree()
{
    while (NPrivateMinHook::g_pMemBlocks)
    {
        NPrivateMinHook::SMemoryBlock * const pMemBlockIt = NPrivateMinHook::g_pMemBlocks->pMemBlockNext;
        void *pBaseAddr = NPrivateMinHook::g_pMemBlocks;
        SIZE_T szRegion = 0;
        NtFreeVirtualMemory(FGetCurrentProcess(), &pBaseAddr, &szRegion, MEM_RELEASE);
        NPrivateMinHook::g_pMemBlocks = pMemBlockIt;
    }
}

#endif // MINHOOK_H
