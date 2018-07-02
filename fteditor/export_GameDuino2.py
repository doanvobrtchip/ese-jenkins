import os, shutil, subprocess, sys, re

def displayName():
    return "Gameduino2 Project"

WriteMemfromflashFunction = """
//write raw content embedded in the flash
void WriteMemfromflash(uint32_t Addr, const prog_uchar *Src,uint32_t NBytes)
{
    GD.cmd_memwrite(Addr,NBytes);
    GD.copy(Src, NBytes);
}
"""

Comments = """
    /*
    NOTE:
    1] Assets have to be copied to the root directory of the SD card and inserted to the device.
    2] Arduino uses the 8.3 format which means the file name is limited to 8 characters long and
        the extension has a maximum of 3 characters long.  Files can not be loaded if the file name
        doesn't meet the 8.3 rule.

    ***If the Arduion Sketch is not running, please check the following:
    - The actual device matches the selected device type in the FTDI EVE Editor.
    - The selected screen resolution matches the actual device.
    - The actual sketch RAM usage during runtime is within the device's limit.
    - Unicode characters in the path to the Arduino sketch might prevent the Arduino IDE from openning the project.
    - Device might need to reset after updloading the sketch.
    */
"""

lutSize = 1024
paletted_format = 8
palettedFormats = [paletted_format]


