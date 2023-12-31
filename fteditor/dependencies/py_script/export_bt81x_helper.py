import os, sys, re, shutil, subprocess, errno, stat
from export_common import parseCommand

lutSize = 256*4

paletted_format = 8		
paletted8_format = 16		
paletted565_format = 14		
paletted4444_format = 15
palettedFormats = [paletted_format, paletted8_format, paletted565_format, paletted4444_format]

globalContext = {
    'mediaFIFOEnabled': False,
    'mediaFIFOAddress':  "",
    'mediaFIFOLength': "",
    'loadImageFromFlash' : False,
    'loadVideoFromFlash' : False,
}

globalValue = {
    'skeletonProjectName': "EVE_Hal_Library",
    'assetsFolder': "Test",
}

functionMap = {
    'CMD_APPEND' : 'Gpu_CoCmd_Append',
    'CMD_BGCOLOR' : 'Gpu_CoCmd_BgColor',
    'CMD_BITMAP_TRANSFORM' : 'Gpu_CoCmd_Bitmap_Transform',
    'CMD_BUTTON' : 'Gpu_CoCmd_Button',
    'CMD_CALIBRATE' : 'Gpu_CoCmd_Calibrate',
    'CMD_CLOCK' : 'Gpu_CoCmd_Clock',
    'CMD_COLDSTART' : 'Gpu_CoCmd_ColdStart',
    'CMD_CSKETCH' : 'Gpu_CoCmd_CSketch',
    'CMD_DIAL' : 'Gpu_CoCmd_Dial',
    'CMD_DLSTART' : 'Gpu_CoCmd_Dlstart',
    'CMD_FGCOLOR' : 'Gpu_CoCmd_FgColor',
    'CMD_GAUGE' : 'Gpu_CoCmd_Gauge',
    'CMD_GETMATRIX' : 'Gpu_CoCmd_GetMatrix',
    'CMD_GETPOINT' : 'Gpu_CoCmd_GetPoint',
    'CMD_GETPROPS' : 'Gpu_CoCmd_GetProps',
    'CMD_GETPTR' : 'Gpu_CoCmd_GetPtr',
    'CMD_GRADCOLOR' : 'Gpu_CoCmd_GradColor',
    'CMD_GRADIENT' : 'Gpu_CoCmd_Gradient',
    'CMD_INFLATE' : 'Gpu_CoCmd_Inflate',
    'CMD_INTERRUPT' : 'Gpu_CoCmd_Interrupt',
    'CMD_INT_RAMSHARED' : 'Gpu_CoCmd_Int_RamShared',
    'CMD_INT_SWLOADIMAGE' : 'Gpu_CoCmd_Int_SWLoadImage',
    'CMD_KEYS' : 'Gpu_CoCmd_Keys',
    'CMD_LOADIDENTITY' : 'Gpu_CoCmd_LoadIdentity',
    'CMD_LOADIMAGE' : 'Gpu_CoCmd_LoadImage',
    'CMD_LOGO' : 'Gpu_CoCmd_Logo',
    'CMD_MEDIAFIFO' : 'Gpu_CoCmd_MediaFifo',
    'CMD_MEMCPY' : 'Gpu_CoCmd_Memcpy',
    'CMD_MEMCRC' : 'Gpu_CoCmd_MemCrc',
    'CMD_MEMSET' : 'Gpu_CoCmd_MemSet',
    'CMD_MEMWRITE' : 'Gpu_CoCmd_MemWrite',
    'CMD_MEMZERO' : 'Gpu_CoCmd_MemZero',
    'CMD_NUMBER' : 'Gpu_CoCmd_Number',
    'CMD_PLAYVIDEO' : 'Gpu_CoCmd_PlayVideo',
    'CMD_PROGRESS' : 'Gpu_CoCmd_Progress',
    'CMD_REGREAD' : 'Gpu_CoCmd_RegRead',
    'CMD_ROMFONT' : 'Gpu_CoCmd_RomFont',
    'CMD_ROTATE' : 'Gpu_CoCmd_Rotate',
    'CMD_SCALE' : 'Gpu_CoCmd_Scale',
    'CMD_SCREENSAVER' : 'Gpu_CoCmd_ScreenSaver',
    'CMD_SCROLLBAR' : 'Gpu_CoCmd_Scrollbar',
    'CMD_SETBASE' : 'Gpu_CoCmd_SetBase',
    'CMD_SETBITMAP' : 'Gpu_CoCmd_SetBitmap',
    'CMD_SETFONT' : 'Gpu_CoCmd_SetFont',
    'CMD_SETFONT2' : 'Gpu_CoCmd_SetFont2',
    'CMD_SETMATRIX' : 'Gpu_CoCmd_SetMatrix',
    'CMD_SETROTATE' : 'Gpu_CoCmd_SetRotate',
    'CMD_SETSCRATCH' : 'Gpu_CoCmd_SetScratch',
    'CMD_SKETCH' : 'Gpu_CoCmd_Sketch',
    'CMD_SLIDER' : 'Gpu_CoCmd_Slider',
    'CMD_SNAPSHOT' : 'Gpu_CoCmd_Snapshot',
    'CMD_SNAPSHOT2' : 'Gpu_CoCmd_Snapshot2',
    'CMD_SPINNER' : 'Gpu_CoCmd_Spinner',
    'CMD_STOP' : 'Gpu_CoCmd_Stop',
    'CMD_SWAP' : 'Gpu_CoCmd_Swap',
    'CMD_SYNC' : 'Gpu_CoCmd_Sync',
    'CMD_TEXT' : 'Gpu_CoCmd_Text',
    'CMD_TOGGLE' : 'Gpu_CoCmd_Toggle',
    'CMD_TRACK' : 'Gpu_CoCmd_Track',
    'CMD_TRANSLATE' : 'Gpu_CoCmd_Translate',
    'CMD_VIDEOFRAME' : 'Gpu_CoCmd_VideoFrame',
    'CMD_VIDEOSTART' : 'Gpu_CoCmd_VideoStart',
    'CMD_FLASHERASE' : 'Gpu_CoCmd_FlashErase',
    'CMD_FLASHWRITE' : 'Gpu_CoCmd_FlashWrite',
    'CMD_FLASHREAD' : 'Gpu_CoCmd_FlashRead',
    'CMD_FLASHUPDATE' : 'Gpu_CoCmd_FlashUpdate',
    'CMD_FLASHDETACH' : 'Gpu_CoCmd_FlashDetach',
    'CMD_FLASHATTACH' : 'Gpu_CoCmd_FlashAttach',
    'CMD_FLASHFAST' : 'Gpu_CoCmd_FlashFast',
    'CMD_FLASHSPIDESEL' : 'Gpu_CoCmd_FlashSpiDesel',
    'CMD_FLASHSPITX' : 'Gpu_CoCmd_FlashSpiTx',
    'CMD_FLASHSPIRX' : 'Gpu_CoCmd_FlashSpiRx',
    'CMD_FLASHSOURCE' : 'Gpu_CoCmd_FlashSource',
    'CMD_CLEARCACHE' : 'Gpu_CoCmd_ClearCache',
    'CMD_INFLATE2' : 'Gpu_CoCmd_Inflate2',
    'CMD_ROTATEAROUND' : 'Gpu_CoCmd_RotateAround',
    'CMD_RESETFONTS' : 'Gpu_CoCmd_ResetFonts',
    'CMD_ANIMSTART' : 'Gpu_CoCmd_AnimStart',
    'CMD_ANIMSTOP' : 'Gpu_CoCmd_AnimStop',
    'CMD_ANIMXY' : 'Gpu_CoCmd_AnimXY',
    'CMD_ANIMDRAW' : 'Gpu_CoCmd_AnimDraw',
    'CMD_GRADIENTA' : 'Gpu_CoCmd_GradientA',
    'CMD_FILLWIDTH' : 'Gpu_CoCmd_FillWidth',
    'CMD_APPENDF' : 'Gpu_CoCmd_AppendF',
    'CMD_ANIMFRAME' : 'Gpu_CoCmd_AnimFrame',
    'CMD_NOP' : 'Gpu_CoCmd_Nop',
    'CMD_SHA1' : 'Gpu_CoCmd_Sha1',
    'CMD_VIDEOSTARTF' : 'Gpu_CoCmd_VideoStartF',
    'CMD_BITMAP_TRANSFORM' : 'Gpu_CoCmd_Bitmap_Transform',
    'CMD_CALIBRATESUB' : 'Gpu_CoCmd_CalibrateSub',
    'CMD_TESTCARD' : 'Gpu_CoCmd_TestCard',
    'CMD_HSF' : 'Gpu_CoCmd_Hsf',
    'CMD_APILEVEL' : 'Gpu_CoCmd_ApiLevel',
    'CMD_GETIMAGE' : 'Gpu_CoCmd_GetImage',
    'CMD_WAIT' : 'Gpu_CoCmd_Wait',
    'CMD_RETURN' : 'Gpu_CoCmd_Return',
    'CMD_CALLLIST' : 'Gpu_CoCmd_CallList',
    'CMD_NEWLIST' : 'Gpu_CoCmd_NewList',
    'CMD_ENDLIST' : 'Gpu_CoCmd_EndList',
    'CMD_PCLKFREQ' : 'Gpu_CoCmd_PclkFreq',
    'CMD_FONTCACHE' : 'Gpu_CoCmd_FontCache',
    'CMD_FONTCACHEQUERY' : 'Gpu_CoCmd_FontCacheQuery',
    'CMD_ANIMFRAMERAM' : 'Gpu_CoCmd_AnimFrameRam',
    'CMD_ANIMSTARTRAM' : 'Gpu_CoCmd_AnimStartRam',
    'CMD_RUNANIM' : 'Gpu_CoCmd_RunAnim',
    'CMD_FLASHPROGRAM' : 'Gpu_CoCmd_FlashProgram',
}

