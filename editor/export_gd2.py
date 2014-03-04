import os, shutil, subprocess, sys

def run(name, document):
	resultText = "<b>Gameduino 2 Export</b><br>"
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
		"CMD_DLSTART" : "cmd_dlstart",
		"CMD_SWAP" : "cmd_swap",
		"CMD_INTERRUPT" : "cmd_interrupt",
		"CMD_CRC" : "cmd_crc",
		"CMD_HAMMERAUX" : "cmd_hammeraux",
		"CMD_MARCH" : "cmd_march",
		"CMD_IDCT" : "cmd_idct",
		"CMD_EXECUTE" : "cmd_execute",
		"CMD_GETPOINT" : "cmd_getpoint",
		"CMD_BGCOLOR" : "cmd_bgcolor",
		"CMD_FGCOLOR" : "cmd_fgcolor",
		"CMD_GRADIENT" : "cmd_gradient",
		"CMD_TEXT" : "cmd_text",
		"CMD_BUTTON" : "cmd_button",
		"CMD_KEYS" : "cmd_keys",
		"CMD_PROGRESS" : "cmd_progress",
		"CMD_SLIDER" : "cmd_slider",
		"CMD_SCROLLBAR" : "cmd_scrollbar",
		"CMD_TOGGLE" : "cmd_toggle",
		"CMD_GAUGE" : "cmd_gauge",
		"CMD_CLOCK" : "cmd_clock",
		"CMD_CALIBRATE" : "cmd_calibrate",
		"CMD_SPINNER" : "cmd_spinner",
		"CMD_STOP" : "cmd_stop",
		"CMD_MEMCRC" : "cmd_memcrc",
		"CMD_REGREAD" : "cmd_regread",
		"CMD_MEMWRITE" : "cmd_memwrite",
		"CMD_MEMSET" : "cmd_memset",
		"CMD_MEMZERO" : "cmd_memzero",
		"CMD_MEMCPY" : "cmd_memcpy",
		"CMD_APPEND" : "cmd_append",
		"CMD_SNAPSHOT" : "cmd_snapshot",
		"CMD_TOUCH_TRANSFORM" : "cmd_touch_transform",
		"CMD_BITMAP_TRANSFORM" : "cmd_bitmap_transform",
		"CMD_INFLATE" : "cmd_inflate",
		"CMD_GETPTR" : "cmd_getptr",
		"CMD_LOADIMAGE" : "cmd_loadimage",
		"CMD_GETPROPS" : "cmd_getprops",
		"CMD_LOADIDENTITY" : "cmd_loadidentity",
		"CMD_TRANSLATE" : "cmd_translate",
		"CMD_SCALE" : "cmd_scale",
		"CMD_ROTATE" : "cmd_rotate",
		"CMD_SETMATRIX" : "cmd_setmatrix",
		"CMD_SETFONT" : "cmd_setfont",
		"CMD_TRACK" : "cmd_track",
		"CMD_DIAL" : "cmd_dial",
		"CMD_NUMBER" : "cmd_number",
		"CMD_SCREENSAVER" : "cmd_screensaver",
		"CMD_SKETCH" : "cmd_sketch",
		"CMD_LOGO" : "cmd_logo",
		"CMD_COLDSTART" : "cmd_coldstart",
		"CMD_GETMATRIX" : "cmd_getmatrix",
		"CMD_GRADCOLOR" : "cmd_gradcolor"
	}
	formatList = [
		"ARGB1555",
		"L1",
		"L4",
		"L8",
		"RGB332",
		"ARGB2",
		"ARGB4",
		"RGB565",
		"PALETTED"
	]
	filterList = [
		"NEAREST",
		"BILINEAR"
	]
	wrapList = [
		"BORDER",
		"REPEAT"
	]
	for line in document["displayList"]:
		if not line == "":
			resultText += "<b>Warning</b>: Only support for Coprocessor, ignoring Display List.<br>"
			break
	outDir = name + "_gd2"
	try:
		os.makedirs(outDir)
	except:
		print "Dir already exists"
	outName = outDir + "/" + name + "_gd2.ino" # os.path.basename(os.getcwd()) + ".ino"
	if os.path.isfile(outName):
		os.remove(outName)
	f = open(outName, "w")
	f.write("#include <EEPROM.h>\n")
	f.write("#include <SPI.h>\n")
	f.write("#include <GD2.h>\n")
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
			content["gd2Name"] = contentName
			f.write("\n")
			charType = "prog_uchar"
			#charType = "char" # For older Linux distro
			f.write("static const PROGMEM " + charType + " " + contentName + "[] = {\n")
			f.write("#\tinclude \"" + targetName + "\"\n")
			f.write("};\n")
	f.write("\n")
	f.write("void setup()\n")
	f.write("{\n")
	f.write("\tGD.begin();\n")
	for content in document["content"]:
		if content["memoryLoaded"]:
			memoryAddress = "RAM_" + content["destName"].replace("/", "_").upper();
			if content["dataEmbedded"]:
				contentName = content["gd2Name"]
				if content["converter"] == "RawJpeg":
					f.write("\tGD.cmd_loadimage(" + memoryAddress + ");\n")
				if content["dataCompressed"]:
					f.write("\tGD.cmd_inflate(" + memoryAddress + ");\n")
				else:
					f.write("\tGD.cmd_memwrite(" + memoryAddress + ", sizeof(" + contentName + "));\n")
				f.write("\tGD.copy(" + contentName + ", sizeof(" + contentName + "));\n")
	bitmapHandle = 0
	for handle in document["handles"]:
		if "sourceContent" in handle:
			memoryAddress = "RAM_" + handle["sourceContent"].replace("/", "_").upper();
			f.write("\tGD.BitmapHandle(" + str(bitmapHandle) + ");\n");
			f.write("\tGD.BitmapSource(" + memoryAddress + ");\n");
			f.write("\tGD.BitmapLayout(" + formatList[handle["layoutFormat"]] + ", " + str(handle["layoutStride"]) + ", " + str(handle["layoutHeight"]) + ");\n");
			f.write("\tGD.BitmapSize(" + filterList[handle["sizeFilter"]] + ", " + wrapList[handle["sizeWrapX"]] + ", " + wrapList[handle["sizeWrapY"]] + ", " + str(handle["sizeWidth"]) + ", " + str(handle["sizeHeight"]) + ");\n");
		bitmapHandle = bitmapHandle + 1
	f.write("}\n")
	f.write("\n")
	f.write("void loop()\n")
	f.write("{\n")
	for line in document["coprocessor"]:
		if not line == "":
			splitline = line.split('(', 1)
			newline = "\tGD." + functionMap[splitline[0]] + "(" + splitline[1] + ";\n"
			f.write(newline)
		else:
			f.write("\t\n")
	f.write("\tGD.swap();\n")
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
	print resultText
	return resultText
