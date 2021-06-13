import os, sys, re, shutil, subprocess, errno, stat, codecs
import export_pico_helper as helper
from export_pico_helper import globalValue

TEMPLATE_PATH = 'export_template/circuitPython'

def replaceStringInFile(file, oldString, newString):
    with open(file, 'r') as resourceFile:
        data = resourceFile.read()

    data = data.replace(oldString, newString)
    
    with open(file, 'w+', encoding='utf-8') as resourceFile:
        resourceFile.truncate()
        resourceFile.write(data)
   
def copydir(source, dest):
    try:
        for item in os.listdir(source):
            s = os.path.join(source, item)
            d = os.path.join(dest, item)
            if len(d) > 255 or len(s) > 255:
                raise ValueError
            if os.path.isdir(s):
                shutil.copytree(s, d, symlinks=False, ignore=None)
            else:
                shutil.copy2(s, d)
    except IOError as ioStatus:
        raise IOError("Unable to generate a complete project. Try again and make sure the folders and files of the project directory are accessible.. " + str(ioStatus))
    except ValueError as valueStatus:
        raise ValueError("The path of the files in the generated project have exceeded the maximum allowed length for the current platform.  Rename the current project and/or move the project closer to your home directory. " + str(valueStatus))
    except shutil.Error as exc:
        errors = exc.args[0]
        cpSrc, cpDst, cpMsg = errors[0]
        raise Exception("Project generation error: " + str(cpMsg))
    except Exception as e:
        raise Exception("Error while generating project. " + str(e))

def generateProjectFiles(destDir, filesToTestFolder):
   
    for content in filesToTestFolder:
        destinationName = ""
        content = content.strip()
        if not content:
            continue
            

        if '/' in content:
            destinationName = content.rsplit('/', 1)[1]
        elif '\\' in content:
            destinationName = content.rsplit('\\', 1)[1]
            
        try:
            shutil.copy(content, destDir + '/' + globalValue['assetsFolder'] + '/' + destinationName)
        except Exception as e:
            raise Exception("Error copying assets to project folder: " + str(e))
            
def run(projectName, document, ram, moduleName, screenResolution):
    
    deviceType = document["project"]["device"]
    if deviceType not in [2069, 2070, 2071, 2072]:
        raise Exception("Board is currently unsupported: " + moduleName)
        
    resultText = "<b>EVE HAL Export</b><br>"
    
    for line in document["displayList"]:
        if line:
            resultText += "<b>Warning</b>: Only support for Coprocessor commands, ignoring Display List.<br>"
            break   
        
    outDir = f'{projectName}_{moduleName}'
    
    try:
        if os.path.isdir(outDir):
            try:
                shutil.rmtree(outDir)
            except shutil.Error as exc:
                errors = exc.args[0]
                _, _, cpMsg = errors[0]
                raise Exception("Project generation error: " + str(cpMsg))
            except IOError as ioStatus:
                raise Exception("Unable to generate a complete project. Please make sure the previously generated project files and folder are not currently being accessed. " + str(ioStatus))
        os.makedirs(outDir)
    except Exception as e:
        raise IOError("Unable to generate a complete project. Try again and make sure the previous generated project files and skeleton project files are not currently being accessed. " + str(e))
    
    ese_folder = os.path.dirname(os.path.abspath(__file__))
    copydir( ese_folder + '/' + TEMPLATE_PATH, outDir)
    
    try:           
        filesToTestFolder = []
         
        export = helper.exportContent(outDir, document)
        
        export += helper.exportLoadImageCommand(document)    
        export += helper.exportCoprocessorCommand(document, filesToTestFolder)

        generateProjectFiles(outDir, filesToTestFolder)
        
        skeletonFilePath = outDir + '/code.py'
        
        replaceStringInFile(skeletonFilePath, '#RESERVED_FOR_SCREEN_SIZE#', screenResolution)
                           
        replaceStringInFile(skeletonFilePath, '#RESERVED_FOR_ESE#', export)
        
        #assetsHeaderPath = outDir + '/Hdr/Assets.h'
        #replaceStringInFile(assetsHeaderPath, '/*RESERVED_FOR_EXPORTING_FROM_ESE*/', exportAssets)

        resultText += "<p>Output files: " + skeletonFilePath + "</p> <p>Project files: " + outDir + "</p><p>ReadMe file: " + outDir + "/ReadMe.txt, details the project folder structure</p>"

        subprocess.call(['explorer', outDir])
    
    except Exception as e:
        exc_type, _, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        raise Exception(exc_type, fname, exc_tb.tb_lineno, str(e))
 
    return resultText
    
    