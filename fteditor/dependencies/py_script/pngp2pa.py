import os
import sys
import getopt
import array
import png
import zlib
import subprocess
import traceback

'''
V0.3-editor: Adjustments for embedding in editor
V0.3: Added PALETTED8, PALETTED565, and PALETTED4444 support for FT81X
V0.2: Add file property for rawh file, change the line after 32 char printed in binh file
V0.1: first version to extract palette and index from PNG8
'''
version = "V0.3-editor"
help_string = ""
def print_usage():
	return

vc_PALETTED = 0
vc_PALETTED8 = 1
vc_PALETTED565 = 2
vc_PALETTED4444 = 3

format_table = {
	vc_PALETTED : "PALETTED",
	vc_PALETTED8 : "PALETTED8",
	vc_PALETTED565 : "PALETTED565",
	vc_PALETTED4444 : "PALETTED4444"}

def run(argv):
	infile_name = ""
	outfile_name = ""
	outfile_format = vc_PALETTED
	pngquant_exec = ""
	infile = None
	try:
		opts,args = getopt.getopt(argv,"hi:o:f:q:")
	except getopt.GetoptError:
		#print_usage()
		raise Exception("Option error")

	for opt,arg in opts:
		if opt  == '-h':
			#print_usage()
			raise Exception("Option error")
		elif opt == '-i':
			infile_name = arg
			if False == os.path.exists(infile_name):
				raise Exception('invalid input file')
		elif opt == '-o':
			outfile_name = arg
		elif opt == '-q':
			pngquant_exec = arg
		elif opt == '-f':
			outfile_format = int(arg)
			if outfile_format not in [0,1,2,3]:
				raise Exception("Invalid format " + arg)
		else:
			#print_usage()
			raise Exception("Bad option " + opt)

	quantfile_name = outfile_name + "_converted-fs8.png"
	if os.path.exists(quantfile_name):
		raise Exception('Quantized file already exists')
	subprocess.call([ pngquant_exec, "256", infile_name ])

	try:
		infile = png.Reader(filename = quantfile_name)
	except:
		#print_usage()
		#traceback.print_exc()
		raise Exception('Error opening file ' + '\'' + quantfile_name +'\'')

	infile_Direct = infile.asDirect()

	try:
		infile_reader = infile.read_flat()
	except:
		raise Exception('Error to read the png file,exit')

	try:
		if (infile_reader[3]['palette'] == None) or (infile_reader[3]['bitdepth'] != 8):
			raise Exception("Please convert PNG to palette mode by using pngquant utility first")
	except:
		#traceback.print_exc()
		raise Exception('Issue with PNG reading')

	img_indexdata =  infile_reader[2]
	def write_index(img_data):
		outfile_raw = open(outfile_name + '.raw' , 'wb')
		outfile_h   = open(outfile_name + '.rawh', 'w')
		outfile_bin = open(outfile_name + '.bin' , 'wb')
		outfile_bin_h = open(outfile_name + '.binh' , 'w')
		imdata = []
		index  = 0
		stride = 0

		if outfile_format == vc_PALETTED:
			stride = infile_Direct[0]
		elif outfile_format == vc_PALETTED565 or outfile_format == vc_PALETTED4444 or outfile_format == vc_PALETTED8:
			stride = infile_Direct[0]
		
		writestring = ('file properties: ', 'resolution ', infile_Direct[0], 'x', infile_Direct[1], 'format ', 
			format_table[outfile_format], 'stride ', stride, ' total size ', infile_Direct[0]*infile_Direct[1])

		outfile_h.write("/*");
		outfile_h.write(str(writestring));
		outfile_h.write("*/\n");
		for i in img_data:
			outfile_raw.write(chr(i))
			imdata.append(i)

			outfile_h.write(str(i))
			outfile_h.write(",")
			index = index + 1
			if (0 == index % 32):
				outfile_h.write("\n")


		outfile_raw.close()
		outfile_h.close()

		deflatdata = zlib.compress(array.array('B', imdata))
		outfile_bin.write(deflatdata)
		outfile_bin.close()
		index  = 0
		for i in deflatdata:
			outfile_bin_h.write("{}".format(i))
			outfile_bin_h.write(",")
			index = index  + 1
			if (0 == index % 32):
				outfile_bin_h.write("\n")
		outfile_bin_h.close()

	write_index(img_indexdata)


	lut = infile_reader[3]['palette']
	def write_lutfile(lut):
		outfile_raw = open(outfile_name + '.lut.raw' , 'wb')
		outfile_h   = open(outfile_name + '.lut.rawh', 'w')
		outfile_bin = open(outfile_name + '.lut.bin' , 'wb')
		outfile_bin_h = open(outfile_name + '.lut.binh' , 'w')
		imdata = []
		index  = 0

		if (len(lut[0]) == 3):		
			for (r,g,b) in lut:
				if(outfile_format == vc_PALETTED or outfile_format == vc_PALETTED8):
					outfile_raw.write(chr(b))
					outfile_raw.write(chr(g))
					outfile_raw.write(chr(r))
					outfile_raw.write(chr(255))

					imdata.append(b)
					imdata.append(g)
					imdata.append(r)
					imdata.append(255)

					outfile_h.write(str(b))
					outfile_h.write(",")
					outfile_h.write(str(g))
					outfile_h.write(",")
					outfile_h.write(str(r))
					outfile_h.write(",")
					outfile_h.write(str(255))
					outfile_h.write(",")

				elif(outfile_format == vc_PALETTED565):
					rgb565encoding = (((r * 31) / 255) << 11) | (((g * 63) / 255) << 5) | (((b * 31) / 255) & 31)
					rgb565upper = rgb565encoding & 255
					rgb565lower = (rgb565encoding >> 8) & 255
					outfile_raw.write(chr(rgb565upper))
					outfile_raw.write(chr(rgb565lower))

					imdata.append(rgb565upper)
					imdata.append(rgb565lower)

					outfile_h.write(str(rgb565upper))
					outfile_h.write(",")
					outfile_h.write(str(rgb565lower))
					outfile_h.write(",")

				elif (outfile_format == vc_PALETTED4444):
					argb4444encoding = (((255 * 15) / 255) << 12) | (((r * 15) / 255) << 8) | (((g * 15) / 255) << 4) | (((b * 15) / 255) & 15)
					argb4444upper = argb4444encoding & 255
					argb4444lower = (argb4444encoding >> 8) & 255
					outfile_raw.write(chr(argb4444upper))
					outfile_raw.write(chr(argb4444lower))

					imdata.append(argb4444upper)
					imdata.append(argb4444lower)

					outfile_h.write(str(argb4444upper))
					outfile_h.write(",")
					outfile_h.write(str(argb4444lower))
					outfile_h.write(",")
				index = index + 1		   
				if (0 == index % 32):
					outfile_h.write("\n") 
		else:
			for (r,g,b,a) in lut:
				if(outfile_format == vc_PALETTED or outfile_format == vc_PALETTED8):
					outfile_raw.write(chr(b))
					outfile_raw.write(chr(g))
					outfile_raw.write(chr(r))
					outfile_raw.write(chr(a))

					imdata.append(b)
					imdata.append(g)
					imdata.append(r)
					imdata.append(a)

					outfile_h.write(str(b))
					outfile_h.write(",")
					outfile_h.write(str(g))
					outfile_h.write(",")
					outfile_h.write(str(r))
					outfile_h.write(",")
					outfile_h.write(str(a))
					outfile_h.write(",")
				elif(outfile_format == vc_PALETTED565):
					rgb565encoding = (((r * 31) / 255) << 11) | (((g * 63) / 255) << 5) | (((b * 31) / 255) & 31)
					rgb565upper = rgb565encoding & 255
					rgb565lower = (rgb565encoding >> 8) & 255
					outfile_raw.write(chr(rgb565upper))
					outfile_raw.write(chr(rgb565lower))

					imdata.append(rgb565upper)
					imdata.append(rgb565lower)

					outfile_h.write(str(rgb565upper))
					outfile_h.write(",")
					outfile_h.write(str(rgb565lower))
					outfile_h.write(",")

				elif (outfile_format == vc_PALETTED4444):
					argb4444encoding = (((a * 15) / 255) << 12) | (((r * 15) / 255) << 8) | (((g * 15) / 255) << 4) | (((b * 15) / 255) & 15)
					argb4444upper = argb4444encoding & 255
					argb4444lower = (argb4444encoding >> 8) & 255
					outfile_raw.write(chr(argb4444upper))
					outfile_raw.write(chr(argb4444lower))

					imdata.append(argb4444upper)
					imdata.append(argb4444lower)

					outfile_h.write(str(argb4444upper))
					outfile_h.write(",")
					outfile_h.write(str(argb4444lower))
					outfile_h.write(",")

				index = index + 1		   
				if (0 == index % 32):
					outfile_h.write("\n")

		outfile_raw.close()
		outfile_h.close()

		deflatdata = zlib.compress(array.array('B', imdata))
		outfile_bin.write(deflatdata)
		outfile_bin.close()
		index  = 0
		for i in deflatdata:
			outfile_bin_h.write("{}".format(i))
			outfile_bin_h.write(",")
			index = index + 1
			if (0 == index % 32):
				outfile_bin_h.write("\n")
		outfile_bin_h.close()

	write_lutfile(lut)
	#print "convert complete!"

if __name__ == '__main__':
	#print 'PNG8 to Palette conversion utility for FT800 ' + version
	run(sys.argv[1:])


