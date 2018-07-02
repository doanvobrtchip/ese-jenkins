import os, sys, re, shutil, subprocess, errno, stat, codecs
import export_bt81x_helper
from export_bt81x_helper import globalValue

def renameAllItemsInFolder(folder, from_name, to_name):    
    for item in os.listdir(folder):
        if os.path.isdir(folder + '/' + item):
            renameAllItemsInFolder(folder + '/' + item, from_name, to_name)
        if from_name in item:
            new = item.replace(from_name, to_name)
            os.rename(folder + '/' + item, folder + '/' + new)
            
def replaceStringInFile(file, oldString, newString):
    with open(file, 'r') as resourceFile:
        data = resourceFile.read()

    data = data.replace(oldString, newString)
    
    with open(file, 'w+', encoding='utf-8') as resourceFile:
        resourceFile.truncate()
        resourceFile.write(data)
   
def copydir2(source, dest):
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
        raise IOError("Unable to generate a complete MSVC project. Try again and make sure the folders and files of the project directory are accessible.. " + str(ioStatus))
    except ValueError as valueStatus:
        raise ValueError("The path of the files in the generated project have exceeded the maximum allowed length for the current platform.  Rename the current project and/or move the project closer to your home directory. " + str(valueStatus))
    except shutil.Error as exc:
        errors = exc.args[0]
        cpSrc, cpDst, cpMsg = errors[0]
        raise Exception("Project generation error: " + str(cpMsg))
    except Exception as e:
        raise Exception("Error while generating project. " + str(e))

def replaceProjectName(folder, from_name, to_name):
    
    ext = ['.project', '.cproject', '.sln', '.vcxproj', '.filters']
    
    for item in os.listdir(folder):
        if os.path.isdir(folder + '/' + item):
            replaceProjectName(folder + '/' + item, from_name, to_name)
        if item[item.rfind('.'):] in ext:
            replaceStringInFile(folder + '/' + item, from_name, to_name)
            
def generateProjectFiles(destDir, projectName, filesToTestFolder, moduleName):

    defaultSkeletonProjectDir = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + globalValue['skeletonProjectName']
    if os.path.isdir(defaultSkeletonProjectDir):
        copydir2(defaultSkeletonProjectDir, destDir)
        renameAllItemsInFolder(destDir, globalValue['skeletonProjectName'], projectName)
    else:
        raise Exception("Required program files are missing.")
   
    replaceProjectName(destDir, globalValue['skeletonProjectName'], projectName)
    
    MSVCPlatformHeader = destDir + '/Hdr/Msvc/Platform.h'
    EmulatorPlatformHeader = destDir + '/Hdr/Msvc_Emulator/Platform.h'
    Ft90xPlatformHeader = destDir + '/Hdr/FT90x/Platform.h'
    Ft93xPlatformHeader = destDir + '/Hdr/FT93x/Platform.h'
    
    try:
        if moduleName == "VM816C50A_MPSSE":
            replaceStringInFile(MSVCPlatformHeader, "//#define VM816C50A_MPSSE", "#define VM816C50A_MPSSE")
            replaceStringInFile(EmulatorPlatformHeader, "//#define VM816C50A_MPSSE", "#define VM816C50A_MPSSE")
            replaceStringInFile(Ft90xPlatformHeader, "//#define VM816C50A_MPSSE", "#define VM816C50A_MPSSE")
            replaceStringInFile(Ft93xPlatformHeader, "//#define VM816C50A_MPSSE", "#define VM816C50A_MPSSE")
        elif moduleName == "VM816C50A_LIBFT4222":
            replaceStringInFile(MSVCPlatformHeader, "//#define VM816C50A_LIBFT4222", "#define VM816C50A_LIBFT4222")
            replaceStringInFile(EmulatorPlatformHeader, "//#define VM816C50A_LIBFT4222", "#define VM816C50A_LIBFT4222")
            replaceStringInFile(Ft90xPlatformHeader, "//#define VM816C50A_LIBFT4222", "#define VM816C50A_LIBFT4222")
            replaceStringInFile(Ft93xPlatformHeader, "//#define VM816C50A_LIBFT4222", "#define VM816C50A_LIBFT4222")        
    except Exception as e:
        raise Exception("Error while renaming project platform files: " + str(e))

    for content in filesToTestFolder:
        destinationName = ""
        content = content.strip()
        if '/' in content:
            destinationName = content.rsplit('/', 1)[1]
        elif '\\' in content:
            destinationName = content.rsplit('\\', 1)[1]
        try:
            shutil.copy(content, destDir + '/' + projectName + '/' + globalValue['assetsFolder'] + '/' + destinationName)
        except Exception as e:
            raise Exception("Error copying assets to project folder: " + str(e))

def run(projectName, document, ram, moduleName):
    deviceType = document["project"]["device"]
    resultText = "<b>EVE HAL Export</b><br>"
    for line in document["displayList"]:
        if not line == "":
            resultText += "<b>Warning</b>: Only support for Coprocessor commands, ignoring Display List.<br>"
            break

    if deviceType in [2069, 2070]:
        outDir = projectName + "_BT81X_EVE_HAL"            
    else:
        raise Exception("Board is currently unsupported: " + moduleName)

    try:
        if os.path.isdir(outDir):
            try:
                shutil.rmtree(outDir)
            except shutil.Error as exc:
                errors = exc.args[0]
                cpSrc, cpDst, cpMsg = errors[0]
                raise Exception("Project generation error: " + str(cpMsg))
            except IOError as ioStatus:
                raise Exception("Unable to generate a complete MSVC project. Please make sure the previously generated project files and folder are not currently being accessed. " + str(ioStatus))
        os.makedirs(outDir)
    except Exception as e:
        raise IOError("Unable to generate a complete MSVC project. Try again and make sure the previous generated project files and skeleton project files are not currently being accessed. " + str(e))
       
    try:           
        filesToTestFolder = []
         
        exportAssets = export_bt81x_helper.exportContent(outDir, document)
        
        export = export_bt81x_helper.exportLoadImageCommand(document)    

        export += export_bt81x_helper.exportCoprocessorCommand(document, filesToTestFolder)

        generateProjectFiles(outDir, projectName, filesToTestFolder, moduleName)
        
        skeletonFilePath = outDir + '/Src/Skeleton.c'
                   
        #export = export.encode('utf-8')
        
        replaceStringInFile(skeletonFilePath, '/*RESERVED_FOR_EXPORTING_FROM_ESE*/', export)
        
        assetsHeaderPath = outDir + '/Hdr/Assets.h'
        replaceStringInFile(assetsHeaderPath, '/*RESERVED_FOR_EXPORTING_FROM_ESE*/', exportAssets)

        resultText += "<p>Output files: " + skeletonFilePath + "</p> <p>Project files: " + outDir + "</p><p>ReadMe file: " + outDir + "/ReadMe.txt, details the project folder structure</p>"

        if sys.platform.startswith('darwin'):
            subprocess.call(('open', outName))
        elif os.name == 'nt':
            subprocess.call(['explorer', outDir])
        elif os.name == 'posix':
            subprocess.call(('xdg-open', outName))
    
    except Exception as e:
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        raise Exception(exc_type, fname, exc_tb.tb_lineno, str(e))
 
    return resultText

