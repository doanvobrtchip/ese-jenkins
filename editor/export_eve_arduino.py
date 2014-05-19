import os, shutil, subprocess, sys

def displayName():
	return "Export EVE Arduino Project"
	
def convertArgs(functionArgs):
	argsMap = {
		"ARGB1555":"FT_ARGB1555",
		"L1":"FT_L1",
		"L4":"FT_L4",
		"L8":"FT_L8",
		"RGB332":"FT_RGB332",
		"ARGB2":"FT_ARGB2",
		"ARGB4":"FT_ARGB4",
		"RGB565":"FT_RGB565",
		"PALETTED":"FT_PALETTED",
		"TEXT8X8":"FT_TEXT8X8",
		"TEXTVGA":"FT_TEXTVGA",
		"BARGRAPH":"FT_BARGRAPH",
		
		"NEAREST":"FT_NEAREST",
		"BILINEAR":"FT_BILINEAR",
		
		"BORDER":"FT_BORDER",
		"REPEAT":"FT_REPEAT",
		
		"NEVER":"FT_NEVER",
		"LESS":"FT_LESS",
		"LEQUAL":"FT_LEQUAL",
		"GREATER":"FT_GREATER",
		"GEQUAL":"FT_GEQUAL",
		"EQUAL":"FT_EQUAL",
		"ALWAYS":"FT_ALWAYS",
		
		"KEEP":"FT_KEEP",
		"REPLACE":"FT_REPLACE",
		"INCR":"FT_INCR",
		"DECR":"FT_DECR",
		"INVERT":"FT_INVERT",
		
		"ZERO":"FT_ZERO",
		"ONE":"FT_ONE",
		"SRC_ALPHA":"FT_SRC_ALPHA",
		"DST_ALPHA":"FT_DST_ALPHA",
		"ONE_MINUS_SRC_ALPHA":"FT_ONE_MINUS_SRC_ALPHA",
		"ONE_MINUS_DST_ALPHA":"FT_ONE_MINUS_DST_ALPHA",
		
		"BITMAPS":"FT_BITMAPS",
		"POINTS":"FT_POINTS",
		"LINES":"FT_LINES",
		"LINE_STRIP":"FT_LINE_STRIP",
		"EDGE_STRIP_R":"FT_EDGE_STRIP_R",
		"EDGE_STRIP_L":"FT_EDGE_STRIP_L",
		"EDGE_STRIP_A":"FT_EDGE_STRIP_A",		
		"EDGE_STRIP_B":"FT_EDGE_STRIP_B",
		"RECTS":"FT_RECTS",
		
		"OPT_MONO":"FT_OPT_MONO",
		"OPT_NODL":"FT_OPT_NODL",
		"OPT_FLAT":"FT_OPT_FLAT",
		"OPT_CENTERX":"FT_OPT_CENTERX",
		"OPT_CENTERY":"FT_OPT_CENTERY",
		"OPT_CENTER":"FT_OPT_CENTER",		
		"OPT_NOBACK":"FT_OPT_NOBACK",
		"OPT_NOTICKS":"FT_OPT_NOTICKS",
		"OPT_NOHM":"FT_OPT_NOHM",
		"OPT_NOPOINTER":"FT_OPT_NOPOINTER",
		"OPT_NOSECS":"FT_OPT_NOSECS",
		"OPT_NOHANDS":"FT_OPT_NOHANDS",
		"OPT_RIGHTX":"FT_OPT_RIGHTX",
		"OPT_SIGNED":"FT_OPT_SIGNED",
	}
	#delete useless whitespace to avoid error in convertArgs
	functionArgs = functionArgs.replace(" ","")
	functionArgsSplit = functionArgs.split(",")
	for i,v in enumerate(functionArgsSplit):
		if argsMap.has_key(v):
			functionArgsSplit[i] = argsMap[v]		

	return ",".join(functionArgsSplit)	

