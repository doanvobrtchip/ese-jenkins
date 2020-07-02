import os
import zlib
import sys
import array
import random
import getopt
import png
import subprocess
import shutil
from helperapi import resource_path
from PIL import Image

import astc_conv
import json

'''
V0.9.3: Upgrade Python 3.6.5
V0.9.2: Support ASTC format; Provide more information on display list commands
V0.9.1: Update the Python Image Library(PIL) to 1.1.7 from PILLOW 4.0.0
V0.9: Support Dithering option
V0.8: Added support for paletted formats.
V0.7: Fixed an issue where odd sized input images are not properly generated for the L2 format.
V0.6: Added L2 format support for FT81X
V0.5: Resize the image width to even number when L4 is converted format.
      Add stride information in .Rawh file
V0.4: First external release
V0.2: Support specific raw or bin output
'''
version = "V0.9.3"
def print_version():
    print ('Image conversion utility for EVE ' + version)
vc_ARGB1555 = 0
vc_L1 = 1
vc_L4 = 2
vc_L8 = 3
vc_RGB332 = 4
vc_ARGB2 = 5
vc_ARGB4 = 6
vc_RGB565 = 7
vc_PALETTED = 8
vc_L2 = 9
vc_PALETTED565 = 10
vc_PALETTED4444 = 11
vc_PALETTED8 = 12

VC_ASTC_4X4_KHR = 13
VC_ASTC_5X4_KHR = 14
VC_ASTC_5X5_KHR = 15
VC_ASTC_6X5_KHR = 16
VC_ASTC_6X6_KHR = 17
VC_ASTC_8X5_KHR = 18
VC_ASTC_8X6_KHR = 19
VC_ASTC_8X8_KHR = 20
VC_ASTC_10X5_KHR = 21
VC_ASTC_10X6_KHR = 22
VC_ASTC_10X8_KHR = 23
VC_ASTC_10X10_KHR = 24
VC_ASTC_12X10_KHR = 25
VC_ASTC_12X12_KHR = 26

format_table = {vc_ARGB1555: "ARGB1555",
                vc_L1: "L1",
                vc_L4: "L4",
                vc_L8: "L8",
                vc_RGB332: "RGB332",
                vc_ARGB2: "ARGB2",
                vc_ARGB4: "ARGB4",
                vc_RGB565: "RGB565",
                vc_PALETTED: "PALETTED",
                vc_L2: "L2",
                vc_PALETTED565: "PALETTED565",
                vc_PALETTED4444: "PALETTED4444",
                vc_PALETTED8: "PALETTED8",
                VC_ASTC_4X4_KHR: "COMPRESSED_RGBA_ASTC_4x4_KHR",
                VC_ASTC_5X4_KHR: "COMPRESSED_RGBA_ASTC_5x4_KHR",
                VC_ASTC_5X5_KHR: "COMPRESSED_RGBA_ASTC_5x5_KHR",
                VC_ASTC_6X5_KHR: "COMPRESSED_RGBA_ASTC_6x5_KHR",
                VC_ASTC_6X6_KHR: "COMPRESSED_RGBA_ASTC_6x6_KHR",
                VC_ASTC_8X5_KHR: "COMPRESSED_RGBA_ASTC_8x5_KHR",
                VC_ASTC_8X6_KHR: "COMPRESSED_RGBA_ASTC_8x6_KHR",
                VC_ASTC_8X8_KHR: "COMPRESSED_RGBA_ASTC_8x8_KHR",
                VC_ASTC_10X5_KHR: "COMPRESSED_RGBA_ASTC_10x5_KHR",
                VC_ASTC_10X6_KHR: "COMPRESSED_RGBA_ASTC_10x6_KHR",
                VC_ASTC_10X8_KHR: "COMPRESSED_RGBA_ASTC_10x8_KHR",
                VC_ASTC_10X10_KHR: "COMPRESSED_RGBA_ASTC_10x10_KHR",
                VC_ASTC_12X10_KHR: "COMPRESSED_RGBA_ASTC_12x10_KHR",
                VC_ASTC_12X12_KHR: "COMPRESSED_RGBA_ASTC_12x12_KHR",
                }

