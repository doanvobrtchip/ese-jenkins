import os, shutil, subprocess, sys, re


def displayName():
    return "EVE Arduino Project"


def convertArgs(functionArgs):
    argsMap = {
    "ARGB1555": "FT_ARGB1555",
    "L1": "FT_L1",
    "L4": "FT_L4",
    "L8": "FT_L8",
    "RGB332": "FT_RGB332",
    "ARGB2": "FT_ARGB2",
    "ARGB4": "FT_ARGB4",
    "RGB565": "FT_RGB565",
    "PALETTED": "FT_PALETTED",
    "TEXT8X8": "FT_TEXT8X8",
    "TEXTVGA": "FT_TEXTVGA",
    "BARGRAPH": "FT_BARGRAPH",

    "NEAREST": "FT_NEAREST",
    "BILINEAR": "FT_BILINEAR",

    "BORDER": "FT_BORDER",
    "REPEAT": "FT_REPEAT",

    "NEVER": "FT_NEVER",
    "LESS": "FT_LESS",
    "LEQUAL": "FT_LEQUAL",
    "GREATER": "FT_GREATER",
    "GEQUAL": "FT_GEQUAL",
    "EQUAL": "FT_EQUAL",
    "NOTEQUAL": "FT_NOTEQUAL",
    "ALWAYS": "FT_ALWAYS",

    "KEEP": "FT_KEEP",
    "REPLACE": "FT_REPLACE",
    "INCR": "FT_INCR",
    "DECR": "FT_DECR",
    "INVERT": "FT_INVERT",

    "ZERO": "FT_ZERO",
    "ONE": "FT_ONE",
    "SRC_ALPHA": "FT_SRC_ALPHA",
    "DST_ALPHA": "FT_DST_ALPHA",
    "ONE_MINUS_SRC_ALPHA": "FT_ONE_MINUS_SRC_ALPHA",
    "ONE_MINUS_DST_ALPHA": "FT_ONE_MINUS_DST_ALPHA",

    "BITMAPS": "FT_BITMAPS",
    "POINTS": "FT_POINTS",
    "LINES": "FT_LINES",
    "LINE_STRIP": "FT_LINE_STRIP",
    "EDGE_STRIP_R": "FT_EDGE_STRIP_R",
    "EDGE_STRIP_L": "FT_EDGE_STRIP_L",
    "EDGE_STRIP_A": "FT_EDGE_STRIP_A",
    "EDGE_STRIP_B": "FT_EDGE_STRIP_B",
    "RECTS": "FT_RECTS",

    "OPT_MONO": "FT_OPT_MONO",
    "OPT_NODL": "FT_OPT_NODL",
    "OPT_FLAT": "FT_OPT_FLAT",
    "OPT_CENTERX": "FT_OPT_CENTERX",
    "OPT_CENTERY": "FT_OPT_CENTERY",
    "OPT_CENTER": "FT_OPT_CENTER",
    "OPT_NOBACK": "FT_OPT_NOBACK",
    "OPT_NOTICKS": "FT_OPT_NOTICKS",
    "OPT_NOHM": "FT_OPT_NOHM",
    "OPT_NOPOINTER": "FT_OPT_NOPOINTER",
    "OPT_NOSECS": "FT_OPT_NOSECS",
    "OPT_NOHANDS": "FT_OPT_NOHANDS",
    "OPT_RIGHTX": "FT_OPT_RIGHTX",
    "OPT_SIGNED": "FT_OPT_SIGNED",
    }

    convertedArgsList = []

    functionArgsSplit = functionArgs.split(",")
    for argument in functionArgsSplit:
        if '|' in argument:
            argumentOptions = argument.split('|')
            for argumentOption in argumentOptions:
                if (argumentOption.replace(" ", "") in argsMap) and (argumentOption.replace(" ", "") not in convertedArgsList):
                    functionArgs = functionArgs.replace(argumentOption.replace(" ", ""), argsMap[argumentOption.replace(" ", "")])
                    convertedArgsList.append(argumentOption.replace(" ", ""))
        if (argument.replace(" ", "") in argsMap) and (argument.replace(" ", "") not in convertedArgsList):
            functionArgs = functionArgs.replace(argument.replace(" ", ""), argsMap[argument.replace(" ", "")])
            convertedArgsList.append(argument.replace(" ", ""))
    #return ",".join(functionArgsSplit)
    return functionArgs

