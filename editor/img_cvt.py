import osfrom PIL import Image, ImageDraw, ImageFontimport zlibimport sysimport arrayimport randomimport getopt'''V0.3: Add .rawh for pallette format'''vc_ARGB1555 = 0vc_L1 = 1vc_L4 = 2vc_L8 = 3vc_RGB332 = 4vc_ARGB2 = 5vc_ARGB4 = 6vc_RGB565 =7vc_PALETTED = 8format_table = {vc_ARGB1555 : "ARGB1555",            vc_L1 : "L1",            vc_L4 : "L4",            vc_L8 : "L8",            vc_RGB332 : "RGB332",            vc_ARGB2 : "ARGB2",            vc_ARGB4 : "ARGB4",            vc_RGB565 : "RGB565",            vc_PALETTED : "PALETTED"}help_string = " \         format is as follow:\r\n \         0 : ARGB1555 [default] \r\n \         1 : L1\r\n \         2 : L4\r\n \         3 : L8\r\n \         4 : RGB332\r\n \         5 : ARGB2\r\n \         6 : ARGB4\r\n \         7 : RGB565\r\n \         8 : PALETTEED\r\n \              "version = "V0.5"def print_usage():    print 'Usage: img_cvt -i inputfile -f output_format'    print help_stringclass Image_Conv:    def __init__(self):        self.size = (480, 272)        self.outfile_raw = ''        self.outfile_bin = ''        self.outfile_h = ''        self.outfile_p = ''    def load_image_conv(self, dither=False, fmt=vc_ARGB1555):        im = self.infile        imdata = []        colorfmts = {            vc_ARGB1555 : (1, 5, 5, 5),            vc_RGB332 :   (0, 3, 3, 2),            vc_ARGB2 :    (2, 2, 2, 2),            vc_ARGB4 :    (4, 4, 4, 4),            vc_RGB565 :   (0, 5, 6, 5)}        colorfmts_A = {            vc_ARGB1555 : (1, 5, 5, 5),            vc_ARGB2 :    (2, 2, 2, 2),            vc_ARGB4 :    (4, 4, 4, 4)}        if fmt in colorfmts:            (asz, rsz, gsz, bsz) = colorfmts[fmt]            im_origin = im            im = im.convert("RGBA")            totalsz = sum((asz, rsz, gsz, bsz))            assert totalsz in (8, 16)            if dither:                rnd = random.Random()                rnd.seed(0)            for y in range(im.size[1]):                for x in range(im.size[0]):                    (r, g, b, a) = im.getpixel((x, y))                    if dither:                        a = min(255, a + rnd.randrange(256 >> asz))                        r = min(255, r + rnd.randrange(256 >> rsz))                        g = min(255, g + rnd.randrange(256 >> gsz))                        b = min(255, b + rnd.randrange(256 >> bsz))                    binary = ((a >> (8 - asz)) << (bsz + gsz + rsz)) | ((r >> (8 - rsz)) << (gsz + bsz)) | ((g >> (8 - gsz)) << (bsz)) | (b >> (8 - bsz))                    imdata.append(binary)            fmtchr = {8:'B', 16:'H'}[totalsz]            data = array.array('B', array.array(fmtchr, imdata).tostring())            if fmt in colorfmts_A:                im = im_origin.convert("RGBA")            else:                im = im_origin.convert("RGB")        elif fmt == vc_PALETTED:            im = im.convert("P", palette=Image.ADAPTIVE)            lut = im.resize((256, 1))            lut.putdata(range(256))            palstr = lut.convert("RGBA").tostring()            rgba = zip(*(array.array('B', palstr[i::4]) for i in range(4)))            cwd = os.getcwd()            #os.chdir(self.paletted_dir)            outfile_p  = open(self.filename + '.rawh' , 'w')            outfile_raw  = open(self.filename + '.raw' , 'wb')            outfile_bin = open(self.filename + '.bin' , 'wb')            outfile_bin_h = open(self.filename + '.binh' , 'w')            writestringpal = ('file properties: ', 'resolution ', im.size[0], 'x', im.size[1], 'format ',                        format_table[fmt], 'LUT in RAM_PAL')            outfile_p.write("/*")            outfile_p.write(str(writestringpal))            outfile_p.write("*/ \n")            index = 0            for i, (r, g, b, a) in enumerate(rgba):                outfile_p.write(str(b))                outfile_p.write(",")                outfile_p.write(str(g))                outfile_p.write(",")                outfile_p.write(str(r))                outfile_p.write(",")                outfile_p.write(str(a))                outfile_p.write(",")                outfile_raw.write(chr(b))                outfile_raw.write(chr(g))                outfile_raw.write(chr(r))                outfile_raw.write(chr(a))                imdata.append(b)                imdata.append(g)                imdata.append(r)                imdata.append(a)                index = index + 1                if (0 == index % 32):                    outfile_p.write("\n")            deflatdata = zlib.compress(array.array('B', imdata))            outfile_bin.write(deflatdata)            for i in deflatdata:                outfile_bin_h.write(str(ord(i)))                outfile_bin_h.write(",")            file.close(outfile_p)            file.close(outfile_raw)            file.close(outfile_bin)            file.close(outfile_bin_h)            #os.chdir(cwd)            data = array.array('B', im.tostring())            totalsz = 8        elif fmt == vc_L8:            im = im.convert("L")            data = array.array('B', im.tostring())            totalsz = 8        elif fmt == vc_L2:            im = im.convert("L")            def to3(c):                return int(round(3 * ord(c) /255.))            data = array.array('B',im.tostring())            #TwoBits = array.array('B', [ to3(l) for l in data ]            #LowBits =        elif fmt == vc_L4:            (newWidth,newHeight) = ((im.size[0] + 1) & ~1,im.size[1])            im = im.resize((newWidth,newHeight),Image.BICUBIC)            im = im.convert("L")            b0 = im.tostring()[::2]#even numbers            b1 = im.tostring()[1::2]#odd numbers            def to15(c):                return int(round(15 * ord(c) / 255.))            data = array.array('B', [((to15(l) << 4) + to15(r)) for (l, r) in zip(b0, b1)])            totalsz = 4        elif fmt == vc_L1:            if dither:                im = im.convert("1", dither=Image.FLOYDSTEINBERG)            else:                im = im.convert("1", dither=Image.NONE)            data = array.array('B', im.tostring())            totalsz = 1        if totalsz == 8:            stride = im.size[0]        if totalsz == 16:            stride = im.size[0] * 2        if totalsz == 1:            stride = (im.size[0] + 7)/8        if totalsz == 4:            stride = im.size[0] / 2        im.save(self.filename +"_converted.png","PNG")        #compress the data into .bin & .binh file        deflatedata = zlib.compress(data)        self.outfile_bin.write(deflatedata)        for i in deflatedata:            self.outfile_bin_h.write(str(ord(i)))            self.outfile_bin_h.write(",")        self.outfile_bin.close()        self.outfile_bin_h.close()        #Write the raw data into .raw file        data.tofile(self.outfile_raw)        self.outfile_raw.close()        #write input file details into .h file        writestring = ('file properties: ', 'resolution ', im.size[0], 'x', im.size[1], 'format ',            format_table[fmt], 'stride ', stride, ' total size ', len(data))        self.outfile_h.write("/*")        self.outfile_h.write(str(writestring))        self.outfile_h.write("*/ \n")        index = 0        for i in data:            self.outfile_h.write(str(i))            self.outfile_h.write(",")            index = index + 1            if (0 == index % (32 * totalsz / 8)):                self.outfile_h.write("\n")        self.outfile_h.close()        return im.size    def run(self,argv):        infile = ""	outname = ""        output_format = vc_RGB565        dither = False        try:            opts,args = getopt.getopt(argv,"hi:f:o:")        except getopt.GetoptError:            print 'opt error'            print_usage()            return        for opt,arg in opts:            if opt  == '-h':                print_usage()                return            elif opt == '-i':                infile = arg                if False == os.path.exists(infile):                    print 'invalid input file'                    return            elif opt == '-f':                output_format = int(arg)                if output_format not in [0,1,2,3,4,5,6,7,8]:                    print 'output format is not supported'                    return            elif opt == '-o':                outname = arg            else:                print 'bad opt ' + opt                print_usage()                return        try:            self.infile = Image.open(infile)        except:            print 'Error to open file ' + '\'' + infile +'\''            print_usage()            return        filename = infile.split('.',1)[0]        if outname == "":            print 'Outname not specified'            outname = filename        #dirname = outname        #if os.path.isdir(dirname):        #    print 'Error:'+ dirname + ' directory exists'        #    print_usage()        #    return        #os.makedirs(dirname)        #os.chdir(dirname)        self.filename = outname        self.outfile_raw = open(outname + '.raw' , 'wb')        self.outfile_h   = open(outname + '.rawh', 'w')        self.outfile_bin = open(outname + '.bin' , 'wb')        self.outfile_bin_h = open(outname + '.binh' , 'w')        if output_format == vc_PALETTED:            self.paletted_dir = dirname + "_LUT"            #os.makedirs(self.paletted_dir)        self.load_image_conv(dither,output_format)        print "convert complete!"if __name__ == '__main__':    print 'image conversion utility for FT800 ' + version    Image_Conv().run(sys.argv[1:])