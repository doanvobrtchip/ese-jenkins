import wave
import audioop
import sys
import os
import array
import zlib
import getopt

usage = "Usage: aud_cvt [h] -i inputfile -f output_format"

output_format_help = "Output format: \r\n \
        0: 8 Bit Signed PCM(LINEAR)[default]  \r\n \
        1: 8 Bit uLaw \r\n \
        2: 4 Bit IMA ADPCM    \r\n \
        "

format_table = {
        0:"8 Bit Signed PCM",
        1:"8 Bit uLaw",
        2:"4 Bit IMA ADPCM"
}

'''
  This utility will convert the .WAV files in 8 Bit(unsigned), 16 Bit, 32 Bit format
  into 8 Bit Signed PCM, 4 Bit IMA ADPCM and 8 Bit uLaw. It supports the input
  file sampling frequency upto 48MHz.
'''

class Audio_Conv:
    def __init__(self):
        self.infile = ''
        self.outfile_binh = ''
        self.outfile_bin = ''
        self.outfile_raw = ''
        self.outfile_rawh = ''

    def save_wave(self,wave_filename,output_format,frames):
        wavefile = wave.open(wave_filename,"wb")
        wavefile.setnchannels(1)
        wavefile.setsampwidth(1) #8 bits PCM
        wavefile.setframerate(self.infile.getframerate())
        wavefile.setcomptype('NONE','PCM')
        wavefile.writeframes(frames)


    def convert(self,output_format,align_task):
        LINEAR_WIDTH = 2
        infile = self.infile
        sample_width = infile.getsampwidth()

        #Get the input file's raw frame
        raw_frame = infile.readframes(infile.getnframes())

        if 0:
            if sample_width == 1:
                #For .WAV file, it is 8 bit unsigned PCM.
                #Convert it to signed PCM
                raw_frame = audioop.bias(raw_frame,sample_width, -128)

            #Convert the sample frame into 16 Bit (2Byte) signed width frame
            if sample_width != LINEAR_WIDTH:
                pcm16 = audioop.lin2lin(raw_frame, sample_width, LINEAR_WIDTH)
            else:
                pcm16 = raw_frame

        if 1:
            pcm16 = raw_frame


        def padding_8byte_align(data):
            data_output = data

            total_len = (len(data) + 8 ) & ~7

            if total_len > len(data):
                data_output = data.ljust(total_len, '\x00')

            return data_output

        if align_task == True:
            return padding_8byte_align(pcm16)


        if output_format == 0:
            #conversion from 16bit PCM to 8 bit signed PCM
            data = ''
            toggle = 0

            for x in pcm16:
                if toggle:
                    data += x
                toggle = 1 - toggle

            #pad necessary bytes for 8 Bytes aligned
            data = padding_8byte_align(data)

            #Convert from string to array
            da= array.array('B', [(ord(c)) for c in data])

        if output_format == 1:
            #convert sample frames into 8 Bit uLaw
            ulaw_output = audioop.lin2ulaw(pcm16, LINEAR_WIDTH)

            #pad necessary bytes for 8 Bytes aligned
            ulaw_output = padding_8byte_align(ulaw_output)

            #Convert from string to array
            da= array.array('B', [(ord(c)) for c in ulaw_output])


        if output_format ==  2:
            #convert sample frames into 4 bit IMA ADPCM
            (adpcm, _) = audioop.lin2adpcm(pcm16, LINEAR_WIDTH, None)
            #Make the number of elements(4bit) multiple of 8.
            #It will make the output data as 8 bytes aligned.

            #print "adpcm - len()=" + str(len(adpcm))
            #print "adpcm type is " + str(adpcm.__class__)

            #pad necessary bytes for 8 Bytes aligned
            adpcm = padding_8byte_align(adpcm)

            #swap the nibble for FT800:
            #The first sample is from Bit 0-3
            #The second sample is from Bit 4-7
            #The source from audioop is reversed. The first sample is at Bit 4-7.
            if 1:
                print 'swap nibble'
                da = array.array('B', [((ord(c) >> 4) | ((ord(c) & 0x0F) << 4)) for c in adpcm])
            #da = array.array('B', [ord(c) for c in adpcm])

        return da


    def write_data_text(self,data,outfile_h):
        index = 0
        for i in data:
            outfile_h.write(str(i))
            outfile_h.write(",")
            index = index + 1
            if (0 == index % 32):
                outfile_h.write("\n")


    def run(self,argv):
        infile = ''
        output_format = 0

        align_task = False

        try:
            opts,args = getopt.getopt(argv,"hi:f:")
        except getopt.GetoptError:
            print usage
            sys.exit(2)


        for opt,arg in opts:
            if opt  == '-h':
                print usage
                print output_format_help
                sys.exit(0)
            elif opt == '-a':
                print "Padding necessary zero for 8 bytes alignment"
                print "\r\n......"
                align_task = True
            elif opt == '-i':
                infile = arg
            elif opt == '-f':
                output_format = int(arg)
                if output_format not in [0,1,2]:
                    print 'output format is not supported'
                    sys.exit(2)
            else:
                print usage
                sys.exit(2)

        if False == os.path.exists(infile):
            print 'invalid input file' + '\'' + infile + '\''
            print usage
            print output_format_help
            sys.exit(2)

        try:
            self.infile = wave.open(infile,'rb')
        except Exception as e:
            print 'Error to open file ' + infile
            #print e
            print usage
            print output_format_help
            sys.exit(2)


        if self.infile.getnchannels() != 1:
            print "Sorry - .wav file must be mono"
            print "failed to convert audio file"
            print usage
            sys.exit(2)
        else:
            print "sample rate is " + str(self.infile.getframerate()) + "Hz"
            filename = infile.split('.',1)[0]

            dirname = filename + "_" + format_table[output_format]
            if os.path.isdir(dirname):
                print 'Error:'+ dirname + ' directory exists'
                sys.exit(2)


            os.makedirs(dirname)
            os.chdir(dirname)

            self.outfile_raw = open(filename + ".raw",'wb')
            self.outfile_rawh = open(filename + ".rawh",'w')
            self.outfile_bin = open(filename + ".bin",'wb')
            self.outfile_binh = open(filename + ".binh",'w')


        da = self.convert(output_format,align_task)


        da.tofile(self.outfile_raw)
        self.write_data_text(da,self.outfile_rawh)

        deflatedata = zlib.compress(da.tostring())
        self.outfile_bin.write(deflatedata)
        self.write_data_text(deflatedata,self.outfile_binh)


        self.outfile_raw.close()
        self.outfile_rawh.close()
        self.outfile_bin.close()
        self.outfile_binh.close()

        print "convert complete"


if __name__ == '__main__':
    print 'Audio conversion utility for FT800 V0.2'
    Audio_Conv().run(sys.argv[1:])