def run(name, document, ram):
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
        #"CMD_CRC" : "cmd_crc",
        #"CMD_HAMMERAUX" : "cmd_hammeraux",
        #"CMD_MARCH" : "cmd_march",
        #"CMD_IDCT" : "cmd_idct",
        #"CMD_EXECUTE" : "cmd_execute",
        #"CMD_GETPOINT" : "cmd_getpoint",
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
        #"CMD_MEMZERO" : "cmd_memzero",
        "CMD_MEMCPY" : "cmd_memcpy",
        "CMD_APPEND" : "cmd_append",
        "CMD_SNAPSHOT" : "cmd_snapshot",
        #"CMD_TOUCH_TRANSFORM" : "cmd_touch_transform",
        #"CMD_BITMAP_TRANSFORM" : "cmd_bitmap_transform",
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
        #"CMD_LOGO" : "cmd_logo",
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
    outDir = "FT80X_" + name + "_gd2"
    try:
        if os.path.isdir(outDir):
            #for (dirpath, dirnames, filenames) in os.walk(outDir):# Windows does not allow the removal of read-only files
            #    for filename in filenames:
            #        os.chmod(os.path.join(dirpath, filename),stat.S_IWRITE)
            shutil.rmtree(outDir)
        os.makedirs(outDir)
    except:
        raise IOError("Unable to generate project. Try again and make sure the previous generated project files and skeleton project files are not currently being accessed.")

    SDCardAccess = False
    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                functionName = splitlinea[0]
                if functionName == "CMD_LOADIMAGE":
                    SDCardAccess = True
                    break
            except:
                pass

    outName = outDir + os.path.sep + "FT80X_" + name + "_gd2.ino" # os.path.basename(os.getcwd()) + ".ino"
    if os.path.isfile(outName):
        os.remove(outName)
    f = open(outName, "w")
    f.write("#include <EEPROM.h>\n")
    f.write("#include <SPI.h>\n")
    f.write("#include <GD2.h>\n")
    if SDCardAccess:
        f.write("#include <SD.h>\n")

    for content in document["content"]:
        try:
            if content["memoryLoaded"]:
                if content["imageFormat"] in palettedFormats:
                    ramOffset = int(content["memoryAddress"])
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                    lutAddress = memoryAddress + "_LUT"
                    f.write("\n")

                    if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                        f.write("#define " + lutAddress + " RAM_PAL" + "\n")
                        f.write("#define " + memoryAddress + " " + str(ramOffset) + "\n")
                    else:
                        f.write("#define " + lutAddress + " " + str(ramOffset) + "\n")
                        f.write("#define " + memoryAddress + " " + str(ramOffset + lutSize) + "\n")
                else:
                    memoryAddress = "RAM_" + re.sub(r'[/. ]', '_', content["destName"]).upper()
                    f.write("\n")
                    f.write("#define " + memoryAddress + " " + str(content["memoryAddress"]) + "\n")
            if content["dataStorage"] == 'Embedded':
                contentName = re.sub(r'[-/. ]', '_', content["destName"]).upper()
                headerName = content["destName"].replace('/', os.path.sep)
                lutContentName = contentName + "_lut"
                lutHeaderName = headerName + ".lut"
                #load raw files
                if content["converter"] == "Raw":
                    fcontent = ""
                    lutContent = ""
                    with open(headerName + '.raw', 'rb') as rawf:
                        fcontent = rawf.read()

                    if content["imageFormat"] in palettedFormats:
                        with open(lutHeaderName + '.raw', 'rb') as rawf:
                            lutContent = rawf.read()

                    targetName = contentName + ".h"
                    targetPath = outDir + os.path.sep + targetName
                    lutTargetName = lutContentName + ".h"
                    lutTargetPath = outDir + os.path.sep + lutTargetName

                    if os.path.isfile(targetPath):
                        os.remove(targetPath)

                    if content["imageFormat"] in palettedFormats:
                        if os.path.isfile(lutTargetPath):
                            os.remove(lutTargetPath)
                        foutput = open(lutTargetPath, 'w+')
                        for i in range(0, len(lutContent)):
                            foutput.write("{}".format(lutContent[i]))
                            foutput.write(",")
                        foutput.close()

                    foutput = open(targetPath, 'w+')
                    for i in range(0,len(fcontent)):
                        foutput.write("{}".format(fcontent[i]))
                        foutput.write(",")
                    foutput.close()
                else:
                    if content["dataCompressed"]:
                        headerName += ".binh"
                        lutHeaderName += ".binh"
                    else:
                        headerName += ".rawh"
                        lutHeaderName += ".rawh"
                    targetName = contentName + ".h"
                    lutTargetName = lutContentName + ".h"
                    targetPath = outDir + os.path.sep + targetName
                    lutTargetPath = outDir + os.path.sep + lutTargetName

                    if content["imageFormat"] in palettedFormats:
                        if os.path.isfile(lutTargetPath):
                            os.remove(lutTargetPath)
                        shutil.copy(lutHeaderName, lutTargetPath)

                    if os.path.isfile(targetPath):
                        os.remove(targetPath)
                    shutil.copy(headerName, targetPath)
                content["gd2Name"] = contentName
                f.write("\n")
                charType = "prog_uchar"
                #charType = "char" # For older Linux distro
                if content["imageFormat"] in palettedFormats:
                    f.write("static const PROGMEM " + charType + " " + lutContentName + "[] = {\n")
                    f.write("\t#include \"" + lutTargetName + "\"\n")
                    f.write("};\n")
                f.write("static const PROGMEM " + charType + " " + contentName + "[] = {\n")
                f.write("\t#include \"" + targetName + "\"\n")
                f.write("};\n")
        except (UnicodeDecodeError, UnicodeEncodeError):
            f.close()
            raiseUnicodeError("Name of the assets")

    f.write(Comments)
    f.write("\n")
    f.write("void setup()\n")
    f.write("{\n")

    if SDCardAccess:
        f.write("\tpinMode(9, OUTPUT);\n")
        f.write("\tif (!SD.begin()){\n")
        f.write("\t//sd card failed to initialize.\n")
        f.write("\t}else{\n")
        f.write("\t//sd card successfully initialized\n")
        f.write("\t}\n")
        f.write("\tGD.begin();\n")
    else:
        f.write("\tGD.begin();\n")

    showMemWriteFunction = False
    for content in document["content"]:
        if content["memoryLoaded"]:
            memoryAddress = "RAM_" + re.sub(r'[/. ]', '_' , content["destName"]).upper()
            lutMemoryAddress = memoryAddress + "_LUT"
            if content["dataStorage"] == 'Embedded':
                contentName = content["gd2Name"]
                lutContentName = contentName + "_lut"
                if content["converter"] == "RawJpeg":
                    f.write("\tGD.cmd_loadimage(" + memoryAddress + ");\n")
                    f.write("\tGD.copy(" + contentName + ", sizeof(" + contentName + "));\n")
                if content["converter"] == "Raw":
                    if content["imageFormat"] in palettedFormats:
                        if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                            f.write("\tWriteMemfromflash( RAM_PAL , " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                        else:
                            f.write("\tWriteMemfromflash(" + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                    else:
                        f.write("\tWriteMemfromflash(" + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                    showMemWriteFunction = True
                else:
                    if content["dataCompressed"]:
                        f.write("\tGD.cmd_inflate(" + memoryAddress + ");\n")
                        f.write("\tGD.copy(" + contentName + ", sizeof(" + contentName + "));\n")
                        if content["imageFormat"] in palettedFormats:
                            if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                                f.write("\tGD.cmd_inflate( RAM_PAL );\n")
                            else:
                                f.write("\tGD.cmd_inflate(" + lutMemoryAddress + ");\n")
                            f.write("\tGD.copy(" + lutContentName + ", sizeof(" + lutContentName + "));\n")
                    else:
                        f.write("\tWriteMemfromflash(" + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                        if content["imageFormat"] in palettedFormats:
                            if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                                f.write("\tWriteMemfromflash( RAM_PAL , " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                            else:
                                f.write("\tWriteMemfromflash(" + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                        showMemWriteFunction = True
    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                splitlineb = splitlinea[1].split(')', 1)
                functionName = splitlinea[0]
                functionArgs = splitlineb[0]
                functionName = functionMap[functionName]
                commentsRegex = re.compile("//.*$")
                if functionName == "BitmapHandle" or functionName == "BitmapSource" or functionName == "BitmapLayout" or functionName == "BitmapSize" or functionName == "cmd_setfont":

                    comment = ""
                    m = commentsRegex.match(splitlineb[1])
                    if m:
                        comment = m.group(0)

                    newline = "\tGD." + functionName + "(" + functionArgs + ");" + comment + "\n"
                    f.write(newline)
                else:
                    break
            except:
                pass
    f.write("}\n")
    f.write("\n")
    f.write("void loop()\n")
    f.write("{\n")
    clearFound = False
    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                splitlineb = splitlinea[1].split(')', 1)
                functionName = splitlinea[0]
                functionArgs = splitlineb[0]
                functionName = functionMap[functionName]
                if functionName == "Clear":
                    clearFound = True
                    break
            except:
                pass
    if not clearFound:
        f.write("\tGD.Clear(1, 1, 1);\n")
    skippedBitmaps = False
    specialParameter = ""
    specialCommandType = ""

    for line in document["coprocessor"]:
        if not line == "":
            try:
                if (line.lstrip()).startswith("//"):#if the line is a comment line then just write it out
                    f.write("\t" + line + "\n")
                    continue

                splitlinea = line.split('(', 1)
                splitlineb = splitlinea[1].split(')', 1)
                functionName = splitlinea[0]
                functionArgs = splitlineb[0]
                functionName = functionMap[functionName]
                commentsRegex = re.compile("//.*$")

                if not skippedBitmaps:
                    if functionName == "BitmapHandle" or functionName == "BitmapSource" or functionName == "BitmapLayout" or functionName == "BitmapSize" or functionName == "cmd_setfont":
                        continue
                    else:
                        skippedBitmaps = True

                if functionName == "cmd_loadimage":
                    functionArgsSplit = functionArgs.split(',')
                    functionArgs = functionArgsSplit[0] + ","
                    functionArgs += functionArgsSplit[1]
                    functionArgsSplit[2] = re.sub(r'["]', "", functionArgsSplit[2])
                    if '/' in functionArgsSplit[2]:
                        specialParameter = functionArgsSplit[2].rsplit('/',1)[1]
                    elif '\\' in functionArgsSplit[2]:
                        specialParameter = functionArgsSplit[2].rsplit('\\',1)[1]
                    else:
                        specialParameter = functionArgsSplit[2]

                    try:
                        shutil.copy(functionArgsSplit[2].strip(), outDir + os.path.sep + specialParameter)
                    except:
                        pass
                    #f.write("\tGD.__end();\n")
                    specialCommandType = "cmd_loadimage"

                comment = ""
                m = commentsRegex.match(splitlineb[1])
                if m:
                    comment = m.group(0)

                newline = "\tGD." + functionName + "(" + functionArgs + ");" + comment + "\n"
                f.write(newline)

                if specialCommandType == "cmd_loadimage":
                    try:
                        fileOnSDCard = specialParameter.split('.')
                        if (len(fileOnSDCard[0]) > 8) or (len(fileOnSDCard[1]) > 3):
                            f.write("\t/* File name doesn't meet the 8.3 rule, see above. */\n")
                    except:
                        pass
                    #f.write("\tLoadToCoprocessorCMDfifo(\"" + specialParameter + "\");\n")
                    f.write("\tGD.load(\"" + specialParameter + "\");\n")
                    specialCommandType = ""
                    #f.write("\tGD.resume();\n")

            except:
                pass
        else:
            if skippedBitmaps:
                f.write("\t\n")
    f.write("\tGD.swap();\n")
    f.write("}\n")
    f.write("\n")

    if showMemWriteFunction:
        f.write(WriteMemfromflashFunction)

    f.write("/* end of file */\n")
    f.close()
    resultText += "<b>Output</b>: " + outName
    if sys.platform.startswith('darwin'):
        subprocess.call(('open', outName))
    elif os.name == 'nt':
        #os.startfile((os.getcwd() + "/" + outName).replace("/", "\\"))
        os.startfile(outName.replace("/", "\\"))
    elif os.name == 'posix':
        subprocess.call(('xdg-open', outName))
    #print resultText
    return resultText