def run(name, document, ram):
	resultText = "<b>EVE Arduino Export</b><br>"
	functionMap = {
		"VERTEX2II" : "Vertex2ii",
		"VERTEX2F" : "Vertex2f",
		"DISPLAY" : "Display",
		"BITMAP_SOURCE" : "BitmapSource",
		"CLEAR_COLOR_RGB" : "ClearColorRGB",
		"TAG" : "Tag",
		"COLOR_RGB" : "ColorRGB",
		"BITMAP_HANDLE" : "BitmapHandle",
		"CELL" : "Cell",
		"BITMAP_LAYOUT" : "BitmapLayout",
		"BITMAP_SIZE" : "BitmapSize",
		"ALPHA_FUNC" : "AlphaFunc",
		"STENCIL_FUNC" : "StencilFunc",
		"BLEND_FUNC" : "BlendFunc",
		"STENCIL_OP" : "StencilOp",
		"POINT_SIZE" : "PointSize",
		"LINE_WIDTH" : "LineWidth",
		"CLEAR_COLOR_A" : "ClearColorA",
		"COLOR_A" : "ColorA",
		"CLEAR_STENCIL" : "ClearStencil",
		"CLEAR_TAG" : "ClearTag",
		"STENCIL_MASK" : "StencilMask",
		"TAG_MASK" : "TagMask",
		"BITMAP_TRANSFORM_A" : "BitmapTransformA",
		"BITMAP_TRANSFORM_B" : "BitmapTransformB",
		"BITMAP_TRANSFORM_C" : "BitmapTransformC",
		"BITMAP_TRANSFORM_D" : "BitmapTransformD",
		"BITMAP_TRANSFORM_E" : "BitmapTransformE",
		"BITMAP_TRANSFORM_F" : "BitmapTransformF",
		"SCISSOR_XY" : "ScissorXY",
		"SCISSOR_SIZE" : "ScissorSize",
		"CALL" : "Call",
		"JUMP" : "Jump",
		"BEGIN" : "Begin",
		"COLOR_MASK" : "ColorMask",
		"END" : "End",
		"SAVE_CONTEXT" : "SaveContext",
		"RESTORE_CONTEXT" : "RestoreContext",
		"RETURN" : "Return",
		"MACRO" : "Macro",
		"CLEAR" : "Clear",
		"CMD_DLSTART" : "Cmd_DLStart",
		"CMD_SWAP" : "Cmd_Swap",
		"CMD_INTERRUPT" : "Cmd_Interrupt",
		"CMD_GETPOINT" : "Cmd_GetPtr",
		"CMD_BGCOLOR" : "Cmd_BGColor",
		"CMD_FGCOLOR" : "Cmd_FGColor",
		"CMD_GRADIENT" : "Cmd_Gradient",
		"CMD_TEXT" : "Cmd_Text",
		"CMD_BUTTON" : "Cmd_Button",
		"CMD_KEYS" : "Cmd_Keys",
		"CMD_PROGRESS" : "Cmd_Progress",
		"CMD_SLIDER" : "Cmd_Slider",
		"CMD_SCROLLBAR" : "Cmd_Scrollbar",
		"CMD_TOGGLE" : "Cmd_Toggle",
		"CMD_GAUGE" : "Cmd_Gauge",
		"CMD_CLOCK" : "Cmd_Clock",
		"CMD_CALIBRATE" : "Cmd_Calibrate",
		"CMD_SPINNER" : "Cmd_Spinner",
		"CMD_STOP" : "Cmd_Stop",
		"CMD_MEMCRC" : "Cmd_Memcrc",
		"CMD_REGREAD" : "Cmd_RegRead",
		"CMD_MEMWRITE" : "Cmd_Memwrite",
		"CMD_MEMSET" : "Cmd_Memset",
		"CMD_MEMZERO" : "Cmd_Memzero",
		"CMD_MEMCPY" : "Cmd_Memcpy",
		"CMD_APPEND" : "cmd_append",
		"CMD_SNAPSHOT" : "Cmd_Snapshot",
		"CMD_INFLATE" : "Cmd_Inflate",
		"CMD_GETPTR" : "Cmd_GetPtr",
		"CMD_LOADIMAGE" : "Cmd_LoadImage",
		"CMD_GETPROPS" : "Cmd_GetProps",
		"CMD_LOADIDENTITY" : "Cmd_LoadIdentity",
		"CMD_TRANSLATE" : "Cmd_Translate",
		"CMD_SCALE" : "Cmd_Scale",
		"CMD_ROTATE" : "Cmd_Rotate",
		"CMD_SETMATRIX" : "Cmd_SetMatrix",
		"CMD_SETFONT" : "Cmd_SetFont",
		"CMD_TRACK" : "Cmd_Track",
		"CMD_DIAL" : "Cmd_Dial",
		"CMD_NUMBER" : "Cmd_Number",
		"CMD_SCREENSAVER" : "Cmd_ScreenSaver",
		"CMD_SKETCH" : "Cmd_Sketch",
		"CMD_LOGO" : "Cmd_Logo",
		"CMD_COLDSTART" : "Cmd_ColdStart",
		"CMD_GETMATRIX" : "Cmd_GetMatrix",
		"CMD_GRADCOLOR" : "Cmd_GradColor"
	}

	for line in document["displayList"]:
		if not line == "":
			resultText += "<b>Warning</b>: Only support for Coprocessor, ignoring Display List.<br>"
			break
	outDir = name + "_FTEVE"
	try:
		os.makedirs(outDir)
	except:
		#print "Dir already exists"
		pass
	outName = outDir + "/" + name + "_FTEVE.ino" # os.path.basename(os.getcwd()) + ".ino"
	if os.path.isfile(outName):
		os.remove(outName)
	f = open(outName, "w")
	f.write("#include <EEPROM.h>\n")
	f.write("#include <SPI.h>\n")
	f.write("#include <Wire.h>\n")
	f.write("#include <FT_VM800P43_50.h>\n")

    
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
			charType = "prog_uchar"
			#charType = "char" # For older Linux distro
			f.write("static PROGMEM " + charType + " " + contentName + "[] = {\n")
			f.write("#\tinclude \"" + targetName + "\"\n")
			f.write("};\n")
	f.write("\n")
	f.write("FT800IMPL_SPI FTImpl(FT_CS_PIN,FT_PDN_PIN,FT_INT_PIN);\n")	
	f.write("void setup()\n")
	f.write("{\n")
	f.write("\tFTImpl.Init(FT_DISPLAY_RESOLUTION);\n")
	f.write("\tFTImpl.SetDisplayEnablePin(FT_DISPENABLE_PIN);\n")
	f.write("\tFTImpl.SetAudioEnablePin(FT_AUDIOENABLE_PIN);\n")
	f.write("\tFTImpl.DisplayOn();\n")
	f.write("\tFTImpl.AudioOn();\n")
	for content in document["content"]:
		if content["memoryLoaded"]:
			memoryAddress = "RAM_" + content["destName"].replace("/", "_").upper();
			if content["dataEmbedded"]:
				contentName = content["FTEVE_Name"]
				if content["converter"] == "RawJpeg":
					f.write("\tFTImpl.Cmd_LoadImage(" + memoryAddress + ");\n")
				if content["dataCompressed"]:
					f.write("\tFTImpl.Cmd_Inflate(" + memoryAddress + ");\n")
				else:
					f.write("\tFTImpl.Cmd_Memwrite(" + memoryAddress + ", sizeof(" + contentName + "));\n")
				f.write("\tFTImpl.WriteCmdfromflash(" + contentName + ", sizeof(" + contentName + "));\n")
				f.write("\tFTImpl.Finish();\n")	
				
	f.write("\tFTImpl.DLStart();\n")			
	for line in document["coprocessor"]:
		if not line == "":
			splitlinea = line.split('(', 1)
			splitlineb = splitlinea[1].split(')',1)
			functionName = splitlinea[0]
			functionName = functionMap[functionName]
				
			if functionName == "BitmapHandle" or functionName == "BitmapSource" or functionName == "BitmapLayout" or functionName == "BitmapSize":
				functionArgs = convertArgs(splitlineb[0])
				newline = "\tFTImpl." + functionName + "(" + functionArgs + ");\n"
				f.write(newline)
			else:
				break
	f.write("\tFTImpl.DLEnd();\n")
	f.write("\tFTImpl.Finish();\n")				
	f.write("}\n")
	f.write("\n")
	f.write("void loop()\n")
	f.write("{\n")
	clearFound = False
	for line in document["coprocessor"]:
		if not line == "":
			splitlinea = line.split('(', 1)
			functionName = splitlinea[0]
			functionName = functionMap[functionName]
			if functionName == "Clear":
				clearFound = True
				break
				
	f.write("\tFTImpl.DLStart();\n")
	if not clearFound:
		f.write("\tFTImpl.Clear(1, 1, 1);\n")
	skippedBitmaps = False
	for line in document["coprocessor"]:
		if not line == "":
			splitlinea = line.split('(',1)		
			splitlineb = splitlinea[1].split(')',1)
			functionName = splitlinea[0]
			functionName = functionMap[functionName]
			if not skippedBitmaps:
				if functionName == "BitmapHandle" or functionName == "BitmapSource" or functionName == "BitmapLayout" or functionName == "BitmapSize":
					continue
				else:
					skippedBitmaps = True
			
			functionArgs = convertArgs(splitlineb[0])
			
			if functionName == "Cmd_FGColor" or functionName == "Cmd_BGColor" or functionName == "Cmd_GradColor":
				functionArgsSplit = eval("[ " + functionArgs + " ]")
				functionArgs = str(((functionArgsSplit[0] * 256) + functionArgsSplit[1]) * 256 + functionArgsSplit[2])
				
			if functionName == "Cmd_Gradient":
				functionArgsSplit = eval(functionArgs)
				color0 = str((functionArgsSplit[3] * 256 + functionArgsSplit[4])*256 + functionArgsSplit[5])
				color1 = str((functionArgsSplit[9] * 256 + functionArgsSplit[10])*256 + functionArgsSplit[11])
				functionArgs = str(functionArgsSplit[0]) + "," + str(functionArgsSplit[1]) + ","
				functionArgs += color0 + ","
				functionArgs += str(functionArgsSplit[6]) + "," + str(functionArgsSplit[7]) + ","
				functionArgs += color1				
				
			newline = "\tFTImpl." + functionName + "(" + functionArgs + ");\n"
			f.write(newline)
		else:
			if skippedBitmaps:
				f.write("\t\n")
	f.write("\tFTImpl.DLEnd();\n")
	f.write("\tFTImpl.Finish();\n")
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
