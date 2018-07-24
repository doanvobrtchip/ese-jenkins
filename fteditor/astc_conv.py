try:
    import Image,ImageChops
except ImportError:
    from PIL import Image,ImageChops
import array
import random
import tempfile
import os
import struct

def astc_dims(fmt):
    if isinstance(fmt, int):
        return astc_dims({
        0x93B0: 'COMPRESSED_RGBA_ASTC_4x4_KHR',
        0x93B1: 'COMPRESSED_RGBA_ASTC_5x4_KHR',
        0x93B2: 'COMPRESSED_RGBA_ASTC_5x5_KHR',
        0x93B3: 'COMPRESSED_RGBA_ASTC_6x5_KHR',
        0x93B4: 'COMPRESSED_RGBA_ASTC_6x6_KHR',
        0x93B5: 'COMPRESSED_RGBA_ASTC_8x5_KHR',
        0x93B6: 'COMPRESSED_RGBA_ASTC_8x6_KHR',
        0x93B7: 'COMPRESSED_RGBA_ASTC_8x8_KHR',
        0x93B8: 'COMPRESSED_RGBA_ASTC_10x5_KHR',
        0x93B9: 'COMPRESSED_RGBA_ASTC_10x6_KHR',
        0x93BA: 'COMPRESSED_RGBA_ASTC_10x8_KHR',
        0x93BB: 'COMPRESSED_RGBA_ASTC_10x10_KHR',
        0x93BC: 'COMPRESSED_RGBA_ASTC_12x10_KHR',
        0x93BD: 'COMPRESSED_RGBA_ASTC_12x12_KHR',
        }[fmt])
    dims = str(fmt).split('_')[3]
    return [int(c) for c in dims.split('x')]


########################################################################
# Helper functions for tiling ASTC
#

def tile(f):
    #ASTC 16 bytes header format as below:
    #I->_ 4 bytes magic number  
    #B->w 1 byte  block dimension X
    #B->h 1 byte  block dimension Y
    #B->_ 1 byte  block dimension Z, always 1 in 2D image
    #H->iw, 2 bytes image width in lower 16 bits
    #B->_,  1 byte  image width in highest 8 bits
    #H->ih, 2 bytes image height in lower 16 bits
    #B->_,  1 byte  image height in highest 8 bits
    #H->_, 2 bytes image depth in lower 16 bits
    #B->_, 1 byte  image depth in highest 8 bits
    (_,w,h,_,iw,_,ih,_,_,_) = struct.unpack("<IBBBHBHBHB", f.read(16))
    (bw, bh) = ((iw + (w - 1)) / w, (ih + (h - 1)) / h)
    (bw, bh) = (int(bw), int(bh))
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

    return b"".join(r)

def pad(im, mult):
    w = ((im.size[0] + (mult-1)) / mult) * mult
    n = Image.new("RGBA", (w, im.size[1]))
    n.paste(im, (0, 0))
    return n

def round_up(n, d):
    return int(d * ((n + d - 1) / d))
    
def convert(im, fmt = "COMPRESSED_RGBA_ASTC_4x4_KHR", effort = "exhaustive", astc_encode = "astcenc"):
    (w, h) = astc_dims(fmt)
    ni = Image.new("RGBA", (round_up(im.size[0], w), round_up(im.size[1], h)))
    ni.paste(im, (0, 0))
    png = tempfile.NamedTemporaryFile(delete = False)
    ni.transpose(Image.FLIP_TOP_BOTTOM).save(png, 'png')
    im = ni

    astc = tempfile.NamedTemporaryFile(delete = False)
    
    os.system("\"%s\" -c %s %s %dx%d -%s -silentmode" % (astc_encode, png.name, astc.name, w, h, effort))

    (bw, bh, d0) = tile(open(astc.name, "rb"))

    astc.close()
    png.close()
    
    if os.path.exists(astc.name):
        os.remove(astc.name)  

    if os.path.exists(png.name):
        os.remove(png.name)          
    
    stride = 16*bw
    dd = array.array('B', d0)
    
    return (stride,ni.size, dd)