supported_astc_formats = [VC_ASTC_4X4_KHR,
                VC_ASTC_5X4_KHR,
                VC_ASTC_5X5_KHR,
                VC_ASTC_6X5_KHR,
                VC_ASTC_6X6_KHR,
                VC_ASTC_8X5_KHR,
                VC_ASTC_8X6_KHR,
                VC_ASTC_8X8_KHR,
                VC_ASTC_10X5_KHR,
                VC_ASTC_10X6_KHR,
                VC_ASTC_10X8_KHR,
                VC_ASTC_10X10_KHR,
                VC_ASTC_12X10_KHR,
                VC_ASTC_12X12_KHR]

supported_formats = supported_astc_formats + [vc_ARGB1555, vc_L1, vc_L4, vc_L8, vc_RGB332, vc_ARGB2, vc_ARGB4, vc_RGB565, vc_L2, vc_PALETTED, vc_PALETTED565, vc_PALETTED4444, vc_PALETTED8]

binary_formats = supported_astc_formats + [vc_ARGB1555, vc_L1, vc_L4, vc_L8, vc_RGB332, vc_ARGB2, vc_ARGB4, vc_RGB565, vc_L2]
paletted_formats = [vc_PALETTED, vc_PALETTED565, vc_PALETTED4444, vc_PALETTED8]

help_string = r"""
         output format is as follows:
         0 : ARGB1555 [default]
         1 : L1
         2 : L4
         3 : L8
         4 : RGB332
         5 : ARGB2
         6 : ARGB4
         7 : RGB565
         8 : PALETTED [FT80X only]
         9 : L2 [FT81X only]
         10 : PALETTED565 [FT81X only]
         11 : PALETTED4444 [FT81X only]
         12 : PALETTED8 [FT81X only]
         13~26: ASTC format [BT815 only]
                13: "COMPRESSED RGBA ASTC 4x4 KHR",
                14: "COMPRESSED RGBA ASTC 5x4 KHR",
                15: "COMPRESSED RGBA ASTC 5x5 KHR",
                16: "COMPRESSED RGBA ASTC 6x5 KHR",
                17: "COMPRESSED RGBA ASTC 6x6 KHR",
                18: "COMPRESSED RGBA ASTC 8x5 KHR",
                19: "COMPRESSED RGBA ASTC 8x6 KHR",
                20: "COMPRESSED RGBA ASTC 8x8 KHR",
                21: "COMPRESSED RGBA ASTC 10x5 KHR",
                22: "COMPRESSED RGBA ASTC 10x6 KHR",
                23: "COMPRESSED RGBA ASTC 10x8 KHR",
                24: "COMPRESSED RGBA ASTC 10x10 KHR",
                25: "COMPRESSED RGBA ASTC 12x10 KHR",
                26: "COMPRESSED RGBA ASTC 12x12 KHR",
            """


pngquant = "pngquant.exe"

def print_usage():
    print_version()
    print('Usage: img_cvt -i inputfile -f output_format [-d] [-e effortoption]')
    print('{}'.format(help_string))
    print('        Unless -d is specified, dithering is turned on by default\n')
    print( '-e:     Set the effort option for ASTC output format, valid strings are: ')
    print( '            veryfast  ')
    print( '            fast')
    print( '            medium')
    print( '            thorough')
    print( '            exhaustive')
    print( '-o:     Output Folder')

def is_valid_effort(effort):
    return effort in ['veryfast',
                      'fast',
                      'medium',
                      'thorough',
                      'exhaustive']
def execute_subprocess(cmd):
    try:
        if os.name == "nt":
            proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            proc.communicate()
            return proc.returncode
    except Exception as ex:
        raise Exception("Unable to execute subprocess.")

def writeToJSONFile(path, fileName, data):
    filePathNameWExt = path + '/' + fileName + '.json'
    with open(filePathNameWExt, 'w') as fp:
        json.dump(data, fp)

def pad(im, mult):
    #w = ((im.size[0] + (mult-1)) / mult) * mult
    w = (im.size[0] + 1) & ~1
    n = Image.new(im.mode, (w, im.size[1]))
    n.paste(im, (0, 0))
    return n

