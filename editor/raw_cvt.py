import osimport zlibimport sysimport arrayimport randomimport getoptversion = "V0.5-EDITOR"def print_usage():	print 'Usage: raw_cvt -i inputfile -o outname -s seek -l length'class Raw_Conv:	def __init__(self):		self.outfile_raw = ''		self.outfile_bin = ''		self.outfile_h = ''		self.outfile_p = ''	def load_raw_conv(self):		self.infile_raw.seek(self.infile_seek)		data = array.array('B')		data.fromfile(self.infile_raw, self.infile_length)		self.infile_raw.close()		#compress the data into .bin & .binh file		deflatedata = zlib.compress(data)		self.outfile_bin.write(deflatedata)		index = 0		for i in deflatedata:			self.outfile_bin_h.write(str(ord(i)))			self.outfile_bin_h.write(",")			index = index + 1			if (0 == index % (32)):				self.outfile_bin_h.write("\n")		self.outfile_bin.close()		self.outfile_bin_h.close()		#Write the raw data into .raw file		data.tofile(self.outfile_raw)		self.outfile_raw.close()		#write input file details into .h file		writestring = ('file properties: ', 'total size ', len(data))		self.outfile_h.write("/*")		self.outfile_h.write(str(writestring))		self.outfile_h.write("*/ \n")		index = 0		for i in data:			self.outfile_h.write(str(i))			self.outfile_h.write(",")			index = index + 1			if (0 == index % (32)):				self.outfile_h.write("\n")		self.outfile_h.close()		return len(data)	def run(self,argv):		infile = ""		outname = ""		seek = 0		length = 0		try:			opts,args = getopt.getopt(argv,"hi:o:s:l:")		except getopt.GetoptError:			print_usage()			raise Exception("Option error")		for opt,arg in opts:			if opt  == '-h':				print_usage()				return			elif opt == '-i':				infile = arg				if False == os.path.exists(infile):					raise Exception("Invalid input file")			elif opt == '-o':				outname = arg			elif opt == '-s':				seek = int(arg)			elif opt == '-l':				length = int(arg)			else:				print_usage()				raise Exception("Bad option " + opt)		try:			self.infile_size = os.path.getsize(infile)			self.infile_raw  = open(infile, 'rb')		except:			print_usage()			raise Exception('Error opening file ' + '\'' + infile +'\'')		self.infile_seek = seek;		if length > 0:			self.infile_length = length;		else:			self.infile_length = self.infile_size - seek;		filename = infile.split('.',1)[0]		if outname == "":			outname = filename			raise Exception("Outname not specified")		self.filename = outname		self.outfile_raw = open(outname + '.raw' , 'wb')		self.outfile_h   = open(outname + '.rawh', 'w')		self.outfile_bin = open(outname + '.bin' , 'wb')		self.outfile_bin_h = open(outname + '.binh' , 'w')		self.load_raw_conv()		print "convert complete!"if __name__ == '__main__':	print 'raw conversion utility for FT800 ' + version	Raw_Conv().run(sys.argv[1:])