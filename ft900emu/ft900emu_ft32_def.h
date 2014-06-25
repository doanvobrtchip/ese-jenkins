/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_FT32_DEF_H
#define FT800EMU_FT32_DEF_H
// #include <...>

// System includes
#include "ft900emu_inttypes.h"

// Project includes

#define FT32_CONDITION_ZERO          0x01u
#define FT32_CONDITION_ZEROSHIFT     0
#define FT32_CONDITION_CARRY         0x02u
#define FT32_CONDITION_CARRYSHIFT    1
#define FT32_CONDITION_OVERFLOW      0x04u
#define FT32_CONDITION_OVERFLOWSHIFT 2
#define FT32_CONDITION_SIGN          0x08u
#define FT32_CONDITION_SIGNSHIFT     3
#define FT32_CONDITION_GTE           0x10u
#define FT32_CONDITION_GT            0x20u
#define FT32_CONDITION_A             0x50u

#define FT32_PATTERNMASK    0xF8000000u
#define FT32_PATTERN_TOC    0x00000000u
#define FT32_PATTERN_TOCI   0x08000000u
#define FT32_PATTERN_EXA    0x38000000u
#define FT32_PATTERN_ALUOP  0x40000000u
#define FT32_PATTERN_CMPOP  0x58000000u
#define FT32_PATTERN_LDK    0x60000000u
#define FT32_PATTERN_LPM    0x68000000u
#define FT32_PATTERN_POP    0x88000000u
#define FT32_PATTERN_PUSH   0x80000000u
#define FT32_PATTERN_UNLINK 0x98000000u
#define FT32_PATTERN_RETURN 0xA0000000u
#define FT32_PATTERN_LDI    0xA8000000u
#define FT32_PATTERN_STI    0xB0000000u
#define FT32_PATTERN_STA    0xB8000000u
#define FT32_PATTERN_LDA    0xC0000000u
#define FT32_PATTERN_LPMI   0xC8000000u
#define FT32_PATTERN_LINK   0x90000000u
#define FT32_PATTERN_EXI    0xE8000000u
#define FT32_PATTERN_FFUOP  0xF0000000u

#define FT32_DWMASK   0x06000000u
#define FT32_DWSHIFT  25
#define FT32_DW_8     0
#define FT32_DW_16    1
#define FT32_DW_32    2
#define FT32_DW(inst) ((inst & FT32_DWMASK) >> FT32_DWSHIFT)
static const uint32_t FT32_DW_MASK[3] = { 0xFFu, 0xFFFFu, 0xFFFFFFFFu };
static const uint32_t FT32_DW_SIZEU[3] = { 7, 15, 31 };
static const uint32_t FT32_DW_SIZE[3] = { 8, 16, 32 };
static const uint32_t FT32_DW_MASKU[3] = { 0x7Fu, 0x7FFFu, 0x7FFFFFFFu };
static const uint32_t FT32_DW_SIGNBIT[3] = { 0x80u, 0x8000u, 0x80000000u };

#define FT32_NSIGNED_SHIFT(siz)        ((sizeof(uint32_t) * 8) - siz)
#define FT32_NSIGNED(bits, siz, shift) ((((int32_t)bits) << (FT32_NSIGNED_SHIFT(siz) - shift)) >> FT32_NSIGNED_SHIFT(siz))
#define FT32_NSIGNED0(bits, siz)       FT32_NSIGNED(bits, siz, 0)

#define FT32_DW_SIGNED(bits, dw)       ((int32_t)(FT32_NSIGNED0(bits, FT32_DW_SIZE[dw]))) // Expand signed
#define FT32_DW_UNSIGNED(bits, dw)     (bits & FT32_DW_MASK[dw]) // Expand unsigned
#define FT32_DW_RES_SIGNED(bits, dw)   ((FT32_REG_SIGNBIT((uint32_t)bits) & FT32_DW_SIGNBIT[dw]) | ((uint32_t)bits & FT32_DW_MASKU[dw])) // Pack signed
#define FT32_DW_RES_UNSIGNED(bits, dw) (bits)

#define FT32_REG_SIGNBITMASK   0x80000000u
#define FT32_REG_SIGNBITSHIFT  31
#define FT32_REG_SIGNBIT(bits) ((int32_t)bits >> FT32_REG_SIGNBITSHIFT)

#define FT32_RDMASK   0x01F00000u
#define FT32_RDSHIFT  20
#define FT32_RD(inst) ((inst & FT32_RDMASK) >> FT32_RDSHIFT)

#define FT32_R1MASK   0x000F8000u
#define FT32_R1SHIFT  15
#define FT32_R1(inst) ((inst & FT32_R1MASK) >> FT32_R1SHIFT)

#define FT32_R2MASK   0x000001F0u
#define FT32_R2SHIFT  4
#define FT32_R2(inst) ((inst & FT32_R2MASK) >> FT32_R2SHIFT)

#define FT32_RIMMMASK             0x00004000u
#define FT32_RIMM(inst)           ((inst & FT32_RIMMMASK) == FT32_RIMMMASK)
#define FT32_RIMMSHIFT            4
#define FT32_RIMM_IMMEDIATESIZE   10
#define FT32_RIMM_IMMEDIATE(inst) FT32_NSIGNED(inst, FT32_RIMM_IMMEDIATESIZE, FT32_RIMMSHIFT)
#define FT32_RIMM_REGMASK         0x000001F0u
#define FT32_RIMM_REG(inst)       ((inst & FT32_RIMM_REGMASK) >> FT32_RIMMSHIFT)

#define FT32_AAMASK   0x0001FFFFu
#define FT32_AA(inst) (inst & FT32_AAMASK)

