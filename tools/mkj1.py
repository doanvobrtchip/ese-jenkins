import array

# if (state == 0x1BA6) printf("COMMAND [%%03x] %%08x\n", MemoryClass::coprocessorReadU32(REG_CMD_READ), acc);

c_func = r"""/**
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

void CoprocessorClass::begin(const char *romFilePath)
{
    state = 0;
    pcV = cV;
    pdV = dV;
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
        switch (state) {
%(handle)s
        default:
            assert(0);
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

/* end of file */
"""


pgm = array.array('H', open("main.binle").read())[:7608]

pop = {}
for i in pgm:
    if i in pop:
        pop[i] += 1
    else:
        pop[i] = 1
# print pop

iN = "pdV"
N = "*pdV"
pN = "*++pdV"
Nm = "*pdV--"

class Destlist:
    def __init__(self):
        self.d = {}
    def get(self, ip):
        if not ip in self.d:
            self.d[ip] = len(self.d)
        return self.d[ip]

destinations = Destlist()
handle = []
allcode = {}
for ip,insn in enumerate(pgm):
    if ip == 0x1bb9:    # Patch to balance stack in cmd1
        insn |= 3;
    if ip == 0x0378:
        act = ["assert(0);"]
    elif insn & 0x8000:
        act = ["%s = acc;" % pN,
               "acc = %d;" % (insn & 0x7fff)]
    else:
        op = (insn >> 13)
        if insn == 0x40f7:
            act = [
                "{",
                "    uint64_t x = *(uint64_t *)(--pdV);",
                "    *pdV = x % acc;",
                "    acc = x / acc;",
                "}"]
        elif op == 0:
            # act = ["state = %d; break;" % (insn & 8191)]
            act = ["goto S_%d;" % destinations.get(insn & 8191)]
        elif op == 1:
            act = ["Xreg = acc;",
                   "acc = %s;" % Nm,
                   "if (Xreg == 0)",
                   "    goto S_%d;" % destinations.get(insn & 8191)]
        elif op == 2:
            act = ["*++pcV = %d;" % ((ip + 1) * 2)]
            if ip == 0x980:
                # 0x1090f8 is the location in coprocessor private RAM where the read pointer is cached.
                act.append("starve = MemoryClass::coprocessorReadU32(REG_CMD_WRITE) == MemoryClass::coprocessorReadU32(0x1090f8);")
                act.append("if (starve) { state = %d; break;}" % (insn & 8191))
            act.append("goto S_%d;" % destinations.get(insn & 8191))
        elif op == 3:
            insn_6_4 = 7 & (insn >> 4)
            aluop =  ((insn >> 8) & 31)
            if True:
                isret = (insn & (1 << 7))
                act = []
                if isret:
                    act.append("state = *pcV >> 1;")
                hop = [None,
                       "%s = acc;" % N,
                       "*pcV = acc;",
                       "(*pcV)++;",
                       "MemoryClass::coprocessorWriteU32(acc, %s);" % N,
                       "MemoryClass::coprocessorWriteU16(acc, %s);" % N,
                       "MemoryClass::coprocessorWriteU8(acc, %s);" % N,
                       None][insn_6_4]
                alu = [
                    "TT = acc;",
                    "TT = NN;",
                    "TT = acc + NN;",
                    "TT = acc & NN;",
                    "TT = acc | NN;",
                    "TT = acc ^ NN;",
                    "TT = ~acc;",
                    "TT = -(acc == NN);",
                    "TT = -((int32_t)NN < (int32_t)acc);",
                    "TT = ((int32_t)(NN)) >> acc;",
                    "TT = acc - 1;",
                    "TT = *pcV;",
                    "TT = MemoryClass::coprocessorReadU32(acc & ~3);",
                    "TT = acc * NN;",
                    "TT = (NN << 15) | (acc & 0x7fff);",
                    "TT = -(NN < acc);",
                    "",
                    "TT = NN << acc;",
                    "TT = MemoryClass::coprocessorReadU8(acc);",
                    "TT = MemoryClass::coprocessorReadU16(acc & ~1);",
                    "TT = PRODUCT64(acc, NN) >> 32;",
                    "TT = PRODUCT64(acc, NN) >> 16;",
                    "TT = acc == *pcV;",
                    "TT = NN - acc;",
                    "TT = acc + 1;",
                    "TT = (int16_t)MemoryClass::coprocessorReadU16(acc);",
                    "{ uint32_t sum32 = acc + NN; TT = (sum32 & 0x80000000) ? 0 : ((sum32 & 0x7fffff00) ? 255 : (sum32 & 0xff)); }",
                    "TT = acc | 0x109000;",
                    "TT = acc + 2;",
                    "TT = acc << 1;",
                    "TT = acc + 4;",
                    ""][aluop]
                alu = alu.replace("NN", N)
                if insn_6_4 in (1,2,4,5,6):
                    alu = alu.replace("TT", "Xreg")
                else:
                    alu = alu.replace("TT", "acc")
                if aluop != 0:
                    act.append(alu)

                if insn_6_4 in (4,5,6):
                    act.append(hop)

                sx = [ None, "++", None, "--" ]
                if insn & 3:
                    act.append("%s%s;" % (iN, sx[insn & 3]))
                if (insn >> 2) & 3:
                    act.append("pcV%s;" % sx[(insn >> 2) & 3])
                if ip == 0xafa:
                    act.append("swapped |= singleFrame;")
                    act.append("if (swapped) { state = %d; break;}" % (ip + 1))

                if insn_6_4 in (1,2,3):
                    act.append(hop)

                if aluop != 0 and insn_6_4 in (1,2,4,5,6):
                    act.append("acc = Xreg;");
                if isret:
                    act.append("break;")
    allcode[ip] = act

for ip,insn in enumerate(pgm):
    if not ip in range(0x00e2, 0x011b):
        act = allcode[ip]
        # act.append("state = %d; break;" % (ip + 1));
        if ip in destinations.d:
            handle.append("S_%d:" % destinations.d[ip])
        handle.append("case %d: " % ip)
        # handle.append(r'printf("%d\n");' % ip);
        handle += ["    " + s for s in act]
handle = "\n".join([" "*8 + s for s in handle])

open("../ft800emu/ft800emu_coprocessor.cpp", "w").write(c_func % locals())

