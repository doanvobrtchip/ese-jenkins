import os, sys, re, shutil, subprocess, errno, stat
from export_common import parseCommand
from export_pico_reg import *

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
    'skeletonProjectName': "Pico",
    'assetsFolder': "assets",
}

def convertArgs(functionArgs):
    for k, v in RegMap.items():
        functionArgs = functionArgs.replace(k, v)
    functionArgs = functionArgs.replace("eve.eve.", "eve.")    
    return functionArgs

def raiseUnicodeError(errorArea):
    raise Exception("Unable to export project: unicode characters are currently unsupported.  Please check: " + errorArea)
    
def exportContent(outDir, document):
    global lutSize
    export = ''
    need_break = False
    for content in document["content"]:
        if "imageFormat" not in content:
            content["imageFormat"] = -1
        try:
            if content["memoryLoaded"]:
                need_break = True
                if content["imageFormat"] in palettedFormats:
                    ramOffset = int(content["memoryAddress"])
                    lutAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper() + "_LUT"
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()

                    if ((content["imageFormat"] == paletted565_format) or
                        (content["imageFormat"] == paletted4444_format)):
                        lutSize = 512

                    export += f'{lutAddress} = {ramOffset}\n'
                    export += f'{memoryAddress} = {ramOffset + lutSize}\n'
                else:
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                    export += f'{memoryAddress} = {content["memoryAddress"]}\n'
            if content["dataStorage"] == "Embedded":
                contentName = re.sub(r'[-/. ]', '_', content["destName"])

                # copy asset files
                ext = 'bin' if content["dataCompressed"] else 'raw'

                content["exportRawName"] = f'{globalValue["assetsFolder"]}/{contentName}.{ext}'
                shutil.copy(f'{content["destName"]}.{ext}', f'{outDir}/{content["exportRawName"]}')
                
                if content["imageFormat"] in palettedFormats:
                    content["exportLutName"] = f'{globalValue["assetsFolder"]}/{contentName}.lut.{ext}'
                    shutil.copy(f'{content["destName"]}.lut.{ext}', f'{outDir}/{content["exportLutName"]}')
                    content["exportLutName"]
                
        except (UnicodeDecodeError, UnicodeEncodeError):
            raiseUnicodeError("Name of the assets")
    
    if need_break:
        export += '\n'
    return export
    
def exportLoadImageCommand(document):
    
    export = ''
    need_flush = False
    for content in document["content"]:
        if not content["memoryLoaded"]:
            continue
        memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
        lutMemoryAddress = memoryAddress + "_LUT"
        if content["dataStorage"] == "Embedded":
            need_flush = True
            contentName = content["exportRawName"]
            
            if content["dataCompressed"]:
                export += f'em.cmd_inflate({memoryAddress})\n'
                export += f'em.load(open("{contentName}", "rb"))\n'
                if content["imageFormat"] in palettedFormats:
                    lutContentName = content["exportLutName"]	
                    export += f'em.cmd_inflate({lutMemoryAddress})\n'
                    export += f'em.load(open("{lutContentName}", "rb"))\n'                       
            else:
                export += f'em.wr({memoryAddress}, open("{contentName}", "rb").read())\n'
                if content["imageFormat"] in palettedFormats:
                    export += f'em.wr({lutMemoryAddress}, open("{lutContentName}", "rb").read())\n'      

    if need_flush:
        export += 'em.flush()\n'
    export += '\n'            
    return export
    