def convertArgs(functionArgs):
    argsMap = {
        "POINTS":"FTPOINTS",
    }

    for k, v in argsMap.items():
        functionArgs = functionArgs.replace(k, v)
        
    return functionArgs

def raiseUnicodeError(errorArea):
    raise Exception("Unable to export project: unicode characters are currently unsupported.  Please check: " + errorArea)
    
def exportContent(outDir, document):
    global lutSize
    export = ''
    for content in document["content"]:
        if "imageFormat" not in content:
            content["imageFormat"] = -1

        targetPath = 0
        try:
            if content["memoryLoaded"]:
                if content["imageFormat"] in palettedFormats:
                    ramOffset = int(content["memoryAddress"])
                    lutAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper() + "_LUT"
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                    export += "\n"

                    if ((content["imageFormat"] == paletted565_format) or
                        (content["imageFormat"] == paletted4444_format)):
                        lutSize = 256*2

                    export += "#define " + lutAddress + " " + str(ramOffset) + "\n"
                    export += "#define " + memoryAddress + " " + str(ramOffset + lutSize) + "\n"
                else:
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                    export += "\n"
                    export += "#define " + memoryAddress + " " + str(content["memoryAddress"]) + "\n"
            if content["dataStorage"] == "Embedded":
                data = ""
                contentName = re.sub(r'[-/. ]', '_', content["destName"])
                headerName = content["destName"] 

                lutContentName = contentName + "_lut"
                lutHeaderName = headerName + '.lut'

                #load raw files
                if content["converter"] == "Raw":
                    fcontent = ""
                    lutContent = ""
                    with open(content["destName"].replace('/',os.path.sep) + '.raw', 'rb') as rawf:
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
                    try:
                        for i in range(0,len(fcontent)):
                            foutput.write("{}".format(fcontent[i]))
                            foutput.write(",")
                    finally:
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
                export += "\n"
                charType = "uchar8_t"

                if content["imageFormat"] in palettedFormats:
                    with open (lutTargetPath, "r") as resourceFile:
                        data = resourceFile.read().replace('\n','')
                    export += "static " + charType + " " + lutContentName + "[] = {\n"
                    export += "\t" + data + "\n"
                    export += "};\n"
                    os.remove(lutTargetPath)

                with open (targetPath, "r") as resourceFile:
                    data = resourceFile.read().replace('\n','')
                export += "static " + charType + " " + contentName + "[] = {\n"
                export += "\t" + str(data) + "\n"
                export += "};\n"
                os.remove(targetPath)
        except (UnicodeDecodeError, UnicodeEncodeError):
            raiseUnicodeError("Name of the assets")
       
    return export
    