lutSize = 1024
paletted_format = 8
palettedFormats = [paletted_format]


WriteMemfromflashFunction = """
//write raw content embedded in the flash
void WriteMemfromflash(uint32_t Addr, prog_uchar *Src,uint32_t NBytes)
{
    FTImpl.Cmd_Memwrite(Addr,NBytes);
    FTImpl.WriteCmdfromflash(Src, NBytes);
}
"""

LoadToCoprocessorCMDfifo = """
void LoadToCoprocessorCMDfifo(char* fileName){
    uint8_t imbuff[512];

    if(FtSd.OpenFile(jpeg, fileName)){
        //Unable to open file
    }
    else{
        while (jpeg.Offset < jpeg.Size)
        {
            uint32_t n = min(512, jpeg.Size - jpeg.Offset);
            n = (uint32_t)(n + 3) & (~3);   // force 32-bit alignment
            jpeg.ReadSector(imbuff);
            FTImpl.WriteCmd(imbuff, n);//alignment is already taken care by this api
        }
    }
}
"""

SDCard_initialization = """
    /* SD card object*/
    FT_SD FtSd(FT_SD_CSPIN);
    /* sd_present holds error values during initialization.  0 means no error and all other errors are non zero value */
    FT_SDStatus sd_present;

    /*SD file object for file accessing*/
    FT_SDFile jpeg;

    /*
    NOTE:
    1] Assets have to be copied to the root directory of the SD card and inserted to the device.
    2] Arduino uses the 8.3 format which means the file name is limited to 8 characters long and
        the extension has a maximum of 3 characters long.  Files can not be loaded if the file name
        doesn't meet the 8.3 rule.

    If the Arduion Sketch is not running, please check the following:
    - The actual device matches the selected device type in the FTDI EVE Editor.
    - The selected screen resolution matches the actual device.
    - The actual sketch RAM usage during runtime is within the device's limit.
    - Unicode characters in the path to the Arduino sketch might prevent the Arduino IDE from openning the project.
    - Device might need to reset after updloading the sketch.
    */

"""

def raiseUnicodeError(errorArea):
    raise Exception("Unable to export project: unicode characters are currently unsupported.  Please check: " + errorArea)


