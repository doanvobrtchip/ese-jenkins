import array
import pickle
import struct
import binascii
import functools
from collections import defaultdict

from base import filesearch

bl4k = array.array('B', [0xff for i in xrange(4096)])

class SpiFlash:
    def sfdp(self, a):
        return self.sfdp_data[a & 0xff]

    def __init__(self, v, verbose = 0, image = None):
        self.verbose = verbose
        self.shifter = (0, 0)
        self.prev_spimo = 0
        self.prev_clk = 0
        self.default_handler = self.handle_command
        self.handler = self.default_handler
        self.stats = defaultdict(lambda: 0)

        if image is not None:
            self.store = array.array('B', open(image, "rb").read())
        else:
            STDFLASH = open(filesearch("build/flash/stdflash.bin")).read().ljust(self.size << 17, chr(0xff))
            self.store = array.array('B', STDFLASH)

        self.wren = 0
        self.status = 0
        self.qspi = False
        self.lag = [0,0,0]
        self.shiftout = 0
        
        self.performance = 0

        self.cmdtab = {
            0x02: self.PP,
            0x01: self.WRSR,
            0x03: self.READ,
            0x05: self.RDSR,
            0x06: self.WREN,
            0x5a: self.SFDP,
            0x9f: self.RDID,
            0xc7: self.CE,
            0xff: self.NOOP,
            0x66: self.NOOP,
            0x99: self.RESET,
        }
        self.devinit()

        self.parse_sfdp()

    def parse_sfdp(self):
        o = self.sfdp_data[0xc]
        erase_ops = array.array('B', self.sfdp_data[o + 28: o + 28 + 8])
        for sz,insn in zip(erase_ops[::2], erase_ops[1::2]):
            if insn not in(0x00, 0xff):
                # print self, "erase: instruction %02x erases %d bytes" % (insn, 2 ** sz)
                assert insn not in self.cmdtab
                self.cmdtab[insn] = functools.partial(self.ERASE_PART, sz)

    def ERASE_PART(self, size, a):
        if self.bytecount == 0:
            self.addr = 0
            return 0xff
        elif self.bytecount in range(1, self.addressbytes + 1):
            self.addr = (self.addr << 8) | a
        if self.bytecount == self.addressbytes:
            # print 'ERASE_PART', hex(self.addr), self.addressbytes
            self.wipe(self.addr, self.addr + 2 ** size)
        return 0xff

    def devinit(self):
        pass

    def setblob(self, b = None):
        if b is not None:
            blobname = b
        else:
            blobname = self.blobname
        self.store[0:4096] = array.array('B', open(filesearch("build/flash/%s.blob" % blobname)).read())

    def erase(self):
        self.store = array.array('B', [0xff] * (8 * 1024 * 1024))

    def clk(self, v):
        rising = v.spimo & ~self.prev_spimo
        return v.spim_clken or (1 & (rising >> 5))

    def update(self, v):
        cs = 1 & (v.spimo >> 4)
        if self.verbose > 2:
            if v.spim_clken or cs:
                print "clk", "qspi", self.qspi, "clk", v.spim_clken, "cs", cs, "spim_dir", v.spim_dir, "spimo", v.spimo
        if cs:
            self.shifter = (0, 0)
            self.shiftout = self.handler(None)
            if not self.performance:
                self.handler = self.default_handler
                # self.qspi = False
            else:
                self.handler = self.READP
                self.bytecount = 0
            return
        # assert v.spim_clken == (1 & (v.spimo >> 5))
        if self.clk(v):
            (c,a) = self.shifter
            if not self.qspi:
                self.shifter = (c + 1, (a << 1) | (v.spimo & v.spim_dir & 1))
            else:
                self.shifter = (c + 4, (a << 4) | (v.spimo & v.spim_dir & 0xf))
            if self.shifter[0] == 8:
                # print "MOSI %02x" % self.shifter[1]
                if self.verbose > 1:
                    print "handle byte %02x" % self.shifter[1]
                self.shiftout = self.handler(self.shifter[1])
                self.bytecount += 1
                self.shifter = (0, 0)

        mask = 0xf ^ v.spim_dir
        v.spimi = mask & self.lag[0]
        if len(self.lag) == 2:
            self.lag = [self.lag[1], self.lag[1]]
        elif len(self.lag) > 2:
            self.lag.pop(0)
        if self.clk(v):
            if not self.qspi:
                self.lag.append((1 & (self.shiftout >> 7)) * 2)
                self.shiftout <<= 1
            else:
                self.lag.append(0xf & (self.shiftout >> 4))
                self.shiftout <<= 4
        self.prev_spimo = v.spimo

    def handle_command(self, a):
        if a is None:
            return 0xff
        self.bytecount = 0
        self.handler = self.cmdtab.get(a, self.unhandled)
        if self.verbose > 0:
            print '(q=%d wren=%d) handler for %02x is %r' % (self.qspi, self.wren, a, self.handler)
        return self.handler(a)

    def unhandled(self, a):
        assert False, "Unhandled command %02x" % a

    def RDID(self, a):
        r = self.rdid_data[self.bytecount % 3]
        return r

    def RDSR(self, a):
        return self.status | (self.wren << 1)

    def WRSR(self, a):
        if self.bytecount == 0:
            self.is_writable()
        elif a is not None:
            self.status = a
        return 0x00

    def WREN(self, a):
        self.wren = 1
        return 0xff

    def is_writable(self):
        assert self.wren
        self.wren = 0

    def wipe(self, start, end):
        # print 'wipe', start, end
        assert (start & 4095) == 0
        assert (end & 4095) == 0
        for i in range(start, end, 4096):
            self.store[i:i+4096] = bl4k
        self.stats['erased'] += (end - start)

    def CE(self, a):
        if a is not None:
            self.is_writable()
        self.wipe(0, len(self.store))
        return 0xff

    def PP(self, a):
        if self.bytecount == 0:
            self.is_writable()
            self.acc = []
        else:
            self.acc.append(a)
        a = self.acc
        ab = self.addressbytes
        if len(a) == ab + 256:
            addr = (a[0] << 16) + (a[1] << 8) + a[2]
            if ab == 4:
                addr = (addr << 8) + a[3]
            d = array.array('B', a[ab:])
            for sa, di in enumerate(d, addr):
                self.store[sa] &= di
            self.stats['written'] += 256
        return 0xff

    def SFDP(self, a):
        if self.bytecount == 0:
            self.addr = 0
            return 0xff
        elif self.bytecount in (1, 2, 3):
            self.addr = (self.addr << 8) | a
        elif self.bytecount == 4:
            pass
        else:
            self.addr += 1
        return self.sfdp(self.addr)

    def READ(self, a):
        if self.bytecount == 0:
            self.addr = 0
            return 0xff
        elif self.bytecount in (1, 2, 3):
            self.addr = (self.addr << 8) | a
        else:
            self.addr += 1
        # print "fetch %x" % self.addr
        return self.store[self.addr]

    def iscontinue(self, mode8):
        assert 0 <= mode8 < 256
        return mode8 == 0xa5

    dummybytes = 2
    addressbytes = 3

    def READ4(self, a):
        if a is None:
            self.qspi = self.performance    # Maybe exit QSPI
        ab = self.addressbytes
        if self.bytecount == 0:
            self.addr = 0
            self.qspi = 1
            return 0xff
        elif self.bytecount in range(1, ab + 1):
            self.addr = (self.addr << 8) | a
        elif self.bytecount in (ab + 1,):        # PE
            self.performance = self.iscontinue(a)
        elif self.bytecount in range(ab + 2, ab + 2 + self.dummybytes):
            pass
        else:
            self.addr += 1
        if self.verbose:
            print "fetch [%x] = %02x" % (self.addr, self.store[self.addr])
        mask = (131072 * self.size) - 1
        return self.store[self.addr & mask]

    def READP(self, a):
        self.check_quad()
        assert self.qspi != 0
        ab = self.addressbytes
        if a is None:
            self.addr = 0
            self.qspi = self.performance    # Maybe exit QSPI
            return 0xff
        if self.bytecount in range(ab):
            self.addr = (self.addr << 8) | a
        elif self.bytecount in (ab,):        # PE
            self.performance = self.iscontinue(a)
        elif self.bytecount in range(ab + 1, ab + 1 + self.dummybytes):
            pass
        else:
            self.addr += 1
        mask = (131072 * self.size) - 1
        if self.verbose:
            print "fetch [%x] = %02x" % (self.addr, self.store[self.addr & mask])
        return self.store[self.addr & mask]

    def NOOP(self, a):
        return 0xff

    def RESET(self, a):
        return 0xff

    def check_quad(self):
        return True