def exportLoadImageCommand(document):
    
    export = ''
    for content in document["content"]:
        if not content["memoryLoaded"]:
            continue
        memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
        lutMemoryAddress = memoryAddress + "_LUT"
        if content["dataStorage"] == "Embedded":
            contentName = content["FTEVE_Name"]
            lutContentName = contentName + "_lut"
            if content["converter"] == "RawJpeg":
                export += "\tGpu_Hal_WrCmd32(phost, CMD_LOADIMAGE);\n"
                export += "\tGpu_Hal_WrCmd32(phost, " + memoryAddress + ");\n"
                export += "\tGpu_Hal_WrCmd32(phost, 0);\n"
                export += "\tGpu_Hal_WrCmdBuf(phost, " + contentName + ", sizeof(" + contentName + "));\n"
            if content["converter"] == "Raw":
                export += "\tGpu_Hal_WrMem(phost, " + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n"
                if content["imageFormat"] in palettedFormats:
                    export += "\tGpu_Hal_WrMem(phost, " + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n"                    
            else:
                if content["dataCompressed"]:
                    export += "\tGpu_Hal_WrCmd32(phost, CMD_INFLATE);\n"                        
                    export += "\tGpu_Hal_WrCmd32(phost, " + memoryAddress + ");\n"
                    export += "\tGpu_Hal_WrCmdBuf(phost, " + contentName + ", sizeof(" + contentName + "));\n"
                    if content["imageFormat"] in palettedFormats:		
                        export += "\tGpu_Hal_WrCmd32(phost, CMD_INFLATE);\n"		
                        export += "\tGpu_Hal_WrCmd32(phost, " + lutMemoryAddress + " );\n"		
                        export += "\tGpu_Hal_WrCmdBuf(phost, " + lutContentName + ", sizeof(" + lutContentName + "));\n"                        
                else:
                    export += "\tGpu_Hal_WrMem(phost, " + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n"
                    if content["imageFormat"] in palettedFormats:		
                        export += "\tGpu_Hal_WrMem(phost, " + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n"                        
    return export
   
