import os, sys, re, shutil, subprocess, errno, stat
from export_common import parseCommand

paletted_format = 8		
paletted8_format = 16		
paletted565_format = 14		
paletted4444_format = 15

def displayName():
    return "EVE HAL 2.0 Project"

def renameStringInFile(file, oldName, newName):
    with open(file, 'r') as resourceFile:
        data = resourceFile.read()

    data = data.replace(oldName, newName)

    with open(file, 'w+') as resourceFile:
        resourceFile.truncate()
        resourceFile.write(data)

def copydir(source, dest):

    for root, dirs, files in os.walk(source):
        if not os.path.isdir(root):
            os.makedirs(root)
        for each_file in files:
            rel_path = root.replace(source, '').lstrip(os.sep)
            dest_path = os.path.join(dest, rel_path, each_file)
            shutil.copyfile(os.path.join(root, each_file), dest_path)

def readOnlyDirectory(operation, name, exc):
    os.chmod(name, stat.S_IWRITE)
    return True

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

def constructHostPath(folderNames):
    fullPath = ""
    for folder in foldernames:
        fullPath += folder
        fullPath += os.path.sep
    return fullPath


def generateProjectFiles(destDir, srcFile, projectName, filesToTestFolder, moduleName):
    skeletonProjectDir = destDir + os.path.sep + projectName
    MSVCSolutionName = skeletonProjectDir + os.path.sep + "Project" + os.path.sep + "Msvc_win32" + os.path.sep + projectName + os.path.sep + projectName
    EmulatorSolutionName = skeletonProjectDir + os.path.sep + "Project" + os.path.sep + "Msvc_Emulator" + os.path.sep + projectName + os.path.sep + projectName
    MSVCPlatformHeader = skeletonProjectDir + os.path.sep + "Hdr" + os.path.sep + "Msvc_win32" + os.path.sep + "FT_Platform.h"
    EmulatorPlatformHeader = skeletonProjectDir + os.path.sep + "Hdr" + os.path.sep + "Msvc_Emulator" + os.path.sep + "FT_Platform.h"
    Ft90xPlatformHeader = skeletonProjectDir + os.path.sep + "Hdr" + os.path.sep + "FT90x" + os.path.sep + "FT_Platform.h"

    MSVCSolution = MSVCSolutionName + ".sln"
    MSVCProject = MSVCSolutionName + ".vcxproj"
    MSVCFilter = MSVCSolutionName + ".vcxproj.filters"

    EmulatorSolution = EmulatorSolutionName + ".sln"
    EmulatorProject = EmulatorSolutionName + ".vcxproj"
    EmulatorFilter = EmulatorSolutionName + ".vcxproj.filters"

    Ft90xCProject = skeletonProjectDir + os.path.sep + "Project" + os.path.sep + "FT90x" + os.path.sep + projectName + os.path.sep + ".cproject"
    Ft90xProject = skeletonProjectDir + os.path.sep + "Project" + os.path.sep + "FT90x" + os.path.sep + projectName + os.path.sep + ".project"



    defaultSkeletonProjectDir = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + globalValue['skeletonProjectName']
    if os.path.isdir(defaultSkeletonProjectDir):
        copydir2(defaultSkeletonProjectDir, skeletonProjectDir)
    else:
        raise Exception("Required program files are missing.")

    try:
        shutil.copy(srcFile, skeletonProjectDir + os.path.sep + "Src" + os.path.sep + projectName + ".c") #copy source file
        #rename Msvc_win32 project files
        os.rename(skeletonProjectDir + "/Project/Msvc_win32/untitled", skeletonProjectDir + "/Project/Msvc_win32/" + projectName)
        os.rename(skeletonProjectDir + "/Project/Msvc_win32/" + projectName + "/untitled.sln", MSVCSolution) #rename project solution
        os.rename(skeletonProjectDir + "/Project/Msvc_win32/" + projectName + "/untitled.vcxproj", MSVCProject) #rename project
        os.rename(skeletonProjectDir + "/Project/Msvc_win32/" + projectName + "/untitled.vcxproj.filters", MSVCFilter) #rename project filter
        #rename MSVC_Emulator project files
        os.rename(skeletonProjectDir + "/Project/Msvc_Emulator/untitled", skeletonProjectDir + "/Project/Msvc_Emulator/" + projectName)
        os.rename(skeletonProjectDir + "/Project/Msvc_Emulator/" + projectName + "/untitled.sln", EmulatorSolution) #rename project solution
        os.rename(skeletonProjectDir + "/Project/Msvc_Emulator/" + projectName + "/untitled.vcxproj", EmulatorProject) #rename project
        os.rename(skeletonProjectDir + "/Project/Msvc_Emulator/" + projectName + "/untitled.vcxproj.filters", EmulatorFilter) #rename project filter
        #rename FT90x project files
        os.rename(skeletonProjectDir + "/Project/FT90x/untitled", skeletonProjectDir + "/Project/FT90x/" + projectName)
        #rename MSVC project files
        renameStringInFile(MSVCSolution, globalValue['skeletonProjectName'], projectName)
        renameStringInFile(MSVCProject, globalValue['skeletonProjectName'], projectName)
        renameStringInFile(MSVCFilter, globalValue['skeletonProjectName'], projectName)
        #rename emulator project files
        renameStringInFile(EmulatorSolution, globalValue['skeletonProjectName'], projectName)
        renameStringInFile(EmulatorProject, globalValue['skeletonProjectName'], projectName)
        renameStringInFile(EmulatorFilter, globalValue['skeletonProjectName'], projectName)
        #rename FT90x project files
        renameStringInFile(Ft90xCProject, globalValue['skeletonProjectName'], projectName)
        renameStringInFile(Ft90xProject, globalValue['skeletonProjectName'], projectName)
        #rename readme.txt
        renameStringInFile(skeletonProjectDir + os.path.sep + "ReadMe.txt", globalValue['skeletonProjectName'], projectName)

        if moduleName == "VM800B43_50" or moduleName == "VM800BU43_50" or moduleName == "VM800C43_50":
            renameStringInFile(MSVCPlatformHeader, "//#define VM800B43_50", "#define VM800B43_50")
            renameStringInFile(EmulatorPlatformHeader, "//#define VM800B43_50", "#define VM800B43_50")
            renameStringInFile(Ft90xPlatformHeader, "//#define DISPLAY_RESOLUTION_WQVGA", "#define DISPLAY_RESOLUTION_WQVGA")
            renameStringInFile(Ft90xPlatformHeader, "//#define FT_800_ENABLE", "#define FT_800_ENABLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ENABLE_SPI_QUAD", "#define ENABLE_SPI_QUAD")
        elif moduleName == "VM800B35" or moduleName == "VM800BU35" or moduleName == "VM800C35":
            renameStringInFile(MSVCPlatformHeader, "//#define VM800B35", "#define VM800B35")
            renameStringInFile(EmulatorPlatformHeader, "//#define VM800B35", "#define VM800B35")
            renameStringInFile(Ft90xPlatformHeader, "//#define DISPLAY_RESOLUTION_QVGA", "#define DISPLAY_RESOLUTION_QVGA")
            renameStringInFile(Ft90xPlatformHeader, "//#define FT_800_ENABLE", "#define FT_800_ENABLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ENABLE_SPI_QUAD", "#define ENABLE_SPI_QUAD")
        elif moduleName == "VM801B43_50":
            renameStringInFile(MSVCPlatformHeader, "//#define VM801B43_50", "#define VM801B43_50")
            renameStringInFile(EmulatorPlatformHeader, "//#define VM801B43_50", "#define VM801B43_50")
            renameStringInFile(Ft90xPlatformHeader, "//#define DISPLAY_RESOLUTION_WQVGA", "#define DISPLAY_RESOLUTION_WQVGA")
            renameStringInFile(Ft90xPlatformHeader, "//#define FT_801_ENABLE", "#define FT_801_ENABLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ENABLE_SPI_QUAD", "#define ENABLE_SPI_QUAD")
        elif moduleName == "VM810C50":
            renameStringInFile(MSVCPlatformHeader, "//#define VM810C50", "#define VM810C50")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(Ft90xPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ENABLE_SPI_QUAD", "#define ENABLE_SPI_QUAD")
        elif moduleName == "ME810A_HV35R":
            renameStringInFile(MSVCPlatformHeader, "//#define VM810C50", "#define VM810C50")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_HVGA_PORTRAIT", "#define DISPLAY_RESOLUTION_HVGA_PORTRAIT")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME810A_HV35R", "#define ME810A_HV35R")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")
        
        ### Newly added here after ESE 2.4.0    
        ### ME81XA modules support -- for MSVC_Win32 platform 
        elif moduleName == "ME811A_WH70C":
            renameStringInFile(MSVCPlatformHeader, "//#define ME811A_WH70C", "#define ME811A_WH70C")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME811A_WH70C", "#define ME811A_WH70C")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")            
        elif moduleName == "ME810A_WH70R":
            renameStringInFile(MSVCPlatformHeader, "//#define ME810A_WH70R", "#define ME810A_WH70R")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME810A_WH70R", "#define ME810A_WH70R")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")
        elif moduleName == "ME812A_WH50R":
            renameStringInFile(MSVCPlatformHeader, "//#define ME812A_WH50R", "#define ME812A_WH50R")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME812A_WH50R", "#define ME812A_WH50R")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")
        elif moduleName == "ME813A_WH50C":
            renameStringInFile(MSVCPlatformHeader, "//#define ME813A_WH50C", "#define ME813A_WH50C")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME813A_WH50C", "#define ME813A_WH50C")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")
            
        ###ME81XAU Modules support, only for FT90X platform            
        elif moduleName == "ME810AU_WH70R":
            renameStringInFile(MSVCPlatformHeader, "//#define ME810AU_WH70R", "#define ME810AU_WH70R")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME810AU_WH70R", "#define ME810AU_WH70R")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")            

        elif moduleName == "ME811AU_WH70C":
            renameStringInFile(MSVCPlatformHeader, "//#define ME811AU_WH70C", "#define ME811AU_WH70C")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_813_ENABLE", "#define FT_813_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME811AU_WH70C", "#define ME811AU_WH70C")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")              
            
        elif moduleName == "ME812AU_WH50R":
            renameStringInFile(MSVCPlatformHeader, "//#define ME812AU_WH50R", "#define ME812AU_WH50R")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_812_ENABLE", "#define FT_812_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME812AU_WH50R", "#define ME812AU_WH50R")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")
        elif moduleName == "ME813AU_WH50C":
            renameStringInFile(MSVCPlatformHeader, "//#define ME813AU_WH50C", "#define ME813AU_WH50C")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WVGA", "#define DISPLAY_RESOLUTION_WVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_813_ENABLE", "#define FT_813_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define ENABLE_SPI_SINGLE", "#define ENABLE_SPI_SINGLE")
            renameStringInFile(Ft90xPlatformHeader, "//#define ME813AU_WH50C", "#define ME813AU_WH50C")
            renameStringInFile(Ft90xPlatformHeader, "//#define MM900EV1A", "#define MM900EV1A")  
   
    except Exception as e:
        raise Exception("Error while renaming exported files. Try to shorten project name and/or move the project closer to your home folder.")

    for content in filesToTestFolder:
        destinationName = ""
        content = content.strip()
        if '/' in content:
            destinationName = content.rsplit('/', 1)[1]
        elif '\\' in content:
            destinationName = content.rsplit('\\', 1)[1]
        #raise Exception(content)
        try:
            shutil.copy(content, destDir + os.path.sep + projectName + os.path.sep + globalValue['assetsFolder'] + os.path.sep + destinationName)
        except Exception as e:
            raise Exception("Error copying assets to project folder: " + str(e))

def convertArgs(functionArgs):
    argsMap = {
        "POINTS":"FTPOINTS",
    }

    functionArgsSplit = functionArgs.split(",")
    for argument in functionArgsSplit:
        if '|' in argument:
            argumentOptions = argument.split('|')
            for argumentOption in argumentOptions:
                if argumentOption.replace(" ", "") in argsMap:
                    functionArgs = functionArgs.replace(argumentOption.replace(" ", ""), argsMap[argumentOption.replace(" ", "")])
        if argument.replace(" ", "") in argsMap:
            functionArgs = functionArgs.replace(argument.replace(" ", ""), argsMap[argument.replace(" ", "")])
    return functionArgs


lutSize = 256*4		
palettedFormats = [paletted_format, paletted8_format, paletted565_format, paletted4444_format]    
    
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
    #"CMD_TOUCH_TRANSFORM" : "Ft_Gpu_CoCmd_TouchTransform",
    "CMD_BITMAP_TRANSFORM" : "Ft_Gpu_CoCmd_Bitmap_Transform",
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

FT8xxInitialConfig = """
/*

Copyright (c) Future Technology Devices International 2014

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.


*/

#include "FT_Platform.h"

#ifdef FT900_PLATFORM
#include "ff.h"
#endif

#define F16(s)        ((ft_int32_t)((s) * 65536))
#define WRITE2CMD(a) Ft_Gpu_Hal_WrCmdBuf(phost,a,sizeof(a))
#define SCRATCH_BUFF_SZ 2048 //increase this value will increase the performance but will use more host RAM space.

/* Global variables for display resolution to support various display panels */
/* Default is WQVGA - 480x272 */
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
ft_char8_t FT_DispCSpread = 1;
ft_char8_t FT_DispDither = 1;

/* Global used for buffer optimization */
Ft_Gpu_Hal_Context_t host,*phost;

ft_uint32_t Ft_CmdBuffer_Index;
ft_uint32_t Ft_DlBuffer_Index;

#ifdef FT900_PLATFORM
FATFS FatFs;
FIL 			 CurFile;
FRESULT          fResult;
SDHOST_STATUS    SDHostStatus;

DWORD get_fattime (void)
{
	/* Returns current time packed into a DWORD variable */
	return 0;
}
#endif

#ifdef BUFFER_OPTIMIZATION
ft_uint8_t  Ft_DlBuffer[FT_DL_SIZE];
ft_uint8_t  Ft_CmdBuffer[FT_CMD_FIFO_SIZE];
#endif
/* Boot up for FT800 followed by graphics primitive sample cases */
/* Initial boot up DL - make the back ground green color */
const ft_uint8_t FT_DLCODE_BOOTUP[12] =
{
    255,0,0,2,//GPU instruction CLEAR_COLOR_RGB
    7,0,0,38, //GPU instruction CLEAR

    0,0,0,0,  //GPU instruction DISPLAY
};
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
#ifdef FT900_PLATFORM
   Ft_Gpu_Hal_WrCmd32(phost,cmd);
#endif
   /* Increment the command index */
   Ft_CmdBuffer_Index += FT_CMD_SIZE;
}

ft_void_t Ft_App_WrDlCmd_Buffer(Ft_Gpu_Hal_Context_t *phost,ft_uint32_t cmd)
{
#ifdef BUFFER_OPTIMIZATION
   /* Copy the command instruction into buffer */
   ft_uint32_t *pBuffcmd;
   pBuffcmd =(ft_uint32_t*)&Ft_DlBuffer[Ft_DlBuffer_Index];
   *pBuffcmd = cmd;
#endif

#ifdef ARDUINO_PLATFORM
   Ft_Gpu_Hal_Wr32(phost,(RAM_DL+Ft_DlBuffer_Index),cmd);
#endif
#ifdef FT900_PLATFORM
   Ft_Gpu_Hal_Wr32(phost,(RAM_DL+Ft_DlBuffer_Index),cmd);
#endif
   /* Increment the command index */
   Ft_DlBuffer_Index += FT_CMD_SIZE;
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

ft_void_t Ft_App_Flush_DL_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION
   if (Ft_DlBuffer_Index> 0)
     Ft_Gpu_Hal_WrMem(phost,RAM_DL,Ft_DlBuffer,Ft_DlBuffer_Index);
#endif
   Ft_DlBuffer_Index = 0;

}

ft_void_t Ft_App_Flush_Co_Buffer(Ft_Gpu_Hal_Context_t *phost)
{
#ifdef  BUFFER_OPTIMIZATION
   if (Ft_CmdBuffer_Index > 0)
     Ft_Gpu_Hal_WrCmdBuf(phost,Ft_CmdBuffer,Ft_CmdBuffer_Index);
#endif
   Ft_CmdBuffer_Index = 0;
}


/* API to give fadeout effect by changing the display PWM from 100 till 0 */
ft_void_t SAMAPP_fadeout()
{
   ft_int32_t i;

    for (i = 100; i >= 0; i -= 3)
    {
        Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);

        Ft_Gpu_Hal_Sleep(2);//sleep for 2 ms
    }
}

/* API to perform display fadein effect by changing the display PWM from 0 till 100 and finally 128 */
ft_void_t SAMAPP_fadein()
{
    ft_int32_t i;

    for (i = 0; i <=100 ; i += 3)
    {
        Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
        Ft_Gpu_Hal_Sleep(2);//sleep for 2 ms
    }
    /* Finally make the PWM 100% */
    i = 128;
    Ft_Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
}


/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
ft_void_t SAMAPP_GPU_DLSwap(ft_uint8_t DL_Swap_Type)
{
    ft_uint8_t Swap_Type = DLSWAP_FRAME,Swap_Done = DLSWAP_FRAME;

    if(DL_Swap_Type == DLSWAP_LINE)
    {
        Swap_Type = DLSWAP_LINE;
    }

    /* Perform a new DL swap */
    Ft_Gpu_Hal_Wr8(phost,REG_DLSWAP,Swap_Type);

    /* Wait till the swap is done */
    while(Swap_Done)
    {
        Swap_Done = Ft_Gpu_Hal_Rd8(phost,REG_DLSWAP);

        if(DLSWAP_DONE != Swap_Done)
        {
            Ft_Gpu_Hal_Sleep(10);//wait for 10ms
        }
    }
}

ft_void_t Ft_BootupConfig()
{
    Ft_Gpu_Hal_Powercycle(phost,FT_TRUE);

        /* Access address 0 to wake up the FT800 */
        Ft_Gpu_HostCommand(phost,FT_GPU_ACTIVE_M);
        Ft_Gpu_Hal_Sleep(20);

        /* Set the clk to external clock */
#ifndef ME800A_HV35R
        Ft_Gpu_HostCommand(phost,FT_GPU_EXTERNAL_OSC);
        Ft_Gpu_Hal_Sleep(10);
#endif

        {
            ft_uint8_t chipid;
            //Read Register ID to check if FT800 is ready.
            chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
            while(chipid != 0x7C)
            {
                chipid = Ft_Gpu_Hal_Rd8(phost, REG_ID);
                ft_delay(100);
            }
    #if defined(MSVC_PLATFORM) || defined (FT900_PLATFORM)
            printf("VC1 register ID after wake up %x\\n",chipid);
    #endif
    }
    /* Configuration of LCD display */
#ifdef DISPLAY_RESOLUTION_QVGA
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
    FT_DispCSpread = 1;
    FT_DispDither = 1;

#endif
#ifdef DISPLAY_RESOLUTION_WVGA
    /* Values specific to QVGA LCD display */
    FT_DispWidth = 800;
    FT_DispHeight = 480;
    FT_DispHCycle =  928;
    FT_DispHOffset = 88;
    FT_DispHSync0 = 0;
    FT_DispHSync1 = 48;
    FT_DispVCycle = 525;
    FT_DispVOffset = 32;
    FT_DispVSync0 = 0;
    FT_DispVSync1 = 3;
    FT_DispPCLK = 2;
    FT_DispSwizzle = 0;
    FT_DispPCLKPol = 1;
    FT_DispCSpread = 0;
    FT_DispDither = 1;
#endif
#ifdef DISPLAY_RESOLUTION_HVGA_PORTRAIT
    /* Values specific to HVGA LCD display */

    FT_DispWidth = 320;
    FT_DispHeight = 480;
    FT_DispHCycle =  400;
    FT_DispHOffset = 40;
    FT_DispHSync0 = 0;
    FT_DispHSync1 = 10;
    FT_DispVCycle = 500;
    FT_DispVOffset = 10;
    FT_DispVSync0 = 0;
    FT_DispVSync1 = 5;
    FT_DispPCLK = 4;
    FT_DispSwizzle = 2;
    FT_DispPCLKPol = 1;
    FT_DispCSpread = 1;
    FT_DispDither = 1;

#endif

#ifdef ME800A_HV35R
    /* After recognizing the type of chip, perform the trimming if necessary */
    Ft_Gpu_ClockTrimming(phost,LOW_FREQ_BOUND);
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
    Ft_Gpu_Hal_Wr16(phost, REG_HSIZE, FT_DispWidth);
    Ft_Gpu_Hal_Wr16(phost, REG_VSIZE, FT_DispHeight);
    Ft_Gpu_Hal_Wr16(phost, REG_CSPREAD, FT_DispCSpread);
    Ft_Gpu_Hal_Wr16(phost, REG_DITHER, FT_DispDither);

#if (defined(ENABLE_FT_800) || defined(ENABLE_FT_810) ||defined(ENABLE_FT_812))
    /* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
    Ft_Gpu_Hal_Wr16(phost, REG_TOUCH_RZTHRESH,RESISTANCE_THRESHOLD);
#endif
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO_DIR,0xff);
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0xff);


    /*It is optional to clear the screen here*/
    Ft_Gpu_Hal_WrMem(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
    Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);


    Ft_Gpu_Hal_Wr8(phost, REG_PCLK,FT_DispPCLK);//after this display is visible on the LCD


#ifdef ENABLE_ILI9488_HVGA_PORTRAIT
    /* to cross check reset pin */
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0xff);
    ft_delay(120);
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0x7f);
    ft_delay(120);
    Ft_Gpu_Hal_Wr8(phost, REG_GPIO,0xff);

    ILI9488_Bootup();

    /* Reconfigure the SPI */
#ifdef FT900_PLATFORM
    printf("after ILI9488 bootup \\n");
    //spi
    // Initialize SPIM HW
    sys_enable(sys_device_spi_master);
    gpio_function(27, pad_spim_sck); /* GPIO27 to SPIM_CLK */
    gpio_function(28, pad_spim_ss0); /* GPIO28 as CS */
    gpio_function(29, pad_spim_mosi); /* GPIO29 to SPIM_MOSI */
    gpio_function(30, pad_spim_miso); /* GPIO30 to SPIM_MISO */

    gpio_write(28, 1);
    spi_init(SPIM, spi_dir_master, spi_mode_0, 4);
#endif

#endif



    /* make the spi to quad mode - addition 2 bytes for silicon */
#ifdef FT_81X_ENABLE
    /* api to set quad and numbe of dummy bytes */
#ifdef ENABLE_SPI_QUAD
    Ft_Gpu_Hal_SetSPI(phost,FT_GPU_SPI_QUAD_CHANNEL,FT_GPU_SPI_TWODUMMY);
#elif ENABLE_SPI_DUAL
    Ft_Gpu_Hal_SetSPI(phost,FT_GPU_SPI_QUAD_CHANNEL,FT_GPU_SPI_TWODUMMY);
#else
    Ft_Gpu_Hal_SetSPI(phost,FT_GPU_SPI_SINGLE_CHANNEL,FT_GPU_SPI_ONEDUMMY);

#endif

#ifdef FT900_PLATFORM
    spi_init(SPIM, spi_dir_master, spi_mode_0, 32);
#ifdef ENABLE_SPI_QUAD
    spi_option(SPIM,spi_option_bus_width,4);
#elif ENABLE_SPI_DUAL
    spi_option(SPIM,spi_option_bus_width,2);
#else
    spi_option(SPIM,spi_option_bus_width,1);
#endif

    spi_option(SPIM,spi_option_fifo_size,64);
    spi_option(SPIM,spi_option_fifo,1);
    spi_option(SPIM,spi_option_fifo_receive_trigger,1);

#endif

#endif
phost->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(phost,REG_CMD_WRITE);

}

"""







FT8xxMainFunctionSetup = """

#if defined MSVC_PLATFORM || defined FT900_PLATFORM
/* Main entry point */
ft_int32_t main(ft_int32_t argc,ft_char8_t *argv[])
#endif
#if defined(ARDUINO_PLATFORM)||defined(MSVC_FT800EMU)
ft_void_t setup()
#endif
{
#ifdef FT900_PLATFORM
    FT900_Config();

    sys_enable(sys_device_sd_card);
    sdhost_init();

            #define GPIO_SD_CLK  (19)
            #define GPIO_SD_CMD  (20)
            #define GPIO_SD_DAT3 (21)
            #define GPIO_SD_DAT2 (22)
            #define GPIO_SD_DAT1 (23)
            #define GPIO_SD_DAT0 (24)
            #define GPIO_SD_CD   (25)
            #define GPIO_SD_WP   (26)
            gpio_function(GPIO_SD_CLK, pad_sd_clk); gpio_pull(GPIO_SD_CLK, pad_pull_none);//pad_pull_none
            gpio_function(GPIO_SD_CMD, pad_sd_cmd); gpio_pull(GPIO_SD_CMD, pad_pull_pullup);
            gpio_function(GPIO_SD_DAT3, pad_sd_data3); gpio_pull(GPIO_SD_DAT3, pad_pull_pullup);
            gpio_function(GPIO_SD_DAT2, pad_sd_data2); gpio_pull(GPIO_SD_DAT2, pad_pull_pullup);
            gpio_function(GPIO_SD_DAT1, pad_sd_data1); gpio_pull(GPIO_SD_DAT1, pad_pull_pullup);
            gpio_function(GPIO_SD_DAT0, pad_sd_data0); gpio_pull(GPIO_SD_DAT0, pad_pull_pullup);
            gpio_function(GPIO_SD_CD, pad_sd_cd); gpio_pull(GPIO_SD_CD, pad_pull_pullup);
            gpio_function(GPIO_SD_WP, pad_sd_wp); gpio_pull(GPIO_SD_WP, pad_pull_pullup);
#endif
    Ft_Gpu_HalInit_t halinit;

    halinit.TotalChannelNum = 1;


    Ft_Gpu_Hal_Init(&halinit);
    host.hal_config.channel_no = 0;
    host.hal_config.pdn_pin_no = FT800_PD_N;
    host.hal_config.spi_cs_pin_no = FT800_SEL_PIN;
#ifdef MSVC_PLATFORM_SPI
    host.hal_config.spi_clockrate_khz = 12000; //in KHz
#endif
#ifdef ARDUINO_PLATFORM_SPI
    host.hal_config.spi_clockrate_khz = 4000; //in KHz
#endif
        Ft_Gpu_Hal_Open(&host);
    phost = &host;

    Ft_BootupConfig();

#if ((defined FT900_PLATFORM) || defined(MSVC_PLATFORM))
    printf("\\n reg_touch_rz =0x%x ", Ft_Gpu_Hal_Rd16(phost, REG_TOUCH_RZ));
    printf("\\n reg_touch_rzthresh =0x%x ", Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_RZTHRESH));
  printf("\\n reg_touch_tag_xy=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG_XY));
    printf("\\n reg_touch_tag=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG));
#endif
#ifdef FT900_PLATFORM
      SDHOST_STATUS sd_status;
      //
    if(sdhost_card_detect() == SDHOST_CARD_INSERTED)
    { printf("-------------1---------------");
    printf("\\n sd_status=%s",sdhost_card_detect());
    }

        if (sdhost_card_detect() != SDHOST_CARD_INSERTED)
        {
        printf("Please Insert SD Card\\r\\n");
        }
        else
        { printf("\\n SD Card inserted!");}


        if (f_mount(&FatFs, "", 0) != FR_OK)
        {
            printf("\\n Mounted failed.");;
        }

            printf("\\n Mounted succesfully.");

#endif

    /*It is optional to clear the screen here*/
    Ft_Gpu_Hal_WrMem(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
    Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);

    Ft_Gpu_Hal_Sleep(1000);//Show the booting up screen.
"""

FT8xxHelperAPI = """

#ifdef FT900_PLATFORM
    ft_void_t FT900_Config()
{
        sys_enable(sys_device_uart0);
        gpio_function(48, pad_uart0_txd); /* UART0 TXD */
        gpio_function(49, pad_uart0_rxd); /* UART0 RXD */
        uart_open(UART0,                    /* Device */
                  1,                        /* Prescaler = 1 */
                  UART_DIVIDER_115200_BAUD,  /* Divider = 1302 */
                  uart_data_bits_8,         /* No. Data Bits */
                  uart_parity_none,         /* Parity */
                  uart_stop_bits_1);        /* No. Stop Bits */

        /* Print out a welcome message... */
        uart_puts(UART0,

            "(C) Copyright 2014-2015, Future Technology Devices International Ltd. \\r\\n"
            "--------------------------------------------------------------------- \\r\\n"
            );

#ifdef ENABLE_ILI9488_HVGA_PORTRAIT
    /* asign all the respective pins to gpio and set them to default values */
    gpio_function(34, pad_gpio34);
    gpio_dir(34, pad_dir_output);
    gpio_write(34,1);

    gpio_function(27, pad_gpio27);
    gpio_dir(27, pad_dir_output);
    gpio_write(27,1);

    gpio_function(29, pad_gpio29);
    gpio_dir(29, pad_dir_output);
    gpio_write(29,1);

    gpio_function(33, pad_gpio33);
    gpio_dir(33, pad_dir_output);
    gpio_write(33,1);


    gpio_function(30, pad_gpio30);
    gpio_dir(30, pad_dir_output);
    gpio_write(30,1);

    gpio_function(28, pad_gpio28);
    gpio_dir(28, pad_dir_output);
    gpio_write(28,1);


    gpio_function(43, pad_gpio43);
    gpio_dir(43, pad_dir_output);
    gpio_write(43,1);
    gpio_write(34,1);
    gpio_write(28,1);
    gpio_write(43,1);
    gpio_write(33,1);
    gpio_write(33,1);

#endif
}
#endif


"""





loadDataToCoprocessorMediafifo = '''
ft_void_t loadDataToCoprocessorMediafifo(ft_char8_t* fileName, ft_uint32_t mediaFIFOAddr, ft_uint32_t mediaFIOFLen){
    ft_uint8_t g_scratch[SCRATCH_BUFF_SZ];
    Ft_Fifo_t stFifo;
    ft_uint32_t i;
    ft_uint32_t filesz, currchunk, bytesread, cmdrd, cmdwr;
    #if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
    FILE *pFile;
    pFile = fopen(fileName,"rb");
    if (pFile == NULL)
    #elif defined(FT900_PLATFORM)
    FIL CurFile;
    fResult = f_open(&CurFile, fileName, FA_READ | FA_OPEN_EXISTING);
    if (fResult != FR_OK)
    #endif
    {
        printf("Unable to open file.\\n");
    } else {

        Ft_Fifo_Init(&stFifo, mediaFIFOAddr, mediaFIOFLen, REG_MEDIAFIFO_READ,REG_MEDIAFIFO_WRITE); //initialize application media fifo structure
        #if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        fseek(pFile,0,SEEK_END);
        filesz = ftell(pFile);
        fseek(pFile,0,SEEK_SET);
        #elif defined(FT900_PLATFORM)
        fResult = f_lseek(&CurFile, 0);
        filesz = f_size(&CurFile);
        #endif

        /* fill the complete fifo buffer before entering into steady state */
        #if defined(FT900_PLATFORM)
        fResult = f_lseek(&CurFile, 0);
        #endif

        filesz -= stFifo.fifo_wp;
        cmdrd = Ft_Gpu_Hal_Rd16(phost, REG_CMD_READ);
        cmdwr = Ft_Gpu_Hal_Rd16(phost, REG_CMD_WRITE);
        while ((cmdrd != cmdwr) || (filesz > 0))  //loop till end of the file
        {
            if (filesz > 0) {
                if (filesz > SCRATCH_BUFF_SZ) {
                    currchunk = SCRATCH_BUFF_SZ;
                } else {
                    currchunk = filesz;
                }
                #if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
                bytesread = fread(g_scratch, 1, currchunk, pFile);
                #elif defined(FT900_PLATFORM)
                fResult = f_read(&CurFile, g_scratch, currchunk, &bytesread);
                #endif
                Ft_Fifo_WriteWait(phost, &stFifo, g_scratch, bytesread); //download the whole chunk into ring buffer - blocking call

                filesz -= currchunk;
            }

            cmdrd = Ft_Gpu_Hal_Rd16(phost, REG_CMD_READ);
            cmdwr = Ft_Gpu_Hal_Rd16(phost, REG_CMD_WRITE);
        }
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        fclose(pFile);
#elif defined(FT900_PLATFORM)
        f_close(&CurFile);
#endif
    }
}
'''

loadDataToCoprocessorCMDfifo_nowait = """
ft_void_t loadDataToCoprocessorCMDfifo_nowait(ft_char8_t* fileName){
        ft_uint8_t g_scratch[SCRATCH_BUFF_SZ];
        ft_uint32_t filesz, currchunk, bytesread, cmdrd, cmdwr;
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        FILE *pFile;
#elif defined(FT900_PLATFORM)
        FIL CurFile;
#endif

#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        pFile = fopen(fileName, "rb");
        if (pFile != NULL)
        {
            printf("Fopen success!\\n");
        }
        else{
            printf("Failed to open file.\\n");
            return;
        }
#elif defined(FT900_PLATFORM)
        fResult = f_open(&CurFile, fileName, FA_READ | FA_OPEN_EXISTING);
        if (fResult == FR_OK)
        {
            printf("Fopen success!\\n");
            fResult = f_lseek(&CurFile, 0);
        }
        else
        {
            printf("Failed to open file %d\\n", fResult);
            return;
        }
#endif


#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        fseek(pFile, 0, SEEK_END);
        filesz = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
#elif defined(FT900_PLATFORM)
        filesz = f_size(&CurFile);
#endif
        {
            ft_int32_t availfreesz = 0, freadbufffill = 0, chunkfilled = 0;
            ft_uint8_t *pbuff = g_scratch;
            while (filesz > 0)
            {
                availfreesz = Ft_Gpu_Hal_Rd32(phost, REG_CMDB_SPACE);
                if (availfreesz > 0)
                {
                    if (0 == freadbufffill)
                    {
                        if (filesz > SCRATCH_BUFF_SZ)
                        {
                            freadbufffill = SCRATCH_BUFF_SZ;
                        }
                        else
                        {
                            freadbufffill = filesz;
                        }
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
                        bytesread = fread(g_scratch, 1, freadbufffill, pFile);
#elif defined(FT900_PLATFORM)
                        fResult = f_read(&CurFile, g_scratch, freadbufffill, &bytesread);
#endif
                        pbuff = g_scratch;
                        filesz -= bytesread;
                    }

                    if (availfreesz > freadbufffill)
                    {
                        availfreesz = freadbufffill;
                    }

                    if (availfreesz > 0)
                    {


#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
                        Ft_Gpu_Hal_WrMem(phost, REG_CMDB_WRITE, pbuff, availfreesz);
#elif defined(FT900_PLATFORM)
                        Ft_Gpu_Hal_StartTransfer(phost, FT_GPU_WRITE, REG_CMDB_WRITE);
                        spi_writen(SPIM, pbuff, availfreesz);
                        Ft_Gpu_Hal_EndTransfer(phost);
#endif

                    }
                    pbuff += availfreesz;
                    freadbufffill -= availfreesz;
                }
            }
        }

#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
    fclose(pFile);
#elif defined(FT900_PLATFORM)
    f_close(&CurFile);
#endif
}

"""


loadDataToCoprocessorCMDfifo = '''
ft_void_t loadDataToCoprocessorCMDfifo(ft_char8_t* fileName){
#if defined(FT900_PLATFORM)
    ft_uint32_t fResult, fileLen;
    ft_uint32_t bytesread;
    FIL CurFile;
    ft_uint8_t pBuff[SCRATCH_BUFF_SZ];
    fResult = f_open(&CurFile, fileName, FA_READ);

    if(fResult == FR_OK){
    fileLen = f_size(&CurFile);
    while(fileLen > 0){
        ft_uint32_t blocklen = fileLen>SCRATCH_BUFF_SZ?SCRATCH_BUFF_SZ:fileLen;
        fResult = f_read(&CurFile,pBuff,blocklen,&bytesread);
        fileLen -= bytesread;
        Ft_Gpu_Hal_WrCmdBuf(phost,pBuff, bytesread);//alignment is already taken care by this api
    }
    f_close(&CurFile);
    }
    else{
        printf("Unable to open file\\n");
    }


#else
    FILE *fp;
    ft_uint32_t fileLen;
    ft_uint8_t pBuff[SCRATCH_BUFF_SZ];
    fp = fopen(fileName, "rb+");

    if(fp){

        fseek(fp,0,SEEK_END);
        fileLen = ftell(fp);
        fseek(fp,0,SEEK_SET);
        while(fileLen > 0)
        {
            ft_uint32_t blocklen = fileLen>SCRATCH_BUFF_SZ?SCRATCH_BUFF_SZ:fileLen;
            fread(pBuff,1,blocklen,fp);
            fileLen -= blocklen;
            Ft_Gpu_Hal_WrCmdBuf(phost,pBuff, blocklen);//alignment is already taken care by this api
        }
        fclose(fp);
    }else{
        printf("Unable to open file: %s\\n",fileName);
        //exit(1);
    }
#endif
}
'''

endOfDisplayListSequence = """
    Ft_App_WrCoCmd_Buffer(phost, DISPLAY());
    Ft_Gpu_CoCmd_Swap(phost);
    Ft_App_Flush_Co_Buffer(phost);
    Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
"""

screenCalibrationSequence = """
        Ft_Gpu_CoCmd_Dlstart(phost);
        Ft_App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));
        Ft_App_WrCoCmd_Buffer(phost, COLOR_RGB(255, 255, 255));
        Ft_Gpu_CoCmd_Text(phost,(FT_DispWidth/2), (FT_DispHeight/2), 27, OPT_CENTER, "Please Tap on the dot");
#if defined(FT_801_ENABLE) || defined(FT_811_ENABLE) || defined(FT_813_ENABLE)
        Ft_Gpu_Hal_Wr8(phost, REG_CTOUCH_EXTENDED, CTOUCH_MODE_COMPATIBILITY);
#endif
        Ft_Gpu_CoCmd_Calibrate(phost, 0);
        /* Download the commands into FIFIO */
        Ft_App_Flush_Co_Buffer(phost);
        /* Wait till coprocessor completes the operation */
        Ft_Gpu_Hal_WaitCmdfifo_empty(phost);
"""

FT8xxMainFunctionEnd = """

    /* Close all the opened handles */
    Ft_Gpu_Hal_Close(phost);
    Ft_Gpu_Hal_DeInit();
#ifdef MSVC_PLATFORM || defined FT900_PLATFORM
    return 0;
#endif
}

void loop()
{
}



/* Nothing beyond this */
"""

startingOfDisplayListSequence = """
    Ft_Gpu_CoCmd_Dlstart(phost);
    Ft_App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));
"""

globalContext = {
    'mediaFIFOEnabled': "False",
    'mediaFIFOAddress':  "",
    'mediaFIFOLength': "",
}


globalValue = {
    'skeletonProjectName': "untitled",
    'assetsFolder': "Test",
}

def raiseUnicodeError(errorArea):
    raise Exception("Unable to export project: unicode characters are currently unsupported.  Please check: " + errorArea)

def run(name, document, ram, moduleName):
    global lutSize
    deviceType = document["project"]["device"]
    resultText = "<b>EVE HAL Export</b><br>"
    HALProjectName = "_FTEVE_HAL"
    for line in document["displayList"]:
        if not line == "":
            resultText += "<b>Warning</b>: Only support for Coprocessor commands, ignoring Display List.<br>"
            break

    if deviceType == 2064 or deviceType == 2065 or deviceType == 2066 or deviceType == 2067:
        outDir = "FT81X_" + name + "_FTEVE_HAL"
        functionMap["CMD_SNAPSHOT2"] = "Ft_Gpu_CoCmd_Snapshot2"
        functionMap["CMD_SETBASE"] = "Ft_Gpu_CoCmd_SetBase"
        functionMap["CMD_MEDIAFIFO"] = "Ft_Gpu_CoCmd_MediaFifo"
        functionMap["CMD_PLAYVIDEO"] = "Ft_Gpu_CoCmd_PlayVideo"
        functionMap["CMD_SETFONT2"] = "Ft_Gpu_CoCmd_SetFont2"
        functionMap["CMD_SETSCRATCH"] = "Ft_Gpu_CoCmd_SetScratch"
        functionMap["CMD_ROMFONT"] = "Ft_Gpu_CoCmd_RomFont"
        functionMap["CMD_SETBITMAP"] = "Ft_Gpu_CoCmd_SetBitmap"
    else:
        outDir = "FT80X_" + name + "_FTEVE_HAL" 

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

    outName = outDir + os.path.sep + name + HALProjectName + ".c"
    
    f = open(outName, "w+")
    f.write(FT8xxInitialConfig)
    for content in document["content"]:
        try:
            if content["memoryLoaded"]:
                if content["imageFormat"] in palettedFormats:
                    ramOffset = int(content["memoryAddress"])
                    lutAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper() + "_LUT"
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()		
                    f.write("\n")		
		
                    if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:		
                        f.write("#define " + lutAddress + " RAM_PAL" + "\n")		
                        f.write("#define " + memoryAddress + " " + str(ramOffset) + "\n")		
                    else:
                        if ((content["imageFormat"] == paletted565_format) or 
                            (content["imageFormat"] == paletted4444_format)):
                            lutSize = 256*2
                            
                        f.write("#define " + lutAddress + " " + str(ramOffset) + "\n")		
                        f.write("#define " + memoryAddress + " " + str(ramOffset + lutSize) + "\n")		
                else:		
                    memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()		
                    f.write("\n")		
                    f.write("#define " + memoryAddress + " " + str(content["memoryAddress"]) + "\n")
            if content["dataStorage"] == "Embedded":
                data = ""
                contentName = re.sub(r'[-/. ]', '_', content["destName"])
                headerName = content["destName"].replace('/', os.path.sep)

                lutContentName = contentName + "_lut"		
                lutHeaderName = headerName + ".lut"                
                
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
                content["FTEVE_Name"] = contentName
                f.write("\n")
                charType = "ft_uchar8_t"
                #charType = "char" # For older Linux distro
             
	
                if content["imageFormat"] in palettedFormats:		
                    with open (lutTargetPath, "r") as resourceFile:		
                        data = resourceFile.read().replace('\n','')		
                    f.write("static " + charType + " " + lutContentName + "[] = {\n")		
                    #f.write("\t#include \"" + targetName + "\"\n")		
                    f.write("\t" + data + "\n")		
                    f.write("};\n")                
                
                with open (targetPath, "r") as resourceFile:
                    data = resourceFile.read().replace('\n','')
                f.write("static " + charType + " " + contentName + "[] = {\n")
                #f.write("\t#include \"" + targetName + "\"\n")
                f.write("\t" + data + "\n")
                f.write("};\n")
        except (UnicodeDecodeError, UnicodeEncodeError):
            f.close()
            raiseUnicodeError("Name of the assets")
    f.write("\n")
    f.write(FT8xxHelperAPI)
    f.write("\n")

    if deviceType == 2064 or deviceType == 2065 or deviceType == 2066 or deviceType == 2067:
        f.write(loadDataToCoprocessorMediafifo)
        f.write("\n")
        f.write(loadDataToCoprocessorCMDfifo_nowait)
        f.write("\n")

    f.write(loadDataToCoprocessorCMDfifo)
    f.write("\n")
    f.write(FT8xxMainFunctionSetup)
    f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")

    for content in document["content"]:
        if content["memoryLoaded"]:
            memoryAddress = "RAM_" + re.sub(r'[/. ]', '_', content["destName"]).upper()
            lutMemoryAddress = memoryAddress + "_LUT"
            if content["dataStorage"] == "Embedded":
                contentName = content["FTEVE_Name"]
                lutContentName = contentName + "_lut"
                if content["converter"] == "RawJpeg":
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost,CMD_LOADIMAGE);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost," + memoryAddress + ");\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost,0);\n")
                    f.write("\tFt_Gpu_Hal_WrCmdBuf(phost," + contentName + ", sizeof(" + contentName + "));\n")
                if content["converter"] == "Raw":
                    f.write("\tFt_Gpu_Hal_WrMem(phost," + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                    if content["imageFormat"] in palettedFormats:
                        if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:		
                            f.write("\tFt_Gpu_Hal_WrMem(phost, RAM_PAL , " + lutContentName + ", sizeof(" + lutContentName + "));\n")		
                        else:		
                            f.write("\tFt_Gpu_Hal_WrMem(phost," + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n")                    
                else:
                    if content["dataCompressed"]:
                        f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_INFLATE);\n")                        
                        f.write("\tFt_Gpu_Hal_WrCmd32(phost," + memoryAddress + ");\n")
                        f.write("\tFt_Gpu_Hal_WrCmdBuf(phost," + contentName + ", sizeof(" + contentName + "));\n")
                        if content["imageFormat"] in palettedFormats:		
                            if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:		
                                f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_INFLATE);\n")		
                                f.write("\tFt_Gpu_Hal_WrCmd32(phost, RAM_PAL );\n")		
                                f.write("\tFt_Gpu_Hal_WrCmdBuf(phost," + lutContentName + ", sizeof(" + lutContentName + "));\n")		
                            else:		
                                f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_INFLATE);\n")		
                                f.write("\tFt_Gpu_Hal_WrCmd32(phost," + lutMemoryAddress + " );\n")		
                                f.write("\tFt_Gpu_Hal_WrCmdBuf(phost," + lutContentName + ", sizeof(" + lutContentName + "));\n")                        
                    else:
                        f.write("\tFt_Gpu_Hal_WrMem(phost," + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                        if content["imageFormat"] in palettedFormats:		
                            if document["project"]["device"] == 2048 or document["project"]["device"] == 2049:		
                                f.write("\tFt_Gpu_Hal_WrMem(phost, RAM_PAL , " + lutContentName + ", sizeof(" + lutContentName + "));\n")		
                            else:		
                                f.write("\tFt_Gpu_Hal_WrMem(phost," + lutMemoryAddress + ", " + lutContentName + ", sizeof(" + lutContentName + "));\n")                        
    for line in document["coprocessor"]:
        if line:
            try:
                functionName, functionArgs, comment = parseCommand(line, functionMap, convertArgs)
                if functionName in ["BITMAP_HANDLE", "BITMAP_SOURCE", "BITMAP_LAYOUT", "BITMAP_SIZE", "CMD_SETFONT"]:
                    newline = "\tFt_App_WrCoCmd_Buffer(phost," + functionName + "(" + functionArgs + "));" + comment + "\n"
                    f.write(newline)
                else:
                    break
            except (UnicodeDecodeError, UnicodeEncodeError):
                f.close()
                raiseUnicodeError("Unicodes in Coprocessor editing box.")

    clearFound = False
    displayScreenCalibration = False
    for line in document["coprocessor"]:
        if line:
            try:
                functionName, _, _ = parseCommand(line, functionMap, convertArgs)
                if functionName == "CLEAR":
                    clearFound = True
                if functionname == "Ft_Gpu_CoCmd_Calibrate":
                    displayScreenCalibration = True
            except:
                pass

    if displayScreenCalibration:
        f.write(screenCalibrationSequence)

    if not clearFound:
        f.write("\tFt_App_WrCoCmd_Buffer(phost,CLEAR(1, 1, 1));\n")

    skippedBitmaps = False
    specialParameter = ""
    specialParameter2 = ""
    specialCommandType = ""
    filesToTestFolder = []

    for line in document["coprocessor"]:
        if line:
            try:
                #if the line is a comment line then just write it out
                if (line.lstrip()).startswith("//"):
                    f.write("\t" + line + "\n")
                    continue
                functionName, functionArgs, comment = parseCommand(line, functionMap, convertArgs)
                coprocessor_cmd = False                
                if functionName in functionMap.values():
                    coprocessor_cmd = True
                if not skippedBitmaps:
                    if functionName in ["BITMAP_HANDLE", "BITMAP_SOURCE", "BITMAP_LAYOUT", "BITMAP_SIZE", "CMD_SETFONT"]:
                        continue
                    else:
                        skippedBitmaps = True

                if functionName == "Ft_Gpu_CoCmd_Snapshot2":

                    f.write("\tFt_App_WrCoCmd_Buffer(phost, DISPLAY());\n")
                    f.write("\tFt_Gpu_CoCmd_Swap(phost);\n")

                    f.write("\t/* Download the commands into fifo */\n")
                    f.write("\tFt_App_Flush_Co_Buffer(phost);\n")

                    f.write("\t/* Wait till coprocessor completes the operation */\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")

                    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n")

                    f.write("\tFt_Gpu_CoCmd_Snapshot2(phost," + functionArgs + ");\n")

                    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n")
                    f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")
                    f.write("\tFt_App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));\n")
                    f.write("\tFt_App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n")
                    f.write("\tFt_App_WrCoCmd_Buffer(phost, COLOR_RGB(0xff, 0xff, 0xff));\n")

                if functionName == "Ft_Gpu_CoCmd_Snapshot":

                    f.write("\tFt_App_WrCoCmd_Buffer(phost, DISPLAY());\n")
                    f.write("\tFt_Gpu_CoCmd_Swap(phost);\n")

                    f.write("\t/* Download the commands into fifo */\n")
                    f.write("\tFt_App_Flush_Co_Buffer(phost);\n")

                    f.write("\t/* Wait till coprocessor completes the operation */\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")

                    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n")

                    f.write("\tFt_Gpu_Hal_Wr16(phost, REG_HSIZE, " + str(document["registers"]["hSize"]) + ");\n")
                    f.write("\tFt_Gpu_Hal_Wr16(phost, REG_VSIZE, " + str(document["registers"]["vSize"]) + ");\n")
                    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n")

                    f.write("\t/* Take snap shot of the current screen */\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_SNAPSHOT);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost," + functionArgs + ");\n")

                    f.write("\t//timeout for snapshot to be performed by coprocessor\n")

                    f.write("\t/* Wait till coprocessor completes the operation */\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")

                    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n")

                    f.write("\t/* reconfigure the resolution wrt configuration */\n")
                    if moduleName == "VM800B43_50" or moduleName == "VM800BU43_50" or moduleName == "VM800C43_50" or moduleName == "VM801B43_50":
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_HSIZE,480);\n")
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_VSIZE,272);\n")
                    elif moduleName == "VM800B35" or moduleName == "VM800BU35" or moduleName == "VM800C35":
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_HSIZE,320);\n")
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_VSIZE,240);\n")
                    elif (moduleName == "VM810C50" or moduleName == "ME810AU_WH70R" or moduleName == "ME811AU_WH70C" or moduleName == "ME812AU_WH50R" or moduleName == "ME813AU_WH50C"
                          or moduleName == "ME810A_WH70R" or moduleName == "ME811A_WH70C" or moduleName == "ME812A_WH50R" or moduleName == "ME813A_WH50C"):
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_HSIZE,800);\n")
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_VSIZE,480);\n")
                    elif moduleName == "ME810A_HV35R":
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_HSIZE,320);\n")
                        f.write("\tFt_Gpu_Hal_Wr16(phost, REG_VSIZE,480);\n")

                    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n")
                    f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")
                    f.write("\tFt_App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));\n")
                    f.write("\tFt_App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n")
                    f.write("\tFt_App_WrCoCmd_Buffer(phost, COLOR_RGB(0xff, 0xff, 0xff));\n")

                    functionName = ""

                if functionName == "Ft_Gpu_CoCmd_LoadImage":
                    functionArgsSplit = functionArgs.split(',')
                    f.write("\tFt_App_Flush_Co_Buffer(phost);\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_LOADIMAGE);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, " + functionArgsSplit[0] + ");\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, " + functionArgsSplit[1] + ");\n")
                    functionArgs = functionArgsSplit[0] + ","
                    functionArgs += functionArgsSplit[1]
                    
                    if "OPT_MEDIAFIFO" in functionArgsSplit[1]:
                        globalContext['mediaFIFOEnabled'] = 'True'
                    functionArgsSplit[2] = re.sub(r'["]', "", functionArgsSplit[2])

                    if '/' in functionArgsSplit[2]:
                        specialParameter = functionArgsSplit[2].rsplit('/',1)[1]
                    elif '\\' in functionArgsSplit[2]:
                        specialParameter = functionArgsSplit[2].rsplit('\\',1)[1]
                    else:
                        specialParameter = functionArgsSplit[2]
                    specialParameter2 = specialParameter
                    filesToTestFolder.append(functionArgsSplit[2])
                    specialParameter = "..\\\\..\\\\..\\\\Test\\\\" + specialParameter
                    specialCommandType = "Ft_Gpu_CoCmd_LoadImage"
                    functionName = ""

                if functionName == "Ft_Gpu_CoCmd_MediaFifo":
                    functionArgsSplit = functionArgs.split(',')
                    functionArgs = functionArgsSplit[0] + ","
                    functionArgs += functionArgsSplit[1]
                    specialCommandType = "Ft_Gpu_CoCmd_MediaFifo"
                    globalContext['mediaFIFOAddress'] = functionArgsSplit[0]
                    globalContext['mediaFIFOLength'] = functionArgsSplit[1]

                if functionName == "Ft_Gpu_CoCmd_Dlstart":
                    f.write(startingOfDisplayListSequence)
                    functionName = ""

                if functionName == "Ft_Gpu_CoCmd_PlayVideo":
                    f.write("\n")
                    functionArgsSplit =functionArgs.split(',')
                    functionArgs = functionArgsSplit[0]
                    if 'OPT_MEDIAFIFO' in functionArgsSplit[0]:
                        globalContext['mediaFIFOEnabled'] = 'True'
                    functionArgsSplit[1] = re.sub(r'["]', "", functionArgsSplit[1])
                    if '/' in functionArgsSplit[1]:
                        specialParameter = functionArgsSplit[1].rsplit('/',1)[1]
                    elif '\\' in functionArgsSplit[1]:
                        specialParameter = functionArgsSplit[1].rsplit('\\',1)[1]
                    else:
                        specialParameter = functionArgsSplit[1]
                    filesToTestFolder.append(functionArgsSplit[1])
                    f.write("\tFt_App_Flush_Co_Buffer(phost);\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_PLAYVIDEO);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, " + functionArgsSplit[0] + ");\n")
                    specialParameter2 = specialParameter
                    specialParameter = "..\\\\..\\\\..\\\\Test\\\\" + specialParameter
                    specialCommandType = "Ft_Gpu_CoCmd_PlayVideo"
                    functionName = ""

                #The following commands don't take any parameters so there shouldn't be a comma after the phost
                parameterComma = ","
                if functionName == "Ft_Gpu_CoCmd_LoadIdentity" or functionName == "Ft_Gpu_CoCmd_Swap" or functionName == "Ft_Gpu_CoCmd_Stop" or functionName == "Ft_Gpu_CoCmd_SetMatrix" or functionName == "Ft_Gpu_CoCmd_ColdStart" or functionName == "Ft_Gpu_CoCmd_Dlstart" or functionName == "Ft_Gpu_CoCmd_ScreenSaver":
                    parameterComma = ""
                #attempt to append comments.
                if functionName:
                    if coprocessor_cmd:
                        newline = "\t" + functionName + "(phost" + parameterComma + functionArgs + ");" + comment + "\n"
                    else:
                        newline = "\tFt_App_WrCoCmd_Buffer(phost" + parameterComma + functionName + "(" + functionArgs + "));" + comment + "\n"
                    f.write(newline)
                
                if specialCommandType == "Ft_Gpu_CoCmd_LoadImage":
                    if globalContext['mediaFIFOEnabled'] == "True":
                        f.write("\t#if defined(FT900_PLATFORM)\n")
                        f.write("\tloadDataToCoprocessorMediafifo(\"" + specialParameter2 + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n")
                        f.write("\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n")
                        f.write("\tloadDataToCoprocessorMediafifo(\"" + specialParameter + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n")
                        f.write("\t#endif\n")
                        f.write("\tFt_App_Flush_Co_Buffer(phost);\n")
                        globalContext['mediaFIFOEnabled'] = "False"
                    else:
                        f.write("\t#if defined(FT900_PLATFORM)\n")
                        f.write("\tloadDataToCoprocessorMediafifo(\"" + specialParameter2 + "\");\n")
                        f.write("\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n")
                        f.write("\tloadDataToCoprocessorCMDfifo(\"" + specialParameter + "\");\n")
                        f.write("\t#endif\n")
                        f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")
                    specialCommandType = ""
                elif specialCommandType == "Ft_Gpu_CoCmd_PlayVideo":
                    if globalContext['mediaFIFOEnabled'] == 'True':
                        f.write("\t#if defined(FT900_PLATFORM)\n")
                        f.write("\tloadDataToCoprocessorMediafifo(\"" + specialParameter2 + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n")
                        f.write("\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n")
                        f.write("\tloadDataToCoprocessorMediafifo(\"" + specialParameter + "\" , " + globalContext['mediaFIFOAddress'] + " , " + globalContext['mediaFIFOLength'] + ");\n")
                        f.write("\t#endif\n")
                        globalContext['mediaFIFOEnabled'] = "False"
                    else:
                        f.write("\t#if defined(FT900_PLATFORM)\n")
                        f.write("\tloadDataToCoprocessorCMDfifo_nowait(\"" + specialParameter2 + "\");\n")
                        f.write("\t#elif defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)\n")
                        f.write("\tloadDataToCoprocessorCMDfifo_nowait(\"" + specialParameter + "\");\n")
                        f.write("\t#endif\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")
                    specialCommandType = ""
                else:
                    specialCommandType = ""

            except Exception as e:
                pass
        else:
            if skippedBitmaps:
                f.write("\t\n")

    f.write(endOfDisplayListSequence)
    f.write("\n")
    f.write(FT8xxMainFunctionEnd)
    f.close()

    generateProjectFiles(outDir, outName, name, filesToTestFolder, moduleName)

    resultText += "<b>Output</b>:<p>Output files: " + outDir + "</p> <p>Project files: " + outDir + os.path.sep + name + "</p><p>\"" + name + HALProjectName + os.path.sep + "ReadMe.txt\" details the project folder structure.</p>"

    if sys.platform.startswith('darwin'):
        subprocess.call(('open', outName))
    elif os.name == 'nt':
        subprocess.call(['explorer', outDir])
    elif os.name == 'posix':
        subprocess.call(('xdg-open', outName))
    return resultText