class MX25(SpiFlash):
    rdid_data = [0xc2, 0x20, 0x17]
    size = 64
    blobname = "mx25l"
    sfdp_data = [83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255, 194, 0, 1, 4, 96, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 184, 255, 255, 255, 255, 3, 68, 235, 0, 255, 0, 255, 4, 187, 238, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 12, 32, 15, 82, 16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 54, 0, 39, 244, 79, 255, 255, 217, 200, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255, 194, 0, 1, 4, 96, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 184, 255, 255, 255, 255, 3, 68, 235, 0, 255, 0, 255, 4, 187, 238, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0, 255, 12, 32, 15, 82, 16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 54, 0, 39, 244, 79, 255, 255, 217, 200, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
    def devinit(self):
        self.cmdtab[0xeb] = self.READ4
    def check_quad(self):
        assert self.status & 64

class MX25L256(SpiFlash):
    rdid_data = [0xc2, 0x20, 0x19]
    size = 256
    blobname = "mx25l"
    sfdp_data = [83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255, 194, 0, 1, 4, 96, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 243, 255, 255, 255, 255, 15, 68, 235, 8, 107, 8, 59, 4, 187, 254, 255, 255, 255, 255, 255, 0, 255, 255, 255, 68, 235, 12, 32, 15, 82, 16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 54, 0, 39, 157, 249, 192, 100, 133, 203, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 194, 245, 8, 11, 5, 2, 5, 7, 0, 0, 15, 24, 44, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
    def EN4B(self, a):
        self.addressbytes = 4
        return 0xff

    def devinit(self):
        self.cmdtab[0xeb] = self.READ4
        self.cmdtab[0xb7] = self.EN4B

    def RESET(self, a):
        self.addressbytes = 3
        return 0xff

class W25Q(SpiFlash):
    rdid_data = [0xef, 0x40, 0x17]
    size = 64
    blobname = "w25q"
    sfdp_data = [83, 70, 68, 80, 0, 1, 0, 255, 0, 0, 1, 9, 128, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 241, 255, 255, 255, 255, 3, 68, 235, 8, 107, 8, 59, 66, 187, 254, 255, 255, 255, 255, 255, 0, 0, 255, 255, 33, 235, 12, 32, 15, 82, 16, 216, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
    dummybytes = 0
    def devinit(self):
        self.cmdtab[0xe3] = self.READ4

class S25FL1(SpiFlash):
    rdid_data = [0x01, 0x40, 0x17]
    size = 64
    blobname = "s25fl1"
    sfdp_data = [83, 70, 68, 80, 6, 1, 3, 255, 0, 0, 1, 9, 128, 0, 0, 255, 239, 0, 1, 4, 128, 0, 0, 255, 0, 6, 1, 16, 128, 0, 0, 255, 1, 1, 1, 0, 0, 0, 0, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 241, 255, 255, 255, 255, 3, 68, 235, 8, 107, 8, 59, 128, 187, 238, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 12, 32, 16, 216, 0, 255, 0, 255, 66, 242, 253, 255, 129, 106, 20, 207, 204, 99, 22, 51, 122, 117, 122, 117, 247, 162, 213, 92, 0, 246, 89, 255, 232, 16, 192, 128, 21, 13, 26, 1, 7, 7, 0, 36, 14, 65, 52, 48, 6, 17, 18, 20, 17, 7, 3, 6, 21, 165, 160, 200, 8, 3, 7, 8, 6, 5, 0, 1, 182, 15, 0, 23, 9, 24, 21, 255, 255, 255, 255, 255, 255, 255, 255, 15, 3, 33, 1, 255, 255, 255, 255, 255, 52, 4, 225, 128, 227, 8, 180, 6]
    def devinit(self):
        self.cmdtab[0xeb] = self.READ4
    def iscontinue(self, a):
        return (a & 0x30) == 0x20

class N25Q(SpiFlash):
    rdid_data = [0x20, 0xba, 0x18]
    size = 128
    blobname = "n25q"
    sfdp_data = [83, 70, 68, 80, 5, 1, 1, 255, 0, 5, 1, 16, 48, 0, 0, 255, 3, 0, 1, 2, 0, 1, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 241, 255, 255, 255, 255, 7, 41, 235, 39, 107, 39, 59, 39, 187, 255, 255, 255, 255, 255, 255, 39, 187, 255, 255, 41, 235, 12, 32, 16, 216, 0, 0, 0, 0, 53, 138, 1, 0, 130, 163, 3, 203, 172, 193, 4, 46, 122, 117, 122, 117, 251, 0, 0, 128, 8, 15, 130, 255, 129, 61, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]
    dummybytes = 4
    def devinit(self):
        self.cmdtab[0xeb] = self.READ4
        self.cmdtab[0x61] = self.WEVCR
        self.cmdtab[0x81] = self.WVCR
        self.cmdtab[0x70] = self.RDFSR

    def iscontinue(self, a):
        return (a & 0x10) == 0

    def WEVCR(self, a):
        """ WRITE ENHANCED VOLATILE CONFIGURATION REGISTER """
        if self.bytecount == 0:
            self.is_writable()
        if self.bytecount == 1:
            self.evcr = a
        if a == None:
            self.qspi = not (self.evcr >> 7)
        return 0xff

    def WVCR(self, a):
        """ WRITE VOLATILE CONFIGURATION REGISTER """
        if self.bytecount == 0:
            self.is_writable()
        if self.bytecount == 1:
            self.vcr = a
        return 0xff

    def RDSR(self, a):
        assert 0, "N25Q series should not use regular status reads"

    def RDFSR(self, a):
        return 0x80 # ready

    def DIE_ERASE(self, a):
        self.wipe(0, len(self.store))
        return 0xff

class N25Q032(N25Q):
    rdid_data = [0x20, 0xba, 0x16]
    size = 32
    blobname = "n25q"
    sfdp_data = ([83, 70, 68, 80, 0, 1, 0, 255, 0, 0, 1, 9, 48, 0, 0, 255] +
                 [0] * 32 +
                 [229, 32, 251, 255, 255, 255, 255,
                 # 31,  # this is what their model has issue -- XXX #242
                    1,  # this is what is in the datasheet
                 41, 235, 39, 107, 39, 59, 39, 187, 255, 255, 255, 255, 255, 255, 39, 187, 255, 255, 41, 235, 12, 32, 16, 216, 0, 0, 0, 0] +
                 [0] * 256)


class N25Q512(N25Q):
    rdid_data = [0x20, 0xba, 0x20]
    size = 512
    blobname = "n25q"
    sfdp_data = [83, 70, 68, 80, 0, 1, 0, 255, 0, 0, 1, 9, 48, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 251, 255, 255, 255, 255, 31, 41, 235, 39, 107, 39, 59, 39, 187, 255, 255, 255, 255, 255, 255, 39, 187, 255, 255, 41, 235, 12, 32, 16, 216, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255]

    def devinit(self):
        N25Q.devinit(self)
        self.cmdtab[0xb7] = self.EN4B
        self.cmdtab[0xc4] = self.DIE_ERASE
        del self.cmdtab[0xc7]

    def EN4B(self, a):
        self.addressbytes = 4
        return 0xff

    def RESET(self, a):
        self.addressbytes = 3
        return 0xff

class GD25Q64C(SpiFlash):
    rdid_data = [200, 64, 23]
    size = 64
    blobname = "unified"
    sfdp_data = \
[83, 70, 68, 80, 0, 1, 1, 255, 0, 0, 1, 9, 48, 0, 0, 255, 200, 0, 1, 3, 96, 0, 0
, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 ,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 229, 32, 241, 255, 255, 255, 255,
 3, 68, 235, 8, 107, 8, 59, 66, 187, 238, 255, 255, 255, 255, 255, 0, 255, 255, 
255, 0, 255, 12, 32, 15, 82, 16, 216, 0, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 0, 54, 0, 39, 158, 249, 119, 100, 252, 235, 255, 255] + [255] * 200
    def devinit(self):
        self.cmdtab[0xeb] = self.READ4
        self.cmdtab[0x31] = self.WRSR2

    def iscontinue(self, a):
        return (a & 0x30) == 0x20

    def WRSR2(self, a):
        if self.bytecount == 0:
            self.is_writable()
        return 0x00

def unhex(s):
    return [int(w, 16) for w in s.split()]

class IS25WP064(SpiFlash):
    rdid_data = [0x9d, 0x60, 0x18]
    size = 64
    blobname = "unified"
    sfdp_data = (
        unhex("53 46 44 50 06 01 01 ff 00 06 01 10 30 00 00 ff 9d 05 01 03 80 00 00 02") +
        [255]*24 +
        unhex("e5 20 f9 ff ff ff ff 03 44 3b 08 6b 08 3b 80 bb fe ff ff ff ff ff 00 ff ff ff 44 eb 0c 20 0f 52 10 d8 00 ff 23 4a c9 00 82 d8 11") +
        [255] * 200)
    def devinit(self):
        self.cmdtab[0xeb] = self.READ4
    def iscontinue(self, a):
        return (a & 0x30) == 0x20

########################################################################
# Helper functions for tiling ASTC
#

def tile(f):
    (_,w,h,_,iw,_,ih,_,_,_) = struct.unpack("<IBBBHBHBHB", f.read(16))
    (bw, bh) = ((iw + (w - 1)) / w, (ih + (h - 1)) / h)
    d = f.read()
    return (bw, bh, tile2(d, bw, bh))
def tile2(d, bw, bh):
    assert len(d) == 16 * bw * bh
    fe = [d[i:i+16] for i in range(0, len(d), 16)]
    assert len(fe) == bw * bh
    r = []
    for j in range(0, bh - 1, 2):
        for i in range(0, bw, 2):
            if i < (bw - 1):
                r += [
                    fe[bw * j + i],
                    fe[bw * (j+1) + i],
                    fe[bw * (j+1) + (i+1)],
                    fe[bw * j + (i+1)]]
            else:
                r += [
                    fe[bw * j + i],
                    fe[bw * (j+1) + i]]
    if bh & 1:
        r += fe[bw * (bh - 1):]
    assert len(r) == (bh * bw)
    return "".join(r)

def stdflash_offset(name):
    mp = pickle.load(open(filesearch("build/flash/stdflash.map")))
    print "mp[name]",mp[name]
    return mp[name]

def stdflash_source(name):
    o = stdflash_offset(name)
    assert (o % 32) == 0
    return 0x800000 + (o / 32)