def exportCoprocessorCommand(document, filesToTestFolder):
    export = ''
    clearFound = False

    try:
        for line in document["coprocessor"]:
            if not line:
                continue
            try:
                cmd_name, _, _ = parseCommand(line, functionMap, convertArgs)
                if 'CLEAR' in cmd_name:
                    clearFound = True
                if cmd_name == functionMap["CMD_CALIBRATE"]:
                    export += 'em.calibrate()\n'                    
                    document["coprocessor"].remove(line)                    
                elif cmd_name == functionMap["CMD_LOGO"]:
                    export += 'em.cmd_logo()\n'                    
                    document["coprocessor"].remove(line)
            except:
                pass

        export += 'em.cmd_flashfast()\n'
        export += 'em.cmd_dlstart()\n'
        if not clearFound:
            export += 'em.Clear(1, 1, 1)\n'
        export += '\n'
                    
        specialParameter = ""
        specialCommandType = ""

        for line in document["coprocessor"]:        
            if not line:
                export += '\n'                
                continue

            if (line.lstrip()).startswith("//"):
                #if the line is a comment line then just write it out
                export += line.replace("//", "#") + "\n"
                continue

            cmd_name, functionArgs, cmd_comment = parseCommand(line, functionMap, convertArgs)
            if cmd_comment:
                cmd_comment = "# " + cmd_comment

            coprocessor_cmd = False
            if cmd_name in functionMap.values():
                coprocessor_cmd = True
            
            if cmd_name == functionMap['CMD_KEYS']:
                fa = re.sub(r"'(.)'", r"ord('\1')", functionArgs)
                if fa == functionArgs:
                    raise Exception("Found invalid option in CMD_KEYS!")
                functionArgs = fa
            if cmd_name == functionMap['CMD_SNAPSHOT2']:
                export += "em.Display()\n"
                export += "em.cmd_swap()\n"
                export += "# Download the commands into fifo\n"
                export += "em.flush()\n"
                export += "# Wait till coprocessor completes the operation\n"
                export += "import time\n"
                export += "time.sleep(0.1)\n"            
                export += f"em.cmd_snapshot2({functionArgs})\n"
                export += "time.sleep(0.1) # timeout for snapshot to be performed by coprocessor\n"
                export += "em.cmd_dlstart()\n"
                export += "em.ClearColorRGB(0xff, 0xff, 0xff)\n"
                export += "em.Clear(1, 1, 1)\n"
                export += "em.ColorRGB(0xff, 0xff, 0xff)\n"
                cmd_name = ""

            if cmd_name == functionMap['CMD_SNAPSHOT']:
                export += "em.Display()\n"
                export += "em.cmd_swap()\n"
                export += "# Download the commands into fifo\n"
                export += "em.flush()\n"
                export += "# Wait till coprocessor completes the operation\n"
                export += "import time\n"
                export += "time.sleep(0.1)\n"
                
                export += f'em.cmd_regwrite(REG_HSIZE, {document["registers"]["hSize"]})\n'
                export += f'em.cmd_regwrite(REG_VSIZE, {document["registers"]["vSize"]})\n'
                export += "time.sleep(0.1)\n"
                
                export += f"em.cmd_snapshot({functionArgs})\n"
                export += "time.sleep(0.1) # timeout for snapshot to be performed by coprocessor\n"
                export += "em.cmd_dlstart()\n"
                export += "em.ClearColorRGB(0xff, 0xff, 0xff)\n"
                export += "em.Clear(1, 1, 1)\n"
                export += "em.ColorRGB(0xff, 0xff, 0xff)\n"

                cmd_name = ""

            if cmd_name == functionMap['CMD_LOADIMAGE']:
                functionArgsSplit = functionArgs.split(',')
                
                if "OPT_FLASH" in functionArgsSplit[1]:
                    globalContext['loadImageFromFlash'] = True
                    
                if "OPT_MEDIAFIFO" in functionArgsSplit[1]:
                    globalContext['mediaFIFOEnabled'] = True
                    export += "em.flush()\n"
                
                export += f"em.cmd_loadimage({functionArgsSplit[0]}, {functionArgsSplit[1]})\n"
                    
                functionArgsSplit[2] = re.sub(r'["]', "", functionArgsSplit[2])
                specialParameter = os.path.split(functionArgsSplit[2])[1]                    
                filesToTestFolder.append(functionArgsSplit[2])
                specialParameter =  f'./{globalValue["assetsFolder"]}/{specialParameter}' 
                specialCommandType = cmd_name
                cmd_name = ""

            if cmd_name == functionMap["CMD_MEDIAFIFO"]:
                functionArgsSplit = functionArgs.split(',')
                functionArgs = functionArgsSplit[0] + ","
                functionArgs += functionArgsSplit[1]
                specialCommandType = cmd_name
                globalContext['mediaFIFOAddress'] = functionArgsSplit[0]
                globalContext['mediaFIFOLength'] = functionArgsSplit[1]

            if cmd_name == functionMap["CMD_DLSTART"]:
                export += "em.DlStart()\n"
                export += "em.Clear(1, 1, 1)\n"
                cmd_name = ""

            if cmd_name == functionMap["CMD_PLAYVIDEO"]:
                export += "\n"
                functionArgsSplit = functionArgs.split(',')
                functionArgs = functionArgsSplit[0]
                
                if 'OPT_FLASH' in functionArgsSplit[0]:
                    globalContext['loadVideoFromFlash'] = True
                    
                if 'OPT_MEDIAFIFO' in functionArgsSplit[0]:
                    globalContext['mediaFIFOEnabled'] = True
                    export += "em.flush()\n"
                
                export += f"em.cmd_playvideo({functionArgsSplit[0]})\n"                

                functionArgsSplit[1] = re.sub(r'["]', "", functionArgsSplit[1])
                specialParameter = os.path.split(functionArgsSplit[1])[1]
                
                filesToTestFolder.append(functionArgsSplit[1])                
                
                specialParameter = f'./{globalValue["assetsFolder"]}/{specialParameter}'
                specialCommandType = cmd_name
                cmd_name = ""
                
            #Attempt to append comments


            if cmd_name:
                if coprocessor_cmd:
                    newline = f"em.{cmd_name}({functionArgs}) {cmd_comment}\n"
                elif cmd_name in DlMap:
                    if cmd_name == "VERTEX2F":
                        functionArgs = ', '.join(str(int(x) // 16) for x in functionArgs.split(','))
                    newline = f"em.{DlMap[cmd_name]}({functionArgs}) {cmd_comment}\n"
                export += newline

            if specialCommandType == functionMap["CMD_LOADIMAGE"]:
                if globalContext['mediaFIFOEnabled'] == True:     
                    #TODO           
                    globalContext['mediaFIFOEnabled'] = False
                elif globalContext['loadImageFromFlash'] == False:
                    export += f'f = open("{specialParameter}", "rb")\n'
                    export += "em.load(f)\n"
                    export += "em.flush()\n"
                    
                    globalContext['loadImageFromFlash'] = False
                specialCommandType = ""
            elif specialCommandType == functionMap["CMD_PLAYVIDEO"]:
                if globalContext['mediaFIFOEnabled'] == True:
                    #TODO
                    globalContext['mediaFIFOEnabled'] = False
                elif globalContext['loadVideoFromFlash'] == False:
                    #TODO
                    globalContext['loadVideoFromFlash'] = False
                specialCommandType = ""
            else:
                specialCommandType = ""
    except Exception as e:
        exc_type, _, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        raise Exception(exc_type, fname, exc_tb.tb_lineno, str(e))
   
    export += '\nem.swap()\n'
    return export

    
