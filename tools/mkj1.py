import array

c_func = r"""
/**
 * graphics coprocessor
 * $Id$
 * \file ft800emu_coprocessor.cpp
 * \brief graphics coprocessor
 * \date 2013-08-03 02:10GMT
*/

#include "ft800emu_coprocessor.h"

// System includes
#include <stdio.h>
#include <string.h>

// Project includes
#include "ft800emu_memory.h"
#include "ft800emu_system.h"
#include "vc.h"

// using namespace ...;

namespace FT800EMU {

#define FT800EMU_COPROCESSOR_ROM_SIZE 8192

CoprocessorClass Coprocessor;

static bool s_Running;

static const int sx[4] = { 0, 1, -2, -1 }; /* 2-bit sign extension */
static const uint16_t pgm_rom[FT800EMU_COPROCESSOR_ROM_SIZE] = {
#include "crom.h"
};
static uint16_t pgm[FT800EMU_COPROCESSOR_ROM_SIZE];

void CoprocessorClass::begin(const char *romFilePath)
{
    if (romFilePath)
    {
        FILE *f;
        f = fopen(romFilePath, "rb");
        if (!f) printf("Failed to open coprocessor ROM file\n");
        else
        {
            size_t s = fread(pgm, 1, FT800EMU_COPROCESSOR_ROM_SIZE, f);
            if (s != FT800EMU_COPROCESSOR_ROM_SIZE) printf("Incomplete coprocessor ROM file\n");
            else printf("Loaded coprocessor ROM file\n");
            if (fclose(f)) printf("Error closing coprocessor ROM file\n");
        } 
    }
    else
    {
        memcpy(pgm, pgm_rom, sizeof(pgm_rom));
    }

    pc = 0;
    dsp = 0;
    rsp = 0;
    pcV = r;
}

#define PRODUCT64(a, b) ((int64_t)(int32_t)(a) * (int64_t)(int32_t)(b))

template <bool singleFrame>
void CoprocessorClass::execute()
{
    if (!singleFrame)
        s_Running = true;

    int Xreg;

    int swapped = 0;
    int starve = 0;
    do {
        switch (pc) {
%(handle)s
        }
    } while (singleFrame ? (!swapped && !starve) : s_Running);
}

void CoprocessorClass::executeManual()
{
    execute<true>();
}

void CoprocessorClass::executeEmulator()
{
    execute<false>();
}

void CoprocessorClass::stopEmulator()
{
    s_Running = false;
}

void CoprocessorClass::end()
{

}

} /* namespace GDEMU */

/* end of file %(pc)s */
"""


pgm = array.array('H', open("main.binle").read())[:]
for pc,op in enumerate(pgm):
    print pc, hex(op)

handle = []
for ip,insn in enumerate(pgm):
    if insn & 0x8000:
        act = ["push(%d);" % (insn & 0x7fff)]
    else:
        op = (insn >> 13)
        if op == 0:
            act = ["pc = %d; break;" % (insn & 8191)]
        elif op == 1:
            act = ["if (pop() == 0) { pc = %d; break;}" % (insn & 8191)]
        elif op == 2:
            act = ["r[++rsp] = %d;" % ((ip + 1) * 2),
                   "pc = %d; break;" % (insn & 8191)]
        elif op == 3:
            insn_6_4 = 7 & (insn >> 4)
            aluop =  ((insn >> 8) & 31)
            if True:
                isret = (insn & (1 << 7))
                act = []
                if isret:
                    act.append("pc = r[rsp] >> 1;")
                hop = [None,
                       "d[dsp] = t;",
                       "r[rsp] = t;",
                       "r[rsp]++;",
                       "MemoryClass::coprocessorWriteU32(t, d[dsp]); swapped = singleFrame && (t == REG_DLSWAP);",
                       "MemoryClass::coprocessorWriteU16(t, d[dsp]);",
                       "MemoryClass::coprocessorWriteU8(t, d[dsp]);",
                       None][insn_6_4]
                alu = [
                    "TT = t;",
                    "TT = NN;",
                    "TT = t + NN;",
                    "TT = t & NN;",
                    "TT = t | NN;",
                    "TT = t ^ NN;",
                    "TT = ~t;",
                    "TT = -(t == NN);",
                    "TT = -((int32_t)NN < (int32_t)t);",
                    "TT = NN >> t;",
                    "TT = t - 1;",
                    "TT = r[rsp];",
                    "TT = MemoryClass::coprocessorReadU32(t & ~3);",
                    "TT = PRODUCT64(t, NN);",
                    "TT = (NN << 15) | (t & 0x7fff);",
                    "TT = -(NN < t);",
                    "",
                    "TT = NN << t;",
                    "TT = MemoryClass::coprocessorReadU8(t);",
                    "TT = MemoryClass::coprocessorReadU16(t & ~1);",
                    "TT = PRODUCT64(t, NN) >> 32;",
                    "TT = PRODUCT64(t, NN) >> 16;",
                    "TT = t == r[rsp];",
                    "TT = NN - t;",
                    "TT = t + 1;",
                    "TT = (int16_t)MemoryClass::coprocessorReadU16(t);",
                    "{ uint32_t sum32 = t + NN; TT = (sum32 & 0x80000000) ? 0 : ((sum32 & 0x7fffff00) ? 255 : (sum32 & 0xff)); }",
                    "TT = (0x00109 << 12) | (t & 0xfff);",
                    "TT = t + 2;",
                    "TT = t << 1;",
                    "TT = t + 4;",
                    ""][aluop]
                alu = alu.replace("NN", "d[dsp]")
                if insn_6_4 in (1,2,3,4,5,6):
                    alu = alu.replace("TT", "Xreg")
                else:
                    alu = alu.replace("TT", "t")
                act.append(alu)

                if insn_6_4 in (4,5,6):
                    act.append(hop)

                sx = [ None, "++", None, "--" ]
                s1 = [ "", "+1", None, "-1" ]
                # act.append("dsp = 31 & (dsp %s);" % s1[insn & 3]);
                if insn & 3:
                    act.append("dsp%s;" % sx[insn & 3])
                if (insn >> 2) & 3:
                    act.append("rsp%s;" % sx[(insn >> 2) & 3])

                if insn_6_4 in (1,2,3):
                    act.append(hop)

                if insn_6_4 in (1,2,3,4,5,6):
                    act.append("t = Xreg;");
                if isret:
                    act.append("break;")

    handle.append("case %d:" % ip)
    handle += ["    " + s for s in act]
handle = "\n".join([" "*8 + s for s in handle])

open("../ft800emu/ft800emu_coprocessor.cpp", "w").write(c_func % locals())