# name: the input file name,
def run(name, document, ram, moduleName):
    try:
        name.decode('ascii')
    except UnicodeDecodeError:
        raiseUnicodeError("Project Name")

    resultText = "<b>EVE Arduino Export</b><br>"
    functionMap = {
    "VERTEX2II": "Vertex2ii",
    "VERTEX2F": "Vertex2f",
    "DISPLAY": "Display",
    "BITMAP_SOURCE": "BitmapSource",
    "CLEAR_COLOR_RGB": "ClearColorRGB",
    "TAG": "Tag",
    "COLOR_RGB": "ColorRGB",
    "BITMAP_HANDLE": "BitmapHandle",
    "CELL": "Cell",
    "BITMAP_LAYOUT": "BitmapLayout",
    "BITMAP_SIZE": "BitmapSize",
    "ALPHA_FUNC": "AlphaFunc",
    "STENCIL_FUNC": "StencilFunc",
    "BLEND_FUNC": "BlendFunc",
    "STENCIL_OP": "StencilOp",
    "POINT_SIZE": "PointSize",
    "LINE_WIDTH": "LineWidth",
    "CLEAR_COLOR_A": "ClearColorA",
    "COLOR_A": "ColorA",
    "CLEAR_STENCIL": "ClearStencil",
    "CLEAR_TAG": "ClearTag",
    "STENCIL_MASK": "StencilMask",
    "TAG_MASK": "TagMask",
    "BITMAP_TRANSFORM_A": "BitmapTransformA",
    "BITMAP_TRANSFORM_B": "BitmapTransformB",
    "BITMAP_TRANSFORM_C": "BitmapTransformC",
    "BITMAP_TRANSFORM_D": "BitmapTransformD",
    "BITMAP_TRANSFORM_E": "BitmapTransformE",
    "BITMAP_TRANSFORM_F": "BitmapTransformF",
    "SCISSOR_XY": "ScissorXY",
    "SCISSOR_SIZE": "ScissorSize",
    "CALL": "Call",
    "JUMP": "Jump",
    "BEGIN": "Begin",
    "COLOR_MASK": "ColorMask",
    "END": "End",
    "SAVE_CONTEXT": "SaveContext",
    "RESTORE_CONTEXT": "RestoreContext",
    "RETURN": "Return",
    "MACRO": "Macro",
    "CLEAR": "Clear",
    "CMD_DLSTART": "Cmd_DLStart",
    "CMD_SWAP": "Cmd_Swap",
    "CMD_INTERRUPT": "Cmd_Interrupt",
    "CMD_GETPOINT": "Cmd_GetPtr",
    "CMD_BGCOLOR": "Cmd_BGColor",
    "CMD_FGCOLOR": "Cmd_FGColor",
    "CMD_GRADIENT": "Cmd_Gradient",
    "CMD_TEXT": "Cmd_Text",
    "CMD_BUTTON": "Cmd_Button",
    "CMD_KEYS": "Cmd_Keys",
    "CMD_PROGRESS": "Cmd_Progress",
    "CMD_SLIDER": "Cmd_Slider",
    "CMD_SCROLLBAR": "Cmd_Scrollbar",
    "CMD_TOGGLE": "Cmd_Toggle",
    "CMD_GAUGE": "Cmd_Gauge",
    "CMD_CLOCK": "Cmd_Clock",
    "CMD_CALIBRATE": "Cmd_Calibrate",
    "CMD_SPINNER": "Cmd_Spinner",
    "CMD_STOP": "Cmd_Stop",
    "CMD_MEMCRC": "Cmd_Memcrc",
    "CMD_REGREAD": "Cmd_RegRead",
    "CMD_MEMWRITE": "Cmd_Memwrite",
    "CMD_MEMSET": "Cmd_Memset",
    "CMD_MEMZERO": "Cmd_Memzero",
    "CMD_MEMCPY": "Cmd_Memcpy",
    "CMD_APPEND": "Cmd_Append",
    "CMD_SNAPSHOT": "Cmd_Snapshot",
    "CMD_INFLATE": "Cmd_Inflate",
    "CMD_GETPTR": "Cmd_GetPtr",
    "CMD_LOADIMAGE": "Cmd_LoadImage",
    "CMD_GETPROPS": "Cmd_GetProps",
    "CMD_LOADIDENTITY": "Cmd_LoadIdentity",
    "CMD_TRANSLATE": "Cmd_Translate",
    "CMD_SCALE": "Cmd_Scale",
    "CMD_ROTATE": "Cmd_Rotate",
    "CMD_SETMATRIX": "Cmd_SetMatrix",
    "CMD_SETFONT": "Cmd_SetFont",
    "CMD_TRACK": "Cmd_Track",
    "CMD_DIAL": "Cmd_Dial",
    "CMD_NUMBER": "Cmd_Number",
    "CMD_SCREENSAVER": "Cmd_ScreenSaver",
    "CMD_SKETCH": "Cmd_Sketch",
    "CMD_LOGO": "Cmd_Logo",
    "CMD_COLDSTART": "Cmd_ColdStart",
    "CMD_GETMATRIX": "Cmd_GetMatrix",
    "CMD_GRADCOLOR": "Cmd_GradColor"
    }
    for line in document["displayList"]:
        if not line == "":
            resultText += "<b>Warning</b>: Only support for Coprocessor, ignoring Display List.<br>"
            break
    outDir = "FT80X_" + name + "_FTEVE"
    try:
        if os.path.isdir(outDir):
            #for (dirpath, dirnames, filenames) in os.walk(outDir):# Windows does not allow the removal of read-only files
            #    for filename in filenames:
            #        os.chmod(os.path.join(dirpath, filename),stat.S_IWRITE)
            shutil.rmtree(outDir)
        os.makedirs(outDir)
    except:
        raise IOError("Unable to generate project. Try again and make sure the previous generated project files and skeleton project files are not currently being accessed.")


    loadimageCommandFound = "False"
    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                functionName = splitlinea[0]
                if functionName == "CMD_LOADIMAGE":
                    loadimageCommandFound = "True"
                    break
            except:
                pass



    #raise Exception('At: ' + os.getcwd() + ' Read/Write permission issue.')
    outName = outDir + os.path.sep + "FT80X_" + name + "_FTEVE.ino"  # os.path.basename(os.getcwd()) + ".ino"

    f = open(outName, "w+")
    f.write("#include <EEPROM.h>\n")
    f.write("#include <SPI.h>\n")
    f.write("#include <Wire.h>\n")

    if moduleName == "ADAM_4DLCD_FT843":
        f.write("#include <FT_ADAM_4DLCD_FT843.h>\n")
    elif moduleName == "Breakout_4DLCD_FT843":
        f.write("#include <FT_Breakout_4DLCD_FT843.h>\n")
    elif moduleName == "VM800B35":
        f.write("#include <FT_VM800B35.h>\n")
    elif moduleName == "VM800P35":
        f.write("#include <FT_VM800P35.h>\n")
    elif moduleName == "VM800P43_50":
        f.write("#include <FT_VM800P43_50.h>\n")
    elif moduleName == "VM801B43":
        f.write("#include <FT_VM801B43.h>\n")
    elif moduleName == "VM801P43_50":
        f.write("#include <FT_VM801P43_50.h>\n")

    for content in document["content"]:
        try:
            if content["memoryLoaded"]:
                if content["imageFormat"] in palettedFormats:
                    ramOffset = int(content["memoryAddress"])
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                    lutAddress = memoryAddress + "_LUT"
                    f.write("\n")

                    if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                        f.write("#define " + lutAddress + " FT_RAM_PAL" + "\n")
                        f.write("#define " + memoryAddress + " " + str(ramOffset) + "\n")
                    else:
                        f.write("#define " + lutAddress + " " + str(ramOffset) + "\n")
                        f.write("#define " + memoryAddress + " " + str(ramOffset + lutSize) + "\n")
                else:
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                    f.write("\n")
                    f.write("#define " + memoryAddress + " " + str(content["memoryAddress"]) + "\n")
            if content["dataStorage"] == 'Embedded':
                data = ""
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
                            foutput.write(str(ord(lutContent[i])))
                            foutput.write(",")
                        foutput.close()

                    foutput = open(targetPath, 'w+')
                    for i in range(0, len(fcontent)):
                        foutput.write(str(ord(fcontent[i])))
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
                content["FTEVE_Name"] = contentName
                f.write("\n")
                charType = "prog_uchar"
                #charType = "char" # For older Linux distro
                if content["imageFormat"] in palettedFormats:
                    f.write("static PROGMEM " + charType + " " + lutContentName + "[] = {\n")
                    f.write("\t#include \"" + lutTargetName + "\"\n")
                    f.write("};\n")
                f.write("static PROGMEM " + charType + " " + contentName + "[] = {\n")
                f.write("\t#include \"" + targetName + "\"\n")
                f.write("};\n")
        except (UnicodeDecodeError, UnicodeEncodeError):
            f.close()
            raiseUnicodeError("Name of the assets")
    f.write("\n")

    if document["project"]["device"] == 2048:
        f.write("FT800IMPL_SPI FTImpl(FT_CS_PIN,FT_PDN_PIN,FT_INT_PIN);\n")
    elif document["project"]["device"] == 2049:
        f.write("FT801IMPL_SPI FTImpl(FT_CS_PIN,FT_PDN_PIN,FT_INT_PIN);\n")
    else:
        f.write("FT800IMPL_SPI FTImpl(FT_CS_PIN,FT_PDN_PIN,FT_INT_PIN);\n")

    if loadimageCommandFound == "True":
        f.write(SDCard_initialization)
        f.write(LoadToCoprocessorCMDfifo)

    f.write("void setup()\n")
    f.write("{\n")

    if loadimageCommandFound == "True":
        f.write("\tFtSd.Init();\n")
        f.write("\tif(sd_present){\n")
        f.write("\t//SD card is not detected.\n")
        f.write("\t}\n")
    f.write("\tFTImpl.Init(FT_DISPLAY_RESOLUTION);\n")
    f.write("\tFTImpl.SetDisplayEnablePin(FT_DISPENABLE_PIN);\n")
    f.write("\tFTImpl.SetAudioEnablePin(FT_AUDIOENABLE_PIN);\n")
    f.write("\tFTImpl.DisplayOn();\n")
    f.write("\tFTImpl.AudioOn();\n")
    f.write("\n\tFTImpl.DLStart();\n")
    showWriteMemfromFlashFunction = False
    for content in document["content"]:
        if content["memoryLoaded"]:
            memoryAddress = "RAM_" + re.sub(r'[/. ]', '_', content["destName"]).upper()
            lutMemoryAddress = memoryAddress + "_LUT"
            if content["dataStorage"] == 'Embedded':
                contentName = content["FTEVE_Name"]
                lutContentName = contentName + "_lut"
                if content["converter"] == "RawJpeg":
                    f.write("\tFTImpl.Cmd_LoadImage(" + memoryAddress + ");\n")
                    f.write("\tFTImpl.WriteCmdfromflash(" + contentName + ", sizeof(" + contentName + "));\n")
                if content["converter"] == "Raw":
                    if content["imageFormat"] in palettedFormats:
                        if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                            f.write("\tWriteMemfromflash( FT_RAM_PAL , " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                        else:
                            f.write("\tWriteMemfromflash(" + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                    else:
                        f.write("\tWriteMemfromflash(" + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                    showWriteMemfromFlashFunction = True
                else:
                    if content["dataCompressed"]:
                        f.write("\tFTImpl.Cmd_Inflate(" + memoryAddress + ");\n")
                        f.write("\tFTImpl.WriteCmdfromflash(" + contentName + ", sizeof(" + contentName + "));\n")
                        if content["imageFormat"] in palettedFormats:
                            if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                                f.write("\tFTImpl.Cmd_Inflate( FT_RAM_PAL );\n")
                            else:
                                f.write("\tFTImpl.Cmd_Inflate(" + lutMemoryAddress + ");\n")
                            f.write("\tFTImpl.WriteCmdfromflash(" + lutContentName + ", sizeof(" + lutContentName + "));\n")
                    else:
                        f.write("\tWriteMemfromflash(" + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                        if content["imageFormat"] in palettedFormats:
                            if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:
                                f.write("\tWriteMemfromflash( FT_RAM_PAL , " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                            else:
                                f.write("\tWriteMemfromflash(" + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n")
                        showWriteMemfromFlashFunction = True
    f.write("\tFTImpl.Finish();\n")
    f.write("\tFTImpl.DLStart();\n")
    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                splitlineb = splitlinea[1].split(')', 1)
                functionName = splitlinea[0]
                functionName = functionMap[functionName]
                #commentsRegex = re.compile(r'(?![\n\r])\s?//[\w\W].+')
                commentsRegex = re.compile("//.*$")
                if functionName == "BitmapHandle" or functionName == "BitmapSource" or functionName == "BitmapLayout" or functionName == "BitmapSize" or functionName == "Cmd_SetFont":
                    functionArgs = convertArgs(splitlineb[0])
                    comment = ""
                    m = commentsRegex.match(splitlineb[1])
                    if m:
                        comment = m.group(0)
                    newline = "\tFTImpl." + functionName + "(" + functionArgs + ");" + comment + "\n"
                    f.write(newline)
                else:
                    break
            except:
                pass
    f.write("\tFTImpl.DLEnd();\n")
    f.write("\tFTImpl.Finish();\n")
    f.write("}\n")
    f.write("\n")
    f.write("void loop()\n")
    f.write("{\n")
    clearFound = False
    clearBuffersFound = False
    skippedBitmaps = False
    isFirstCommand = True
    incrementCallandJump = False
    specialParameter = ""
    specialCommandType = ""

    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                functionName = splitlinea[0]
                functionName = functionMap[functionName]
                if functionName == "Clear":
                    clearFound = True
                if functionName == "ClearColorA" or functionName == "ClearColorRGB" or functionName == "ClearStencil" or functionName == "ClearTag":
                    clearBuffersFound = True
                    incrementCallandJump = True
            except:
                pass


    f.write("\tFTImpl.DLStart();\n")


    for line in document["coprocessor"]:
        if not line == "":
            if (line.lstrip()).startswith("//"):  #if the line is a comment line then just write it out
                f.write("\t" + line + "\n")
                continue

            try:
                splitlinea = line.split('(', 1)
                splitlineb = splitlinea[1].split(')', 1)
                functionName = splitlinea[0]
                functionName = functionMap[functionName]
                commentsRegex = re.compile("//.*$")
                if not skippedBitmaps:
                    if functionName == "BitmapHandle" or functionName == "BitmapSource" or functionName == "BitmapLayout" or functionName == "BitmapSize" or functionName == "Cmd_SetFont":
                        continue
                    else:
                        skippedBitmaps = True
                functionArgs = convertArgs(splitlineb[0])

                if (functionName == "Clear") and isFirstCommand and clearFound and not clearBuffersFound:
                    isFirstCommand = False
                    continue

                if (functionName == "Call" or functionName == "Jump") and clearBuffersFound:
                    try:
                        tempInt = int(functionArgs) + 1
                        functionArgs = str(tempInt)
                    except:
                        pass


                if functionName == "Cmd_LoadImage":
                    functionArgsSplit = functionArgs.split(',')
                    functionArgs = functionArgsSplit[0] + ","
                    functionArgs += functionArgsSplit[1]
                    functionArgsSplit[2] = re.sub(r'["]', "", functionArgsSplit[2])
                    #functionArgsSplit[2] = re.sub(r'[:]', '.', functionArgsSplit[2])
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

                    specialCommandType = "Cmd_LoadImage"

                comment = ""
                try:
                    m = commentsRegex.match(splitlineb[1])
                except:
                    pass
                if m:
                    comment = m.group(0)
                newline = "\tFTImpl." + functionName + "(" + functionArgs + ");" + comment + "\n"
                f.write(newline)

                if specialCommandType == "Cmd_LoadImage":
                    try:
                        fileOnSDCard = specialParameter.split('.')
                        if (len(fileOnSDCard[0]) > 8) or (len(fileOnSDCard[1]) > 3):
                            f.write("\t/* File name doesn't meet the 8.3 rule, see above. */\n")
                    except:
                        pass
                    f.write("\tLoadToCoprocessorCMDfifo(\"" + specialParameter + "\");\n")
                    specialCommandType = ""

            except Exception as e:
                #raise Exception(e)
                pass
        else:
            if skippedBitmaps:
                f.write("\t\n")
    f.write("\tFTImpl.DLEnd();\n")
    f.write("\tFTImpl.Finish();\n")
    f.write("}\n")
    f.write("\n")
    if showWriteMemfromFlashFunction:
        f.write("\n\n")
        f.write(WriteMemfromflashFunction)
        f.write("\n")
    f.write("/* end of file */\n")
    f.close()
    resultText += "<b>Output</b>: " + outName
    if sys.platform.startswith('darwin'):
        subprocess.call(('open', outName))
    elif os.name == 'nt':
        os.startfile(outName)
    #os.startfile((os.getcwd() + "/" + outName).replace("/", "\\"))
    elif os.name == 'posix':
        subprocess.call(('xdg-open', outName))
    #print resultText
    return resultText
