import os, shutil, subprocess, sys

def displayName():
	return "Export HAL(FTDI) Project"

def convertArgs(functionArgs):
	argsMap = {
		"POINTS":"FT_POINTS",
	}
	#delete useless whitespace to avoid error in convertArgs
	functionArgs = functionArgs.replace(" ","")
	functionArgsSplit = functionArgs.split(",")
	for i,v in enumerate(functionArgsSplit):
		if argsMap.has_key(v):
			functionArgsSplit[i] = argsMap[v]

	return ",".join(functionArgsSplit)

functionMap = {
	"CMD_DLSTART" : "Ft_Gpu_CoCmd_Dlstart",
	"CMD_SWAP" : "Ft_Gpu_CoCmd_Swap",
	"CMD_INTERRUPT" : "Ft_Gpu_CoCmd_Interrupt",
	"CMD_GETPOINT" : "Ft_Gpu_CoCmd_GetPtr",
	"CMD_BGCOLOR" : "Ft_Gpu_CoCmd_BgColor",
	"CMD_FGCOLOR" : "Ft_Gpu_CoCmd_FgColor",
	"CMD_GRADIENT" : "Ft_Gpu_CoCmd_Gradient",
	"CMD_TEXT" : "Ft_Gpu_CoCmd_Text",
	"CMD_BUTTON" : "Ft_Gpu_CoCmd_Button",
	"CMD_KEYS" : "Ft_Gpu_CoCmd_Keys",
	"CMD_PROGRESS" : "Ft_Gpu_CoCmd_Progress",
	"CMD_SLIDER" : "Ft_Gpu_CoCmd_Slider",
	"CMD_SCROLLBAR" : "Ft_Gpu_CoCmd_Scrollbar",
	"CMD_TOGGLE" : "Ft_Gpu_CoCmd_Toggle",
	"CMD_GAUGE" : "Ft_Gpu_CoCmd_Gauge",
	"CMD_CLOCK" : "Ft_Gpu_CoCmd_Clock",
	"CMD_CALIBRATE" : "Ft_Gpu_CoCmd_Calibrate",
	"CMD_SPINNER" : "Ft_Gpu_CoCmd_Spinner",
	"CMD_STOP" : "Ft_Gpu_CoCmd_Stop",
	"CMD_MEMCRC" : "Ft_Gpu_CoCmd_MemCrc",
	"CMD_REGREAD" : "Ft_Gpu_CoCmd_RegRead",
	"CMD_MEMWRITE" : "Ft_Gpu_CoCmd_MemWrite",
	"CMD_MEMSET" : "Ft_Gpu_CoCmd_MemSet",
	"CMD_MEMZERO" : "Ft_Gpu_CoCmd_MemZero",
	"CMD_MEMCPY" : "Ft_Gpu_CoCmd_Memcpy",
	"CMD_APPEND" : "Ft_Gpu_CoCmd_Append",
	"CMD_SNAPSHOT" : "Ft_Gpu_CoCmd_Snapshot",
	"CMD_TOUCH_TRANSFORM" : "Ft_Gpu_CoCmd_TouchTransform",
	"CMD_BITMAP_TRANSFORM" : "Ft_Gpu_CoCmd_BitmapTransform",
	"CMD_INFLATE" : "Ft_Gpu_CoCmd_Inflate",
	"CMD_GETPTR" : "Ft_Gpu_CoCmd_GetPtr",
	"CMD_LOADIMAGE" : "Ft_Gpu_CoCmd_LoadImage",
	"CMD_GETPROPS" : "Ft_Gpu_CoCmd_GetProps",
	"CMD_LOADIDENTITY" : "Ft_Gpu_CoCmd_LoadIdentity",
	"CMD_TRANSLATE" : "Ft_Gpu_CoCmd_Translate",
	"CMD_SCALE" : "Ft_Gpu_CoCmd_Scale",
	"CMD_ROTATE" : "Ft_Gpu_CoCmd_Rotate",
	"CMD_SETMATRIX" : "Ft_Gpu_CoCmd_SetMatrix",
	"CMD_SETFONT" : "Ft_Gpu_CoCmd_SetFont",
	"CMD_TRACK" : "Ft_Gpu_CoCmd_Track",
	"CMD_DIAL" : "Ft_Gpu_CoCmd_Dial",
	"CMD_NUMBER" : "Ft_Gpu_CoCmd_Number",
	"CMD_SCREENSAVER" : "Ft_Gpu_CoCmd_ScreenSaver",
	"CMD_SKETCH" : "Ft_Gpu_CoCmd_Sketch",
	"CMD_LOGO" : "Ft_Gpu_CoCmd_Logo",
	"CMD_COLDSTART" : "Ft_Gpu_CoCmd_ColdStart",
	"CMD_GETMATRIX" : "Ft_Gpu_CoCmd_GetMatrix",
	"CMD_GRADCOLOR" : "Ft_Gpu_CoCmd_GradColor"
}
bootupfunc_def = """
ft_void_t Ft_App_BootupConfig()
{
	ft_int16_t FT_DispWidth = 480;
	ft_int16_t FT_DispHeight = 272;
	ft_int16_t FT_DispHCycle =  548;
	ft_int16_t FT_DispHOffset = 43;
	ft_int16_t FT_DispHSync0 = 0;
	ft_int16_t FT_DispHSync1 = 41;
	ft_int16_t FT_DispVCycle = 292;
	ft_int16_t FT_DispVOffset = 12;
	ft_int16_t FT_DispVSync0 = 0;
	ft_int16_t FT_DispVSync1 = 10;
	ft_uint8_t FT_DispPCLK = 5;
	ft_char8_t FT_DispSwizzle = 0;
	ft_char8_t FT_DispPCLKPol = 1;
	ft_char8_t FT_DispCSpread = 0;

	/* Do a power cycle for safer side */
	Ft_Gpu_Hal_Powercycle(phost,FT_TRUE);

	/* Set the clk to external clock */
	Ft_Gpu_HostCommand(phost,FT_GPU_EXTERNAL_OSC);
	Ft_Gpu_Hal_Sleep(10);


	/* Switch PLL output to 48MHz */
	Ft_Gpu_HostCommand(phost,FT_GPU_PLL_48M);
	Ft_Gpu_Hal_Sleep(10);

	/* Do a core reset for safer side */
	Ft_Gpu_HostCommand(phost,FT_GPU_CORE_RESET);

	/* Access address 0 to wake up the FT800 */
	Ft_Gpu_HostCommand(phost,FT_GPU_ACTIVE_M);

	Ft_Gpu_Hal_Wr8(phost, REG_GPIO_DIR,0x80 | Ft_Gpu_Hal_Rd8(phost,REG_GPIO_DIR));
	Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0x080 | Ft_Gpu_Hal_Rd8(phost,REG_GPIO));


	/* Configuration of LCD display */
	#ifdef DISPLAY_QVGA
	/* Values specific to QVGA LCD display */
	FT_DispWidth = 320;
	FT_DispHeight = 240;
	FT_DispHCycle =  408;
	FT_DispHOffset = 70;
	FT_DispHSync0 = 0;
	FT_DispHSync1 = 10;
	FT_DispVCycle = 263;
	FT_DispVOffset = 13;
	FT_DispVSync0 = 0;
	FT_DispVSync1 = 2;
	FT_DispPCLK = 8;
	FT_DispSwizzle = 2;
	FT_DispPCLKPol = 0;
	FT_DispCSpread = 0;
	#endif

	Ft_Gpu_Hal_Wr16(phost, REG_HCYCLE, FT_DispHCycle);
	Ft_Gpu_Hal_Wr16(phost, REG_HOFFSET, FT_DispHOffset);
	Ft_Gpu_Hal_Wr16(phost, REG_HSYNC0, FT_DispHSync0);
	Ft_Gpu_Hal_Wr16(phost, REG_HSYNC1, FT_DispHSync1);
	Ft_Gpu_Hal_Wr16(phost, REG_VCYCLE, FT_DispVCycle);
	Ft_Gpu_Hal_Wr16(phost, REG_VOFFSET, FT_DispVOffset);
	Ft_Gpu_Hal_Wr16(phost, REG_VSYNC0, FT_DispVSync0);
	Ft_Gpu_Hal_Wr16(phost, REG_VSYNC1, FT_DispVSync1);
	Ft_Gpu_Hal_Wr8(phost, REG_SWIZZLE, FT_DispSwizzle);
	Ft_Gpu_Hal_Wr8(phost, REG_PCLK_POL, FT_DispPCLKPol);
	Ft_Gpu_Hal_Wr8(phost, REG_CSPREAD, FT_DispCSpread);
	Ft_Gpu_Hal_Wr8(phost, REG_PCLK,FT_DispPCLK);//after this display is visible on the LCD
	Ft_Gpu_Hal_Wr16(phost, REG_HSIZE, FT_DispWidth);
	Ft_Gpu_Hal_Wr16(phost, REG_VSIZE, FT_DispHeight);


	/* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
	Ft_Gpu_Hal_Wr16(phost, REG_TOUCH_RZTHRESH,1200);
}
"""
WrAppCmdBuf_Func = '''
ft_uint32_t Ft_CmdBuffer_Index = 0;

#ifdef BUFFER_OPTIMIZATION
ft_uint8_t  Ft_CmdBuffer[FT_CMD_FIFO_SIZE];
#endif

ft_void_t Ft_App_WrCoCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{
#ifdef  BUFFER_OPTIMIZATION
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd;
   pBuffcmd =(ft_uint32_t*)&Ft_CmdBuffer[Ft_CmdBuffer_Index];
   *pBuffcmd = cmd;
#endif
#ifdef ARDUINO_PLATFORM
   Ft_Gpu_Hal_WrCmd32(phost,cmd);
#endif

   /* Increment the command index */
   Ft_CmdBuffer_Index += FT_CMD_SIZE;
}

ft_void_t Ft_App_WrCoStr_Buffer(Ft_Gpu_Hal_Context_t *phost,const ft_char8_t *s)
{
#ifdef  BUFFER_OPTIMIZATION
  ft_uint16_t length = 0;
  length = strlen(s) + 1;//last for the null termination

  strcpy(&Ft_CmdBuffer[Ft_CmdBuffer_Index],s);

  /* increment the length and align it by 4 bytes */
  Ft_CmdBuffer_Index += ((length + 3) & ~3);
#endif
}

ft_void_t Ft_App_Flush_Co_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION
   if (Ft_CmdBuffer_Index > 0)
	 Ft_Gpu_Hal_WrCmdBuf(phost,Ft_CmdBuffer,Ft_CmdBuffer_Index);
#endif
   Ft_CmdBuffer_Index = 0;
}


ft_void_t Ft_App_Copy2CmdBuf(Ft_Gpu_Hal_Context_t *phost,ft_uint8_t *src, ft_uint16_t length)
{
#ifdef  BUFFER_OPTIMIZATION
	ft_uint16_t bytes2send = length;
	if (FT_CMD_FIFO_SIZE <= Ft_CmdBuffer_Index)
		Ft_App_Flush_Co_Buffer(phost);


	while(length)
	{
		if (length > FT_CMD_FIFO_SIZE)
			bytes2send = FT_CMD_FIFO_SIZE;
		memcpy(&Ft_CmdBuffer[Ft_CmdBuffer_Index],src,bytes2send);
		src += bytes2send;
		Ft_CmdBuffer_Index +=bytes2send;

		Ft_App_Flush_Co_Buffer(phost);
		length -= bytes2send;
	}
#endif
}
'''