def exportSpecialCommand(document):
    export = ''
    for line in document["coprocessor"]:
        if line:
            try:
                cmd_name, cmd_args, comment = parseCommand(line, functionMap, convertArgs)            
                if cmd_name in ["BITMAP_HANDLE", "BITMAP_SOURCE", "BITMAP_LAYOUT", "BITMAP_SIZE"]:
                    line = "\tApp_WrCoCmd_Buffer(phost, {}({})); {}\n".format(cmd_name, cmd_args, comment)
                    export += line
                else:
                    break
            except (UnicodeDecodeError, UnicodeEncodeError):
                raiseUnicodeError("Unicode in Coprocessor editing box.")
            
    return export
    
def exportCoprocessorCommand(document, filesToTestFolder):
    export = ''
    clearFound = False
    for line in document["coprocessor"]:
        if not line: continue
        try:
            cmd_name, _, _ = parseCommand(line, functionMap, convertArgs)
            if 'CLEAR' in cmd_name:
                clearFound = True
            if cmd_name == "Gpu_CoCmd_Calibrate":
                export += '\tApp_Calibrate_Screen(phost);\n'                    
                document["coprocessor"].remove(line)                    
            elif cmd_name == "Gpu_CoCmd_Logo":
                export += '\tApp_Show_Logo(phost);\n'                    
                document["coprocessor"].remove(line)
        except:
            pass

    export += '\tGpu_CoCmd_FlashFast(phost, 0);\n'
    export += '\tGpu_CoCmd_Dlstart(phost);\n'
    if not clearFound:
        export += '\tApp_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n'
    export += '\n';
                
    specialParameter = ""
    specialParameter2 = ""
    specialCommandType = ""

    for line in document["coprocessor"]:        
        if not line:
            continue

        if (line.lstrip()).startswith("//"): #if the line is a comment line then just write it out
            export += "\t" + line + "\n"
            continue
        cmd_name, functionArgs, cmd_comment = parseCommand(line, functionMap, convertArgs)
        
        coprocessor_cmd = False
        if cmd_name in functionMap.values():
            coprocessor_cmd = True
        if cmd_name == "Gpu_CoCmd_Snapshot2":
            export += "\tApp_WrCoCmd_Buffer(phost, DISPLAY());\n"
            export += "\tGpu_CoCmd_Swap(phost);\n"
            export += "\t/* Download the commands into fifo */\n"
            export += "\tApp_Flush_Co_Buffer(phost);\n"
            export += "\t/* Wait till coprocessor completes the operation */\n"
            export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
            export += "\tGpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n"
            export += "\tGpu_CoCmd_Snapshot2(phost, " + functionArgs + ");\n"
            export += "\tGpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n"
            export += "\tGpu_CoCmd_Dlstart(phost);\n"
            export += "\tApp_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));\n"
            export += "\tApp_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n"
            export += "\tApp_WrCoCmd_Buffer(phost, COLOR_RGB(0xff, 0xff, 0xff));\n"

        if cmd_name == "Gpu_CoCmd_Snapshot":
            export += "\tApp_WrCoCmd_Buffer(phost, DISPLAY());\n"
            export += "\tGpu_CoCmd_Swap(phost);\n"
            export += "\t/* Download the commands into fifo */\n"
            export += "\tApp_Flush_Co_Buffer(phost);\n"
            export += "\t/* Wait till coprocessor completes the operation */\n"
            export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
            export += "\tGpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n"

            export += "\tGpu_Hal_Wr16(phost, REG_HSIZE, " + str(document["registers"]["hSize"]) + ");\n"
            export += "\tGpu_Hal_Wr16(phost, REG_VSIZE, " + str(document["registers"]["vSize"]) + ");\n"
            export += "\tGpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n"

            export += "\t/* Take snap shot of the current screen */\n"
            export += "\tGpu_Hal_WrCmd32(phost, CMD_SNAPSHOT);\n"
            export += "\tGpu_Hal_WrCmd32(phost, " + functionArgs + ");\n"

            export += "\t//timeout for snapshot to be performed by coprocessor\n"
            export += "\t/* Wait till coprocessor completes the operation */\n"
            export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
            export += "\tGpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n"
            export += "\t/* reconfigure the resolution wrt configuration */\n"
            export += "\tGpu_Hal_Wr16(phost, REG_HSIZE, 800);\n"
            export += "\tGpu_Hal_Wr16(phost, REG_VSIZE, 480);\n"
            export += "\tGpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n"
            export += "\tGpu_CoCmd_Dlstart(phost);\n"
            export += "\tApp_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));\n"
            export += "\tApp_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n"
            export += "\tApp_WrCoCmd_Buffer(phost, COLOR_RGB(0xff, 0xff, 0xff));\n"

            cmd_name = ""

        if cmd_name == "Gpu_CoCmd_LoadImage":
            functionArgsSplit = functionArgs.split(',')
            
            if "OPT_FLASH" in functionArgsSplit[1]:
                globalContext['loadImageFromFlash'] = True
                
            if "OPT_MEDIAFIFO" in functionArgsSplit[1]:
                globalContext['mediaFIFOEnabled'] = True
                export += "\tApp_Flush_Co_Buffer(phost);\n"
                export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
            
            export += "\tGpu_CoCmd_LoadImage(phost, " + functionArgsSplit[0] + ", " + functionArgsSplit[1] + ");\n"
                
            functionArgsSplit[2] = re.sub(r'["]', "", functionArgsSplit[2])
            specialParameter = os.path.split(functionArgsSplit[2])[1]                    
            specialParameter2 = specialParameter
            filesToTestFolder.append(functionArgsSplit[2])
            specialParameter = "..\\\\..\\\\..\\\\Test\\\\" + specialParameter
            specialCommandType = cmd_name
            cmd_name = ""

        if cmd_name == "Gpu_CoCmd_MediaFifo":
            functionArgsSplit = functionArgs.split(',')
            functionArgs = functionArgsSplit[0] + ","
            functionArgs += functionArgsSplit[1]
            specialCommandType = "Gpu_CoCmd_MediaFifo"
            globalContext['mediaFIFOAddress'] = functionArgsSplit[0]
            globalContext['mediaFIFOLength'] = functionArgsSplit[1]

        if cmd_name == "Gpu_CoCmd_Dlstart":
            export += '\tGpu_CoCmd_Dlstart(phost);\n'
            export += '\tApp_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n'
            cmd_name = ""

        if cmd_name == "Gpu_CoCmd_PlayVideo":
            export += "\n"
            functionArgsSplit = functionArgs.split(',')
            functionArgs = functionArgsSplit[0]
            
            if 'OPT_FLASH' in functionArgsSplit[0]:
                globalContext['loadVideoFromFlash'] = True
                
            if 'OPT_MEDIAFIFO' in functionArgsSplit[0]:
                globalContext['mediaFIFOEnabled'] = True
                export += "\tApp_Flush_Co_Buffer(phost);\n"
                export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
            
            export += "\tGpu_CoCmd_PlayVideo(phost, " + functionArgsSplit[0] + ");\n"                

            functionArgsSplit[1] = re.sub(r'["]', "", functionArgsSplit[1])
            specialParameter = os.path.split(functionArgsSplit[1])[1]
            
            filesToTestFolder.append(functionArgsSplit[1])                
            
            specialParameter2 = specialParameter
            specialParameter = "..\\\\..\\\\..\\\\Test\\\\" + specialParameter
            specialCommandType = cmd_name
            cmd_name = ""

        #The following commands don't take any parameters so there shouldn't be a comma after the phost
        parameterComma = ", "
        if cmd_name in ["Gpu_CoCmd_LoadIdentity", "Gpu_CoCmd_Swap", "Gpu_CoCmd_Stop", "Gpu_CoCmd_SetMatrix", "Gpu_CoCmd_ColdStart", "Gpu_CoCmd_Dlstart", "Gpu_CoCmd_ScreenSaver", "Gpu_CoCmd_VideoStartF"]:
            parameterComma = ""
            
        if cmd_name == "Gpu_CoCmd_FlashFast" and functionArgs == "":
            functionArgs = '0'
        if cmd_name == "Gpu_CoCmd_MemCrc":
            functionArgs = functionArgs + ", 0"
        if cmd_name == "Gpu_CoCmd_Nop":
            export += '\tGpu_CoCmd_Nop(phost);\n'
            cmd_name = ""
        if cmd_name == "Gpu_CoCmd_TestCard":
            export += '\tGpu_CoCmd_TestCard(phost);\n'
            cmd_name = ""
        if cmd_name == "Gpu_CoCmd_Return":
            export += '\tGpu_CoCmd_Return(phost);\n'
            cmd_name = ""
        if cmd_name == "Gpu_CoCmd_EndList":
            export += '\tGpu_CoCmd_EndList(phost);\n'
            cmd_name = ""
            
        #Attempt to append comments
        comments = cmd_comment
        if len(cmd_name):
            if coprocessor_cmd:
                newline = "\t" + cmd_name + "(phost" + parameterComma + functionArgs + ");" + comments + "\n"
            else:
                newline = "\tApp_WrCoCmd_Buffer(phost" + parameterComma + cmd_name + "(" + functionArgs + "));" + comments + "\n"
            export += newline

        if specialCommandType == "Gpu_CoCmd_LoadImage":
            if globalContext['mediaFIFOEnabled'] == True:
                export += "\t#if defined(FT900_PLATFORM)\n"
                export += "\tloadDataToCoprocessorMediafifo(phost, \"" + specialParameter2 + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n"
                export += "\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n"
                export += "\tloadDataToCoprocessorMediafifo(phost, \"" + specialParameter + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n"
                export += "\t#endif\n"
                export += "\tApp_Flush_Co_Buffer(phost);\n"
                globalContext['mediaFIFOEnabled'] = False
            elif globalContext['loadImageFromFlash'] == False:
                export += "\t#if defined(FT900_PLATFORM)\n"
                export += "\tloadDataToCoprocessorMediafifo(phost, \"" + specialParameter2 + "\");\n"
                export += "\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n"
                export += "\tloadDataToCoprocessorCMDfifo(phost, \"" + specialParameter + "\");\n"
                export += "\t#endif\n"
                export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
                
            globalContext['loadImageFromFlash'] = False
            specialCommandType = ""
        elif specialCommandType == "Gpu_CoCmd_PlayVideo":
            if globalContext['mediaFIFOEnabled'] == True:
                export += "\t#if defined(FT900_PLATFORM)\n"
                export += "\tloadDataToCoprocessorMediafifo(phost, \"" + specialParameter2 + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n"
                export += "\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n"
                export += "\tloadDataToCoprocessorMediafifo(phost, \"" + specialParameter + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n"
                export += "\t#endif\n"
                globalContext['mediaFIFOEnabled'] = False
            elif globalContext['loadVideoFromFlash'] == False:
                export += "\t#if defined(FT900_PLATFORM)\n"
                export += "\tloadDataToCoprocessorCMDfifo_nowait(phost, \"" + specialParameter2 + "\");\n"
                export += "\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n"
                export += "\tloadDataToCoprocessorCMDfifo_nowait(phost, \"" + specialParameter + "\");\n"
                export += "\t#endif\n"
                export += "\tGpu_Hal_WaitCmdfifo_empty(phost);\n"
                
            globalContext['loadVideoFromFlash'] = False
            specialCommandType = ""
        else:
            specialCommandType = ""
   
    return export

    