def remove_transparency(im, bg_colour=(0, 0, 0)):

    # Only process if image has transparency (http://stackoverflow.com/a/1963146)
    if im.mode in ('RGBA', 'LA') or (im.mode == 'P' and 'transparency' in im.info):

        # Need to convert to RGBA if LA format due to a bug in PIL (http://stackoverflow.com/a/1963146)
        alpha = im.convert('RGBA').split()[-1]

        # Create a new background image of our matt color.
        # Must be RGBA because paste requires both images have the same format
        # (http://stackoverflow.com/a/8720632  and  http://stackoverflow.com/a/9459208)
        bg = Image.new('RGBA', im.size, bg_colour + (0,))
        bg.paste(im, mask=alpha)
        return bg.convert('RGB')
    else:
        return im

class Image_Conv:
    def __init__(self):
        self.size = (480, 272)
        self.filename = None

        self.infile = None
        self.infile_basename = None
        self.infile_name = None
        self.output_format = None
        self.output_dir = None

        # regular images
        self.main_raw = None
        self.main_rawh = None
        self.main_bin = None
        self.main_binh = None

        # paletted images
        self.index_raw = None
        self.index_rawh = None
        self.index_bin = None
        self.index_binh = None
        self.lut_raw = None
        self.lut_rawh = None
        self.lut_bin = None
        self.lut_binh = None

    # pre-allocate all files
    def generate_file_paths(self, inputfile):
    
        cwd = os.getcwd()

        abspath = os.path.abspath(inputfile)
        self.infile_name = os.path.basename(abspath)
        self.infile_basename = os.path.split(self.output_dir)[-1]
        
        if self.output_dir is None:
            self.output_dir = os.path.join(cwd, '{}_{}'.format(self.infile_basename, format_table[self.output_format]))
        else:
            self.output_dir = os.path.split(self.output_dir)[0]

        try:
            if not os.path.exists(self.output_dir):
                os.makedirs(self.output_dir)
        except Exception as ex:
            raise Exception('Error occured while processing output folder: {}. {}'.format(self.output_dir, ex))

        # regular images
        if self.output_format in binary_formats:
            try:
                self.main_bin = open(os.path.join(self.output_dir, '{}.bin'.format(self.infile_basename)), 'wb')
                self.main_binh = open(os.path.join(self.output_dir, '{}.binh'.format(self.infile_basename)), 'w')
                self.main_raw = open(os.path.join(self.output_dir, '{}.raw'.format(self.infile_basename)), 'wb')
                self.main_rawh = open(os.path.join(self.output_dir, '{}.rawh'.format(self.infile_basename)), 'w')
            except Exception as ex:
                raise Exception('Unable to generate output files, check directory permission at: {} ({})'.format(self.output_dir, ex))

        # paletted images
        elif self.output_format in paletted_formats:
            try:
                self.index_raw = open(os.path.join(self.output_dir, '{}.raw'.format(self.infile_basename)), 'wb')
                self.index_rawh = open(os.path.join(self.output_dir, '{}.rawh'.format(self.infile_basename)), 'w')
                self.index_bin = open(os.path.join(self.output_dir, '{}.bin'.format(self.infile_basename)), 'wb')
                self.index_binh = open(os.path.join(self.output_dir, '{}.binh'.format(self.infile_basename)), 'w')
            except Exception as ex:
                raise Exception('Unable to generate output files, check directory permission at: {} ({})'.format(self.output_dir, ex))
            
            outputdir_lut = self.output_dir 
            try:
                self.lut_raw = open(os.path.join(outputdir_lut, '{}.lut.raw'.format(self.infile_basename)), 'wb')
                self.lut_rawh = open(os.path.join(outputdir_lut, '{}.lut.rawh'.format(self.infile_basename)), 'w')
                self.lut_bin = open(os.path.join(outputdir_lut, '{}.lut.bin'.format(self.infile_basename)), 'wb')
                self.lut_binh = open(os.path.join(outputdir_lut, '{}.lut.binh'.format(self.infile_basename)), 'w')
            except:
                raise Exception('Unable to generate output files, check directory permission at: {}'.format(outputdir_lut))       

    def ispng8(self, file):
        """ Check the input file to see whether it's a png8 formatted image or not, the PNG.py is required """
        try:
            infile = png.Reader(filename = file)
            infile_reader = infile.read_flat()
            bitdepth = infile_reader[3]['bitdepth']
            try:
                if infile_reader[3]['palette'] is None:
                    return False
                if bitdepth != 8 and bitdepth != 4 and bitdepth != 2 and bitdepth != 1:
                    return False
                return True
            except KeyError:
                    return False
        except:
            raise

    def save_binfiles(self,data,im_size):
        #compress the data into .bin file
        deflatedata = zlib.compress(data)
        self.main_bin.write(deflatedata)
        self.main_bin.close()

        #Prepare the header for the .binh file
        writestring = ('file properties: ', 'resolution ', im_size[0], 'x', im_size[1], 'format ',
            format_table[self.output_format], 'stride ', self.stride, ' total size ', len(data))

        self.main_binh.write("/*")
        self.main_binh.write(str(writestring))
        self.main_binh.write("*/ \n")

        #write the data by byte
        for i in deflatedata:
            self.main_binh.write('{}'.format(i))
            self.main_binh.write(",")
        self.main_binh.close()

    def save_rawfiles(self, data, im_size, totalsz, is_astc = False):
        #print "is_astc is " + str(is_astc)
        #Write the raw data into .raw file
        data.tofile(self.main_raw)
        self.main_raw.close()

        #write input file details into .h file
        writestring = ('file properties: ', 'resolution ', im_size[0], 'x', im_size[1], 'format ',
            format_table[self.output_format], 'stride ', self.stride, ' total size ', len(data))

        self.main_rawh.write("/*")
        self.main_rawh.write(str(writestring))
        self.main_rawh.write("*/ \n")

        index = 0
        for i in data:
            self.main_rawh.write(str(i))
            self.main_rawh.write(",")
            index = index + 1
            #Break line each line of pixels when it is ASTC
            if is_astc:
                if 0 == index % self.stride:
                    self.main_rawh.write("\n")
            #Break the line every 32 pixels
            elif 0 == index % (32 * totalsz / 8):
                self.main_rawh.write("\n")
        self.main_rawh.close()

    def calc_stride(self, im, bitsPerPixel):
        # TODO: warn the user if the width will not yield an integer stride value.
        if bitsPerPixel == 8:
            stride = im.size[0]
        if bitsPerPixel == 16:
            stride = im.size[0] * 2
        if bitsPerPixel == 1:
            stride = (im.size[0] + 7)/8
        if bitsPerPixel == 2:
            stride = im.size[0] / 4
        if bitsPerPixel == 4:
            stride = im.size[0] / 2

        return stride

    def load_image_conv(self, dither = False):

        im = self.infile
        is_astc = False
        # Handle alpha bitmaps here. A solid color in all
        # pixels with nonzero alpha means use alpha channel
        if self.output_format in (vc_L1, vc_L2, vc_L4, vc_L8, vc_RGB332, vc_RGB565):
            im = remove_transparency(im)

        colorfmts = {
            vc_ARGB1555: (1, 5, 5, 5),
            vc_RGB332:   (0, 3, 3, 2),
            vc_ARGB2:    (2, 2, 2, 2),
            vc_ARGB4:    (4, 4, 4, 4),
            vc_RGB565:   (0, 5, 6, 5)}

        colorfmts_A = {
            vc_ARGB1555: (1, 5, 5, 5),
            vc_ARGB2:    (2, 2, 2, 2),
            vc_ARGB4:    (4, 4, 4, 4)}

        if self.output_format in supported_astc_formats:
            def is_exe(fpath):
                return os.path.isfile(fpath) and os.access(fpath, os.X_OK)
            if (not self.astc_encode.endswith('astcenc.exe')) or (not is_exe(self.astc_encode)):
                raise Exception( self.astc_encode + " is not a valid astc-encoder")
            is_astc = True
            astc_format = {
            VC_ASTC_4X4_KHR   :0x93B0 ,
            VC_ASTC_5X4_KHR   :0x93B1 ,
            VC_ASTC_5X5_KHR   :0x93B2 ,
            VC_ASTC_6X5_KHR   :0x93B3 ,
            VC_ASTC_6X6_KHR   :0x93B4 ,
            VC_ASTC_8X5_KHR   :0x93B5 ,
            VC_ASTC_8X6_KHR   :0x93B6 ,
            VC_ASTC_8X8_KHR   :0x93B7 ,
            VC_ASTC_10X5_KHR  :0x93B8 ,
            VC_ASTC_10X6_KHR  :0x93B9 ,
            VC_ASTC_10X8_KHR  :0x93BA ,
            VC_ASTC_10X10_KHR :0x93BB ,
            VC_ASTC_12X10_KHR :0x93BC ,
            VC_ASTC_12X12_KHR :0x93BD ,
            }[self.output_format]

            (stride,(iw,ih),data) = astc_conv.convert(im, astc_format, self.astc_effort, self.astc_encode)

            self.stride = stride

            im = im.resize((iw,ih), Image.BICUBIC)
            im.save(os.path.join(self.output_dir, self.infile_basename +"_Converted.png"), "PNG")
            self.save_binfiles(data, im.size)
            self.save_rawfiles(data, im.size, self.stride, is_astc)
            return im.size

        elif self.output_format in colorfmts:
            (asz, rsz, gsz, bsz) = colorfmts[self.output_format]
            im_origin = im

            im = im.convert("RGBA")
            imdata = []

            totalsz = sum((asz, rsz, gsz, bsz))
            assert totalsz in (8, 16)

            if dither:
                rnd = random.Random()
                rnd.seed(0)
            for y in range(im.size[1]):
                for x in range(im.size[0]):
                    (r, g, b, a) = im.getpixel((x, y))

                    if dither:
                        a = min(255, a + rnd.randrange(256 >> asz))
                        r = min(255, r + rnd.randrange(256 >> rsz))
                        g = min(255, g + rnd.randrange(256 >> gsz))
                        b = min(255, b + rnd.randrange(256 >> bsz))
                    binary = ((a >> (8 - asz)) << (bsz + gsz + rsz)) | ((r >> (8 - rsz)) << (gsz + bsz)) | ((g >> (8 - gsz)) << (bsz)) | (b >> (8 - bsz))

                    imdata.append(binary)

            fmtchr = {8: 'B', 16: 'H'}[totalsz]
            data = array.array('B', array.array(fmtchr, imdata).tobytes())

            if self.output_format in colorfmts_A:
                im = im_origin.convert("RGBA")
            else:
                im = remove_transparency(im)

        elif self.output_format == vc_L8:
            im = im.convert("L")
            data = array.array('B', im.tobytes())
            totalsz = 8

        elif self.output_format == vc_L4:
            (newWidth, newHeight) = ((im.size[0] + 1) & ~1, im.size[1])
            im = im.resize((newWidth,newHeight),Image.BICUBIC)
            #im = pad(im,2)
            im = im.convert("L")

            b0 = im.tobytes()[::2]  #even numbers
            b1 = im.tobytes()[1::2] #odd numbers

            def to15(c):
                return int(round(15 * c / 255.))

            data = array.array('B', [((to15(l) << 4) + to15(r)) for (l, r) in zip(b0, b1)])
            totalsz = 4

        elif self.output_format == vc_L2:
            (newWidth, newHeight) = ((im.size[0] + 3) & ~3,im.size[1])
            #im = pad(im,4)
            im = im.resize((newWidth,newHeight),Image.BICUBIC)
            im = im.convert("L")

            totalsz = 2
            b0 = im.tobytes()[0::4]     # even numbers
            b1 = im.tobytes()[1::4]     # odd numbers
            b2 = im.tobytes()[2::4]     # even numbers
            b3 = im.tobytes()[3::4]     # odd numbers

            def to3(c):
                return int(round(3 * c / 255.))

            data = array.array('B', [((to3(l) << 6) + (to3(r)<<4) + (to3(x)<<2) + to3(y)) for (l, r, x, y) in zip(b0, b1, b2, b3)])

        elif self.output_format == vc_L1:
            if dither:
                im = im.convert("1", dither=Image.FLOYDSTEINBERG)
            else:
                im = im.convert("1", dither=Image.NONE)
            data = array.array('B', im.tobytes())
            totalsz = 1

        im.save(os.path.join(self.output_dir, self.infile_basename +"_Converted.png"), "PNG")

        self.stride = self.calc_stride(im, totalsz)
        self.save_binfiles(data, im.size)
        self.save_rawfiles(data, im.size, totalsz, is_astc)
    
        return im.size

    def load_paletted_conv(self, infile_name=None):

        if not infile_name:
            raise Exception('Palette conversion error: missing input file.')

        try:
            infile = png.Reader(filename=infile_name)
        except:
            raise Exception('Unable to open: {}'.format(infile_name))

        try:
            infile_Direct = infile.asDirect()
        except:
            raise Exception('The input file doesn\'t appear to be an PNG image.')

        try:
            infile_reader = infile.read_flat()
        except:
            raise Exception('Error while attempting to read the input image.')

        bitdepth = infile_reader[3]['bitdepth']
        try:
            if infile_reader[3]['palette'] is None:
                raise Exception('This PNG file is not encoded as Palette based')
            if bitdepth != 8 and bitdepth != 4 and bitdepth != 2 and bitdepth != 1:
                raise Exception('this PNG file\'s bitdepth is illegal')
        except Exception as ex:
                raise Exception('Please convert the input PNG to palette mode by using pngQuant utility first. ({})'.format(ex))

        img_indexdata = infile_reader[2]

        def write_index(img_data):
            imdata = []
            index = 0
            stride = 0

            if self.output_format == vc_PALETTED:
                stride = infile_Direct[0]
            elif self.output_format == vc_PALETTED565 or self.output_format == vc_PALETTED4444 or self.output_format == vc_PALETTED8:
                stride = infile_Direct[0]

            writestring = ('file properties: ', 'resolution ', infile_Direct[0], 'x', infile_Direct[1], 'format ',
                format_table[self.output_format], 'stride ', stride, ' total size ', infile_Direct[0]*infile_Direct[1])

            self.index_rawh.write("/*")
            self.index_rawh.write(str(writestring))
            self.index_rawh.write("*/\n")

            for i in img_data:
                self.index_raw.write(chr(i).encode("latin-1"))
                imdata.append(i)
                self.index_rawh.write(str(i))
                self.index_rawh.write(",")

                index = index + 1
                if 0 == index % 32:
                    self.index_rawh.write("\n")

            self.index_raw.close()
            self.index_rawh.close()

            if self.iszip:
                deflatdata = zlib.compress(array.array('B', imdata))
                self.index_bin.write(deflatdata)
                self.index_bin.close()
                index = 0
                for i in deflatdata:
                    self.index_binh.write(str(i))
                    self.index_binh.write(",")
                    index = index  + 1
                    if 0 == index % 32:
                        self.index_binh.write("\n")
                self.index_binh.close()
                #os.remove(os.path.join(self.output_dir, '{}_index.raw'.format(self.infile_basename)))
                #os.remove(os.path.join(self.output_dir, '{}_index.rawh'.format(self.infile_basename)))

        write_index(img_indexdata)

        lut = infile_reader[3]['palette']

        def write_lutfile(lut):
            imdata = []
            index = 0

            if len(lut[0]) == 3:
                for (r, g, b) in lut:
                    if self.output_format == vc_PALETTED or self.output_format == vc_PALETTED8:
                        self.lut_raw.write(chr(b).encode('latin-1'))
                        self.lut_raw.write(chr(g).encode('latin-1'))
                        self.lut_raw.write(chr(r).encode('latin-1'))
                        self.lut_raw.write(chr(255).encode('latin-1'))

                        imdata.append(b)
                        imdata.append(g)
                        imdata.append(r)
                        imdata.append(255)

                        self.lut_rawh.write(str(b))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(g))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(r))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(255))
                        self.lut_rawh.write(",")

                    elif self.output_format == vc_PALETTED565:
                        rgb565encoding = ( int((r * 31) / 255) << 11) | (int((g * 63) / 255) << 5) | (int((b * 31) / 255) & 31)
                        rgb565upper = rgb565encoding & 255
                        rgb565lower = (rgb565encoding >> 8) & 255
                        self.lut_raw.write(chr(rgb565upper).encode('latin-1'))
                        self.lut_raw.write(chr(rgb565lower).encode('latin-1'))

                        imdata.append(rgb565upper)
                        imdata.append(rgb565lower)

                        self.lut_rawh.write(str(rgb565upper))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(rgb565lower))
                        self.lut_rawh.write(",")

                    elif self.output_format == vc_PALETTED4444:
                        argb4444encoding = (int((255 * 15) / 255) << 12) | (int((r * 15) / 255) << 8) | (int((g * 15) / 255) << 4) | (int((b * 15) / 255) & 15)
                        argb4444upper = argb4444encoding & 255
                        argb4444lower = (argb4444encoding >> 8) & 255
                        self.lut_raw.write(chr(argb4444upper).encode('latin-1'))
                        self.lut_raw.write(chr(argb4444lower).encode('latin-1'))

                        imdata.append(argb4444upper)
                        imdata.append(argb4444lower)

                        self.lut_rawh.write(str(argb4444upper))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(argb4444lower))
                        self.lut_rawh.write(",")
                    index = index + 1
                    if 0 == index % 32:
                        self.lut_rawh.write("\n")
            else:
                for (r, g, b, a) in lut:
                    if self.output_format == vc_PALETTED or self.output_format == vc_PALETTED8:
                        self.lut_raw.write(chr(b).encode('latin-1'))
                        self.lut_raw.write(chr(g).encode('latin-1'))
                        self.lut_raw.write(chr(r).encode('latin-1'))
                        self.lut_raw.write(chr(a).encode('latin-1'))

                        imdata.append(b)
                        imdata.append(g)
                        imdata.append(r)
                        imdata.append(a)

                        self.lut_rawh.write(str(b))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(g))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(r))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(a))
                        self.lut_rawh.write(",")
                    elif self.output_format == vc_PALETTED565:
                        rgb565encoding = ( int((r * 31) / 255) << 11) | ( int((g * 63) / 255) << 5) | ( int((b * 31) / 255) & 31)
                        rgb565upper = rgb565encoding & 255
                        rgb565lower = (rgb565encoding >> 8) & 255
                        self.lut_raw.write(chr(rgb565upper).encode('latin-1'))
                        self.lut_raw.write(chr(rgb565lower).encode('latin-1'))

                        imdata.append(rgb565upper)
                        imdata.append(rgb565lower)

                        self.lut_rawh.write(str(rgb565upper))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(rgb565lower))
                        self.lut_rawh.write(",")

                    elif  self.output_format == vc_PALETTED4444:
                        argb4444encoding = (int((a * 15) / 255) << 12) | (int((r * 15) / 255) << 8) | (int((g * 15) / 255) << 4) | (int((b * 15) / 255) & 15)
                        argb4444upper = argb4444encoding & 255
                        argb4444lower = (argb4444encoding >> 8) & 255
                        self.lut_raw.write(chr(argb4444upper).encode('latin-1'))
                        self.lut_raw.write(chr(argb4444lower).encode('latin-1'))

                        imdata.append(argb4444upper)
                        imdata.append(argb4444lower)

                        self.lut_rawh.write(str(argb4444upper))
                        self.lut_rawh.write(",")
                        self.lut_rawh.write(str(argb4444lower))
                        self.lut_rawh.write(",")

                    index = index + 1
                    if 0 == index % 32:
                        self.lut_rawh.write("\n")

            self.lut_raw.close()
            self.lut_rawh.close()
            if self.iszip:
                deflatdata = zlib.compress(array.array('B', imdata))
                self.lut_bin.write(deflatdata)
                self.lut_bin.close()
                index = 0
                for i in deflatdata:
                    self.lut_binh.write("{}".format(i))
                    self.lut_binh.write(",")
                    index = index + 1
                    if 0 == index % 32:
                        self.lut_binh.write("\n")
                self.lut_binh.close()

        write_lutfile(lut)
        
    def run(self, argv):

        dither = True
        self.astc_effort = "exhaustive"
        self.astc_encode = resource_path("astcenc.exe")
        self.iszip = True
        
        try:
            opts, args = getopt.getopt(argv, "vhdi:f:e:x:o:z")
        except getopt.GetoptError:
            print_usage()
            sys.exit(2)

        for opt, arg in opts:
            if opt == '-v':
                print_version()
                sys.exit(0)
            if opt == '-h':
                print_usage()
                sys.exit(0)
            if opt == '-d':
                dither = False
                print("Dithering is turned off Now\n")
            elif opt == '-i':
                self.filename = arg
                if not os.path.exists(self.filename):
                    raise Exception('Input file doesn\'t exist')
            elif opt == '-f':
                self.output_format = int(arg)
                if self.output_format not in supported_formats:
                    raise Exception('The input format is not supported.')
            elif opt == '-e':
                self.astc_effort = arg
                if not is_valid_effort(arg):
                    raise Exception('Invalid ASTC effort option.')
            elif opt == '-x':
                self.astc_encode = arg
            elif opt == '-o':
                self.output_dir = arg
            elif opt == "-z":
                self.iszip = True
            else:
                print_usage()
                sys.exit(2)

        if self.filename is None:
            print_usage()
            print('Missing image input parameter.')
            sys.exit(2)

        if self.output_format is None:
            self.output_format = 0
            print('Default output format to: {}'.format(format_table[self.output_format]))

        # pre-allocate all files
        self.generate_file_paths(self.filename)

        # TODO: if the image width dimension will not yield an integer stride value, resize the input image or simply warn the user.  Resizing might alter the aspect ratio of the original image.  Resize it but warn user that the input image has been resized due to the selected format.  The input image file format might get changed.

        # for paletted formats, if the input image is not png8, convert it.
        if self.output_format in paletted_formats:
            try:
                if (not self.filename.endswith('.png')) or (not self.filename.endswith('.PNG')):
                    convertedpng = os.path.join(self.output_dir, self.infile_basename + "_converted.png")
                    pngimg = Image.open(self.filename)
                    pngimg.save(convertedpng, "PNG")
                    self.filename = convertedpng

                if self.ispng8(self.filename) is False:
                    newimg = os.path.join(self.output_dir, self.infile_basename + "-fs8.png")
                    cmd = '\"{}\" -f --output \"{}\" \"{}\"'.format(resource_path(pngquant), newimg, self.filename)
                    returncode = execute_subprocess(cmd)
                    if returncode != 0:
                        raise Exception(resource_path(pngquant), pngquant, newimg, self.filename, 'Unable to convert image to PNG8')
                    self.filename = newimg


            except Exception as ex:  
                self.index_raw.close()
                self.index_rawh.close()
                self.index_bin.close()
                self.index_binh.close()
                self.lut_raw.close()
                self.lut_rawh.close()
                self.lut_bin.close()
                self.lut_binh.close()

                raise Exception(str(ex), 'The input image is incompatible. Error occurred while attempting to convert the '
                                'input image to a compatible format.')                               

        # convert to a paletted format
        if self.output_format in paletted_formats:
            self.load_paletted_conv(infile_name=self.filename)
            self.infile = Image.open(self.filename)
            outputsize = self.infile.size
            
            self.infile.fp.close()
            
        # convert to a regular binary format
        else:
            try:
                self.infile = Image.open(self.filename)
            except:
                self.infile.fp.close()
                
                self.main_raw.close()
                self.main_rawh.close()
                self.main_bin.close()
                self.main_binh.close()
                shutil.rmtree(self.output_dir)
                raise Exception('The input image is not compatible: {}'.format(self.filename))

            outputsize = self.load_image_conv(dither)

        print('Image Conversion completed!\n')
        json = {}
        print( "Filename              : %s" % self.infile_basename)
        json['name'] = self.infile_basename
        json['type'] = 'bitmap'
        print( "Format                : %s" % format_table[self.output_format])
        json['format'] = format_table[self.output_format]
        if self.output_format in supported_astc_formats:
            print( "ASTC compression speed: %s" % self.astc_effort)
            json['astcCompressionSpeed'] = self.astc_effort
        print( "Width                 : %d" % outputsize[0])
        json['width'] = outputsize[0]
        print( "Height                : %d" % outputsize[1])
        json['height'] = outputsize[1]
        if self.iszip:
            print( "Compressed            : yes")
            json['compressed'] = 1
        else:
            print( "Compressed            : no")
            json['compressed'] = 0
        writeToJSONFile(self.output_dir, self.infile_basename, json)

if __name__ == '__main__':
    Image_Conv().run(sys.argv[1:])