setup_loop_sel = '''
#ifdef MSVC_FT800EMU
void loop()
{

}
#endif
#ifdef MSVC_FT800EMU
void setup()
#endif
#ifdef MSVC_PLATFORM
void main()
#endif
'''

def run(name, document, ram):
	resultText = "<b>EVE HAL Export</b><br>"

	for line in document["displayList"]:
		if not line == "":
			resultText += "<b>Warning</b>: Only support for Coprocessor, ignoring Display List.<br>"
			break
	outDir = name + "_FTEVE_HAL"
	try:
		os.makedirs(outDir)
	except:
		#print "Dir already exists"
		pass
	outName = outDir + "/" + name + "_FTEVE_HAL.c" # os.path.basename(os.getcwd()) + ".ino"
	if os.path.isfile(outName):
		os.remove(outName)
	f = open(outName, "w")

	f.write("#include <FT_Platform.h>\n")

	for content in document["content"]:
		if content["memoryLoaded"]:
			memoryAddress = "RAM_" + content["destName"].replace("/", "_").upper();
			f.write("\n")
			f.write("#define " + memoryAddress + " " + str(content["memoryAddress"]) + "\n")
		if content["dataEmbedded"]:
			contentName = content["destName"].replace("/", "_")
			headerName = content["destName"];
			if content["dataCompressed"]:
				headerName += ".binh"
			else:
				headerName += ".rawh"
			targetName = contentName + ".h"
			targetPath = outDir + "/" + targetName
			if os.path.isfile(targetPath):
				os.remove(targetPath)
			shutil.copy(headerName, targetPath)
			content["FTEVE_Name"] = contentName
			f.write("\n")
			charType = "ft_uchar8_t"
			#charType = "char" # For older Linux distro
			f.write("static " + charType + " " + contentName + "[] = {\n")
			f.write("#\tinclude \"" + targetName + "\"\n")
			f.write("};\n")
	f.write("\n")
	f.write("Ft_Gpu_HalInit_t halinit;\n")
	f.write("Ft_Gpu_Hal_Context_t host,*phost=&host;\n\n")
	f.write(bootupfunc_def)
	f.write(WrAppCmdBuf_Func)
	f.write(setup_loop_sel)
	f.write("{\n")
	f.write("\tFt_Gpu_Hal_Init(&halinit);\n")
	f.write("\thost.hal_config.spi_clockrate_khz = 15000; //in KHz;\n")
	f.write("\tFt_Gpu_Hal_Open(phost);\n")
	f.write("\tFt_App_BootupConfig();\n")

	for content in document["content"]:
		if content["memoryLoaded"]:
			memoryAddress = "RAM_" + content["destName"].replace("/", "_").upper();
			if content["dataEmbedded"]:
				contentName = content["FTEVE_Name"]
				if content["converter"] == "RawJpeg":
					f.write("\tFt_Gpu_CoCmd_LoadImage(phost," + memoryAddress + ");\n")
				if content["dataCompressed"]:
					f.write("\tFt_Gpu_CoCmd_Inflate(phost," + memoryAddress + ");\n")
				else:
					f.write("\tFt_Gpu_CoCmd_MemWrite(phost," + memoryAddress + ", sizeof(" + contentName + "));\n")
				f.write("\tFt_App_Copy2CmdBuf(phost," + contentName + ", sizeof(" + contentName + "));\n")
				f.write("\tFt_App_Flush_Co_Buffer(phost);\n")

	f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")

	# optional to split off bitmaps
	#handlesFound = False
	#for line in document["coprocessor"]:
	#	if not line == "":
	#		splitlinea = line.split('(', 1)
	#		splitlineb = splitlinea[1].split(')',1)
	#		functionName = splitlinea[0]
	#		if functionMap.has_key(functionName):
	#			functionName = functionMap[functionName]
	#
	#		if functionName == "BITMAP_HANDLE" or functionName == "BITMAP_SOURCE" or functionName == "BITMAP_LAYOUT" or functionName == "BITMAP_SIZE":
	#			functionArgs = convertArgs(splitlineb[0])
	#			newline = "\tFt_App_WrCoCmd_Buffer(phost," + functionName + "(" + functionArgs + "));\n"
	#			f.write(newline)
	#			handlesFound = True;
	#		else:
	#			break
	#if handlesFound:
	#	f.write("\tFt_App_WrCoCmd_Buffer(phost,DISPLAY());\n")
	#	f.write("\tFt_Gpu_CoCmd_Swap(phost);\n")
	#	f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")


	jumpOffset = 0
	clearFound = False
	for line in document["coprocessor"]:
		if not line == "":
			splitlinea = line.split('(', 1)
			functionName = splitlinea[0]
			if functionMap.has_key(functionName):
				functionName = functionMap[functionName]
			if functionName == "Clear":
				clearFound = True
				break

	if not clearFound:
		jumpOffset = jumpOffset + 1
		f.write("\tFt_App_WrCoCmd_Buffer(phost,CLEAR(1, 1, 1));\n")
	#skippedBitmaps = False
	for line in document["coprocessor"]:
		if not line == "":
			splitlinea = line.split('(',1)
			splitlineb = splitlinea[1].split(')',1)
			functionName = splitlinea[0]

			coprocessor_cmd = False
			if functionMap.has_key(functionName):
				functionName = functionMap[functionName]
				coprocessor_cmd = True
			#if not skippedBitmaps:
			#	if functionName == "BITMAP_HANDLE" or functionName == "BITMAP_SOURCE" or functionName == "BITMAP_LAYOUT" or functionName == "BITMAP_SIZE":
			#		jumpOffset = jumpOffset - 1
			#		continue
			#	else:
			#		skippedBitmaps = True

			functionArgs = convertArgs(splitlineb[0])

			if functionName == "JUMP" or functionName == "CALL":
				functionArgsSplit = eval("[ " + functionArgs + " ]")
				functionArgs = str(functionArgsSplit[0] + jumpOffset)

			if functionName == "Ft_Gpu_CoCmd_Gradient":
				functionArgsSplit = eval(functionArgs)
				color0 = str((functionArgsSplit[2] * 256 + functionArgsSplit[3])*256 + functionArgsSplit[4])
				color1 = str((functionArgsSplit[7] * 256 + functionArgsSplit[8])*256 + functionArgsSplit[9])
				functionArgs = str(functionArgsSplit[0]) + "," + str(functionArgsSplit[1]) + ","
				functionArgs += color0 + ","
				functionArgs += str(functionArgsSplit[5]) + "," + str(functionArgsSplit[6]) + ","
				functionArgs += color1

			if functionName == "Ft_Gpu_CoCmd_BgColor" or functionName == "Ft_Gpu_CoCmd_FgColor" or functionName == "Ft_Gpu_CoCmd_GradColor":
				functionArgsSplit = eval("[ " + functionArgs + " ]")
				functionArgs = str(((functionArgsSplit[0] * 256) + functionArgsSplit[1]) * 256 + functionArgsSplit[2])
			if coprocessor_cmd:
				newline = "\t" + functionName + "(phost," + functionArgs + ");\n"
			else:
				newline = "\tFt_App_WrCoCmd_Buffer(phost," + functionName + "(" + functionArgs + "));\n"


			f.write(newline)
		else:
			if skippedBitmaps:
				f.write("\t\n")
	f.write("\tFt_App_WrCoCmd_Buffer(phost,DISPLAY());\n")
	f.write("\tFt_Gpu_CoCmd_Swap(phost);\n")
	f.write("\tFt_App_Flush_Co_Buffer(phost);\n")
	f.write("\tFt_Gpu_Hal_Close(phost);\n")
	f.write("\tFt_Gpu_Hal_DeInit();\n")
	f.write("}\n")
	f.write("\n")
	f.write("/* end of file */\n")

	f.close()
	resultText += "<b>Output</b>: " + outName
	if sys.platform.startswith('darwin'):
		subprocess.call(('open', outName))
	elif os.name == 'nt':
		os.startfile((os.getcwd() + "/" + outName).replace("/", "\\"))
	elif os.name == 'posix':
		subprocess.call(('xdg-open', outName))
	#print resultText
	return resultText