#define FT32_PAMASK   0x0003FFFFu
#define FT32_PA(inst) (inst & FT32_PAMASK)

#define FT32_TOC_CALL                   0x00040000u
#define FT32_TOC_INDIRECT               FT32_PATTERN_TOCI
#define FT32_TOC_CRMASK                 0x00300000u
#define FT32_TOC_CRSHIFT                20
#define FT32_TOC_CRREG                  28
#define FT32_TOC_CR_REGISTER(inst)      (((inst & FT32_TOC_CRMASK) >> FT32_TOC_CRSHIFT) + FT32_TOC_CRREG)
#define FT32_TOC_CR_UNCONDITIONAL(inst) ((inst & FT32_TOC_CRMASK) == FT32_TOC_CRMASK)
#define FT32_TOC_CBMASK                 0x07C00000u
#define FT32_TOC_CBSHIFT                22
#define FT32_TOC_CB(inst)               ((inst & FT32_TOC_CBMASK) >> FT32_TOC_CBSHIFT)
#define FT32_TOC_CVMASK                 0x00080000u
#define FT32_TOC_CVSHIFT                19
#define FT32_TOC_CV(inst)               ((inst & FT32_TOC_CVMASK) >> FT32_TOC_CVSHIFT)
#define FT32_TOC_PAMASK                 0x0003FFFFu
#define FT32_TOC_PA(inst)               (inst & FT32_TOC_PAMASK)

#define FT32_RETURN_INTMASK 0x04000000u

#define FT32_PM_CUR_MASK 0xFFFFu

#define FT32_ALU_ADD   0x0u
#define FT32_ALU_ROR   0x1u
#define FT32_ALU_SUB   0x2u
#define FT32_ALU_LDL   0x3u
#define FT32_ALU_AND   0x4u
#define FT32_ALU_OR    0x5u
#define FT32_ALU_XOR   0x6u
#define FT32_ALU_XNOR  0x7u
#define FT32_ALU_ASHL  0x8u
#define FT32_ALU_LSHR  0x9u
#define FT32_ALU_ASHR  0xAu
#define FT32_ALU_BINS  0xBu
#define FT32_ALU_BEXTS 0xCu
#define FT32_ALU_BEXTU 0xDu
#define FT32_ALU_FLIP  0xEu

#define FT32_FFU_UDIV       0x0u
#define FT32_FFU_UMOD       0x1u
#define FT32_FFU_DIV        0x2u
#define FT32_FFU_MOD        0x3u
#define FT32_FFU_STRCMP     0x4u
#define FT32_FFU_MEMCPY     0x5u
#define FT32_FFU_STRLEN     0x6u
#define FT32_FFU_MEMSET     0x7u
#define FT32_FFU_MUL        0x8u
#define FT32_FFU_MULUH      0x9u
#define FT32_FFU_STPCPY     0xAu
//                          0xBu
#define FT32_FFU_STREAMIN   0xCu
#define FT32_FFU_STREAMINI  0xDu
#define FT32_FFU_STREAMOUT  0xEu
#define FT32_FFU_STREAMOUTI 0xFu

#define FT32_ALMASK   0x0Fu
#define FT32_AL(inst) (inst & FT32_ALMASK)

#define FT32_K8MASK    0x000000FFu
#define FT32_K8SIZE    8
#define FT32_K8(inst)  FT32_NSIGNED0(inst, FT32_K8SIZE)
#define FT32_K16MASK   0x0000FFFFu
#define FT32_K16(inst) (inst & FT32_K16MASK)
#define FT32_K20MASK   0x000FFFFFu
#define FT32_K20SIZE   20
#define FT32_K20(inst) FT32_NSIGNED0(inst, FT32_K20SIZE)

#define FT32_CMPMASK   0x18000000u
#define FT32_CMP(inst) (inst & FT32_CMPMASK)

#define FT32_BINS_WIDTH(rimmv)    (((rimmv >> 5) & 0x0Fu) ? ((rimmv >> 5) & 0x0Fu) : 16)
#define FT32_BINS_POSITION(rimmv) (rimmv & 0x1Fu)

#define FT32_EFLAGS_CARRY                0x00000001u
#define FT32_EFLAGS_CARRYSHIFT           0
#define FT32_EFLAGS_ZERO                 0x00000040u
#define FT32_EFLAGS_ZEROSHIFT            6
#define FT32_EFLAGS_SIGN                 0x00000080u
#define FT32_EFLAGS_SIGNSHIFT            7
#define FT32_EFLAGS_OVERFLOW             0x00000800u
#define FT32_EFLAGS_OVERFLOWSHIFT        11
#define FT32_EFLAGS_TO_CONDITION(eflags) ( \
	((eflags & FT32_EFLAGS_CARRY) << (FT32_CONDITION_CARRYSHIFT - FT32_EFLAGS_CARRYSHIFT)) \
	| ((eflags & FT32_EFLAGS_ZERO) >> (FT32_EFLAGS_ZEROSHIFT - FT32_CONDITION_ZEROSHIFT)) \
	| ((eflags & FT32_EFLAGS_SIGN) >> (FT32_EFLAGS_SIGNSHIFT - FT32_CONDITION_SIGNSHIFT)) \
	| ((eflags & FT32_EFLAGS_OVERFLOW) >> (FT32_EFLAGS_OVERFLOWSHIFT - FT32_CONDITION_OVERFLOWSHIFT)) \
)

#define FT32_REG_CMP        30
#define FT32_REG_STACK      31
#define FT32_REG_STACK_MASK 0xFFFF;

#endif /* #ifndef FT900EMU_FT32_DEF_H */

/* end of file */
