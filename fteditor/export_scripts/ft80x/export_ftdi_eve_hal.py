import os, sys, re, shutil, subprocess, errno, stat


def displayName():
    return "EVE HAL 2.0 Project"


def renameStringInFile(file, oldName, newName):
    with open(file, 'r') as resourceFile:
        data = resourceFile.read().decode("utf-8-sig").encode("utf-8")

    data = data.replace(oldName, newName)
    #source = unicode(data, 'utf-8')

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
    except IOError:
        raise IOError("Unable to generate a complete MSVC project. Try again and make sure the folders and files of the project directory are accessible..")
    except ValueError:
        raise ValueError("The path of the files in the generated project have exceeded the maximum allowed length for the current platform.  Rename the current project and/or move the project closer to your home directory.")
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


def generateProjectFiles(destDir, srcFile, projectName, filesToTestFolder, screenSize, deviceType):
    skeletonProjectDir = destDir + os.path.sep + projectName
    MSVCSolutionName = skeletonProjectDir + os.path.sep + "Project" + os.path.sep + "Msvc_win32" + os.path.sep + projectName + os.path.sep + projectName
    EmulatorSolutionName = skeletonProjectDir + os.path.sep + "Project" + os.path.sep + "Msvc_Emulator" + os.path.sep + projectName + os.path.sep + projectName
    MSVCPlatformHeader = skeletonProjectDir + os.path.sep + "Hdr" + os.path.sep + "Msvc_Emulator" + os.path.sep + "FT_Platform.h"
    EmulatorPlatformHeader = skeletonProjectDir + os.path.sep + "Hdr" + os.path.sep + "Msvc_win32" + os.path.sep + "FT_Platform.h"
    MSVCSolution = MSVCSolutionName + ".sln"
    MSVCProject = MSVCSolutionName + ".vcxproj"
    MSVCFilter = MSVCSolutionName + ".vcxproj.filters"

    EmulatorSolution = EmulatorSolutionName + ".sln"
    EmulatorProject = EmulatorSolutionName + ".vcxproj"
    EmulatorFilter = EmulatorSolutionName + ".vcxproj.filters"

    #if os.path.isdir(os.getcwd() + os.path.sep + globalValue['skeletonProjectName']):
    scriptDir = os.path.abspath(__file__).split("export_scripts")
    defaultSkeletonProjectDir = scriptDir[0] + globalValue['skeletonProjectName']
    if os.path.isdir(defaultSkeletonProjectDir):
        #copydir2(os.path.dirname(os.path.realpath(__file__)) + "/Examples/untitled", skeletonProjectDir)  #
        copydir2(defaultSkeletonProjectDir, skeletonProjectDir)
    else:
        raise Exception("Required program files are missing.")

    #try:
    #	#distutils.dir_util.copy_tree(os.getcwd() + "/Examples/untitled", skeletonProjectDir)
    #	#copydir(os.getcwd() + "/Examples/untitled", skeletonProjectDir)
    #	copydir2(os.getcwd() + "/Examples/untitled", skeletonProjectDir)
    ##except Exception as exc:
    #except:
    #	return

    #try:
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
    #rename MSVC project files
    renameStringInFile(MSVCSolution, globalValue['skeletonProjectName'], projectName)
    renameStringInFile(MSVCProject, globalValue['skeletonProjectName'], projectName)
    renameStringInFile(MSVCFilter, globalValue['skeletonProjectName'], projectName)
    #rename emulator project files
    renameStringInFile(EmulatorSolution, globalValue['skeletonProjectName'], projectName)
    renameStringInFile(EmulatorProject, globalValue['skeletonProjectName'], projectName)
    renameStringInFile(EmulatorFilter, globalValue['skeletonProjectName'], projectName)
    #rename readme.txt
    renameStringInFile(skeletonProjectDir + os.path.sep + "ReadMe.txt", globalValue['skeletonProjectName'], projectName)

    #except Exception as e:
    #    raise Exception("Error while renaming project files: " + str(e))

    try:
        if(screenSize == "320x240"):
            renameStringInFile(MSVCPlatformHeader, "//#define DISPLAY_RESOLUTION_QVGA", "#define DISPLAY_RESOLUTION_QVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_QVGA", "#define DISPLAY_RESOLUTION_QVGA")
        else:
            renameStringInFile(MSVCPlatformHeader, "//#define DISPLAY_RESOLUTION_WQVGA", "#define DISPLAY_RESOLUTION_WQVGA")
            renameStringInFile(EmulatorPlatformHeader, "//#define DISPLAY_RESOLUTION_WQVGA", "#define DISPLAY_RESOLUTION_WQVGA")

        #change device type
        if deviceType == 2048:
            renameStringInFile(MSVCPlatformHeader, "//#define FT_800_ENABLE", "#define FT_800_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_800_ENABLE", "#define FT_800_ENABLE")
        elif deviceType == 2049:
            renameStringInFile(MSVCPlatformHeader, "//#define FT_801_ENABLE", "#define FT_801_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_801_ENABLE", "#define FT_801_ENABLE")
        elif deviceType == 2064:
            renameStringInFile(MSVCPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_810_ENABLE", "#define FT_810_ENABLE")
        elif deviceType == 2065:
            renameStringInFile(MSVCPlatformHeader, "//#define FT_811_ENABLE", "#define FT_811_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_811_ENABLE", "#define FT_811_ENABLE")
        else:
            renameStringInFile(MSVCPlatformHeader, "//#define FT_800_ENABLE", "#define FT_800_ENABLE")
            renameStringInFile(EmulatorPlatformHeader, "//#define FT_800_ENABLE", "#define FT_800_ENABLE")

    except Exception as e:
        raise Exception("Error while renaming configure project files: " + str(e))


    for content in filesToTestFolder:
        destinationName = ""
        content = content.strip()
        #raise Exception(content)
        if '/' in content:
            destinationName = content.rsplit('/', 1)[1]
        elif '\\' in content:
            destinationName = content.rsplit('\\', 1)[1]
        #raise Exception(content)
        try:
            shutil.copy(content, destDir + os.path.sep + projectName + os.path.sep + globalValue['assetsFolder'] + os.path.sep + destinationName)
        except Exception as e:
            raise Exception("Error copying assets to project folder: " + e)

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
    /* useful for timer */
    ft_millis_init();
    interrupt_enable_globally();
    //printf("ft900 config done \\n");
}
#endif


"""


loadDataToCoprocessorCMDfifo = '''
ft_void_t loadDataToCoprocessorCMDfifo(ft_char8_t* fileName){
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
}
'''


FT8xxMainFunctionSetup = """

#if defined MSVC_PLATFORM || defined FT900_PLATFORM
/* Main entry point */
ft_int32_t main(ft_int32_t argc,ft_char8_t *argv[])
#endif
#if defined(ARDUINO_PLATFORM)||defined(MSVC_FT800EMU)
ft_void_t setup()
#endif
{

     ft_uint8_t chipid;
#ifdef FT900_PLATFORM
    FT900_Config();
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

    //printf("Ft_Gpu_Hal_Open done \\n");
    phost = &host;

    Ft_BootupConfig();

#ifdef FT900_PLATFORM
    Ft_Gpu_ClockTrimming(phost,LOW_FREQ_BOUND);
    /* Change clock freuency to 25mhz */
    spi_init(SPIM, spi_dir_master, spi_mode_0, 4);
    spi_option(SPIM,spi_option_bus_width,1);
    spi_option(SPIM,spi_option_fifo_size,64);
    spi_option(SPIM,spi_option_fifo,1);
    spi_option(SPIM,spi_option_fifo_receive_trigger,1);
#endif

#if ((defined FT900_PLATFORM) || defined(MSVC_PLATFORM)) ||defined(MSVC_FT800EMU)
    printf("\\n reg_touch_rz =0x%x ", Ft_Gpu_Hal_Rd16(phost, REG_TOUCH_RZ));
    printf("\\n reg_touch_rzthresh =0x%x ", Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_RZTHRESH));
    printf("\\n reg_touch_tag_xy=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG_XY));
    printf("\\n reg_touch_tag=0x%x",Ft_Gpu_Hal_Rd32(phost, REG_TOUCH_TAG));
#endif

    /*It is optional to clear the screen here*/
    Ft_Gpu_Hal_WrMem(phost, RAM_DL,(ft_uint8_t *)FT_DLCODE_BOOTUP,sizeof(FT_DLCODE_BOOTUP));
    Ft_Gpu_Hal_Wr8(phost, REG_DLSWAP,DLSWAP_FRAME);

    Ft_Gpu_Hal_Sleep(1000);//Show the booting up screen.
"""

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

globalValue = {
    'skeletonProjectName': "untitled",
    'assetsFolder': "Test",
}

def raiseUnicodeError(errorArea):
    raise Exception("Unable to export project: unicode characters are currently unsupported.  Please check: " + errorArea)


def run(name, document, ram):
    resultText = "<b>EVE HAL Export</b><br>"
    HALProjectName = "_FTEVE_HAL"
    for line in document["displayList"]:
        if not line == "":
            resultText += "<b>Warning</b>: Only support for Coprocessor commands, ignoring Display List.<br>"
            break

    try:
        name.decode('ascii')
    except UnicodeDecodeError:
        raiseUnicodeError("Project Name")

    #outDir = os.getcwd() + os.path.sep + "FT80X_" + name + "_FTEVE_HAL"
    outDir = "FT80X_" + name + "_FTEVE_HAL"  #output files are relative to the current project folder
    #outDir = outDir.encode("UTF-8")

    try:
        if os.path.isdir(outDir):
            try:
                shutil.rmtree(outDir)
            except IOError:
                raise Exception("Unable to generate a complete MSVC project. Please make sure the previously generated project files and folder are not currently being accessed.")
        os.makedirs(outDir)
    except:
        raise Exception("Unable to generate a MSVC project files.")

    outName = outDir + os.path.sep + name + HALProjectName + ".c"

    f = open(outName, "w+")
    f.write(FT8xxInitialConfig)
    for content in document["content"]:
        try:
            if content["memoryLoaded"]:
                memoryAddress = "RAM_" + re.sub(r'[-/. ]', '_', content["destName"]).upper()
                f.write("\n")
                f.write("#define " + memoryAddress + " " + str(content["memoryAddress"]) + "\n")
            if content["dataEmbedded"]:
                data = ""
                contentName = re.sub(r'[-/. ]', '_', content["destName"])
                #headerName = os.getcwd() + os.path.sep + content["destName"].replace('/', os.path.sep)
                headerName = content["destName"].replace('/', os.path.sep)

                #load raw files
                if content["converter"] == "Raw":
                    fcontent = ""
                    #with open(os.getcwd() + os.path.sep + content["destName"].replace('/',os.path.sep) + '.raw', 'rb') as rawf:
                    with open(content["destName"].replace('/',os.path.sep) + '.raw', 'rb') as rawf:
                        fcontent = rawf.read()
                    targetName = contentName + ".h"
                    targetPath = outDir + os.path.sep + targetName
                    if os.path.isfile(targetPath):
                        os.remove(targetPath)
                    foutput = open(targetPath, 'w+')
                    for i in range(0,len(fcontent)):
                        foutput.write(str(ord(fcontent[i])))
                        foutput.write(",")
                    foutput.close()
                else:
                    if content["dataCompressed"]:
                        headerName += ".binh"
                    else:
                        headerName += ".rawh"
                    targetName = contentName + ".h"
                    targetPath = outDir + os.path.sep + targetName
                    if os.path.isfile(targetPath):
                        os.remove(targetPath)
                    shutil.copy(headerName, targetPath)
                content["FTEVE_Name"] = contentName
                f.write("\n")
                charType = "ft_uchar8_t"
                #charType = "char" # For older Linux distro
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
    f.write(loadDataToCoprocessorCMDfifo)
    f.write("\n")
    f.write(FT8xxMainFunctionSetup)
    f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")

    for content in document["content"]:
        if content["memoryLoaded"]:
            memoryAddress = "RAM_" + re.sub(r'[/. ]', '_' , content["destName"]).upper()
            if content["dataEmbedded"]:
                contentName = content["FTEVE_Name"]
                if content["converter"] == "RawJpeg":
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost,CMD_LOADIMAGE);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost," + memoryAddress + ");\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost,0);\n")
                    f.write("\tFt_Gpu_Hal_WrCmdBuf(phost," + contentName + ", sizeof(" + contentName + "));\n")
                if content["converter"] == "Raw":
                    f.write("\tFt_Gpu_Hal_WrMem(phost," + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
                else:
                    if content["dataCompressed"]:
                        f.write("\tFt_Gpu_Hal_WrCmd32(phost,CMD_INFLATE);\n")
                        f.write("\tFt_Gpu_Hal_WrCmd32(phost," + memoryAddress + ");\n")
                        f.write("\tFt_Gpu_Hal_WrCmdBuf(phost," + contentName + ", sizeof(" + contentName + "));\n")
                    else:
                        f.write("\tFt_Gpu_Hal_WrMem(phost," + memoryAddress + ", " + contentName + ", sizeof(" + contentName + "));\n")
    for line in document["coprocessor"]:
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                splitlineb = splitlinea[1].split(')',1)
                functionName = splitlinea[0]
                if functionMap.has_key(functionName):
                    functionName = functionMap[functionName]
                commentsRegex = re.compile("//.*$")
                if functionName == "BITMAP_HANDLE" or functionName == "BITMAP_SOURCE" or functionName == "BITMAP_LAYOUT" or functionName == "BITMAP_SIZE" or functionName == "CMD_SETFONT":
                    functionArgs = convertArgs(splitlineb[0])
                    comment = ""
                    m = commentsRegex.match(splitlineb[1])
                    if m:
                        comment = m.group(0)
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
        if not line == "":
            try:
                splitlinea = line.split('(', 1)
                functionName = splitlinea[0]
                if functionMap.has_key(functionName):
                    functionName = functionMap[functionName]
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
    specialCommandType = ""
    filesToTestFolder = []
    for line in document["coprocessor"]:
        if not line == "":
            try:
                if (line.lstrip()).startswith("//"):#if the line is a comment line then just write it out
                    f.write("\t" + line + "\n")
                    continue
                splitlinea = line.split('(',1)
                splitlineb = splitlinea[1].split(')',1)
                functionName = splitlinea[0]
                commentsRegex = re.compile("//.*$")
                coprocessor_cmd = False
                #if functionMap.has_key(functionName):
                if functionName in functionMap:
                    functionName = functionMap[functionName]
                    coprocessor_cmd = True
                if not skippedBitmaps:
                    if functionName == "BITMAP_HANDLE" or functionName == "BITMAP_SOURCE" or functionName == "BITMAP_LAYOUT" or functionName == "BITMAP_SIZE" or functionName == "CMD_SETFONT":
                        continue
                    else:
                        skippedBitmaps = True
                functionArgs = convertArgs(splitlineb[0])

                #if functionName == "Ft_Gpu_CoCmd_Snapshot":
                #    functionArgsSplit = functionArgs.split(',')
                #
                #    f.write("\tFt_App_WrCoCmd_Buffer(phost, DISPLAY()););\n")
                #    f.write("\tFt_Gpu_CoCmd_Swap(phost););\n")
                #
                #    f.write("\t/* Download the commands into fifo */);\n")
                #    f.write("\tFt_App_Flush_Co_Buffer(phost););\n")
                #
                #    f.write("\t/* Wait till coprocessor completes the operation */);\n")
                #    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost););\n")
                #
                #    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor+ );\n")
                #
                #
                #    f.write("\t/* Take snap shot of the current screen */\n")
                #    f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_SNAPSHOT);\n")
                #    f.write("\tFt_Gpu_Hal_WrCmd32(phost," + functionArgsSplit[0] + ");\n")
                #
                #    f.write("\t//timeout for snapshot to be performed by coprocessor\n")
                #
                #    f.write("\t/* Wait till coprocessor completes the operation */\n")
                #    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")
                #
                #    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n")
                #
                #    f.write("\t/* reconfigure the resolution wrt configuration */\n")
                #    f.write("\tFt_Gpu_Hal_Wr16(phost, REG_HSIZE,480);\n")
                #    f.write("\tFt_Gpu_Hal_Wr16(phost, REG_VSIZE,272);\n")
                #
                #    f.write("\tFt_Gpu_Hal_Sleep(100); //timeout for snapshot to be performed by coprocessor\n")
                #    f.write("\tFt_Gpu_CoCmd_Dlstart(phost);\n")
                #    f.write("\tFt_App_WrCoCmd_Buffer(phost, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));\n")
                #    f.write("\tFt_App_WrCoCmd_Buffer(phost, CLEAR(1, 1, 1));\n")
                #    f.write("\tFt_App_WrCoCmd_Buffer(phost, COLOR_RGB(255, 255, 255));\n")
                #
                #    functionName = ""

                if functionName == "Ft_Gpu_CoCmd_LoadImage":
                    functionArgsSplit = functionArgs.split(',')
                    #raise Exception(functionArgsSplit)
                    f.write("\tFt_App_Flush_Co_Buffer(phost);\n")
                    f.write("\tFt_Gpu_Hal_WaitCmdfifo_empty(phost);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, CMD_LOADIMAGE);\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, " + functionArgsSplit[0] + ");\n")
                    f.write("\tFt_Gpu_Hal_WrCmd32(phost, " + functionArgsSplit[1] + ");\n")
                    functionArgs = functionArgsSplit[0] + ","
                    functionArgs += functionArgsSplit[1]
                    try:
                        functionArgsSplit[2].decode('ascii')
                    except UnicodeDecodeError:
                        f.close()
                        raiseUnicodeError("Input file path")
                    functionArgsSplit[2] = re.sub(r'["]', "", functionArgsSplit[2])
                    #functionArgsSplit[2] = re.sub(r'[:]', '.', functionArgsSplit[2])
                    if '/' in functionArgsSplit[2]:
                        specialParameter = functionArgsSplit[2].rsplit('/',1)[1]
                        #f.close()
                        #raise Exception(specialParameter)
                    elif '\\' in functionArgsSplit[2]:
                        specialParameter = functionArgsSplit[2].rsplit('\\',1)[1]
                    else:
                        specialParameter = functionArgsSplit[2]
                    #f.close()
                    #raise Exception("loadimage error: " + specialParameter)
                    specialParameter = "..\\\\..\\\\..\\\\Test\\\\" + specialParameter
                    filesToTestFolder.append(functionArgsSplit[2])
                    f.write("\tloadDataToCoprocessorCMDfifo(\"" + specialParameter + "\");\n")
                    functionName = ""
                    #f.close()
                    #raise Exception(functionArgsSplit[2])
                    #specialCommandType = "Ft_Gpu_CoCmd_LoadImage"
                #The following commands don't take any parameters so there shouldn't be a comma after the phost
                parameterComma = ","
                if functionName == "Ft_Gpu_CoCmd_LoadIdentity" or functionName == "Ft_Gpu_CoCmd_Swap" or functionName == "Ft_Gpu_CoCmd_Stop" or functionName == "Ft_Gpu_CoCmd_SetMatrix" or functionName == "Ft_Gpu_CoCmd_ColdStart" or functionName == "Ft_Gpu_CoCmd_Dlstart" or functionName == "Ft_Gpu_CoCmd_ScreenSaver":
                    parameterComma = ""
                #attempt to append comments.
                comments = ""
                if len(functionName):
                    m = commentsRegex.match(splitlineb[1])
                    if m:
                        comments = m.group(0)
                    if coprocessor_cmd:
                            newline = "\t" + functionName + "(phost" + parameterComma + functionArgs + ");" + comments + "\n"
                    else:
                            newline = "\tFt_App_WrCoCmd_Buffer(phost" + parameterComma + functionName + "(" + functionArgs + "));" + comments + "\n"
                    f.write(newline)

            except:
                pass
        else:
            if skippedBitmaps:
                f.write("\t\n")

    f.write(endOfDisplayListSequence)
    f.write("\n")
    f.write(FT8xxMainFunctionEnd)
    f.close()


    #screenSize = document["ScreenSize"];
    generateProjectFiles(outDir, outName, name, filesToTestFolder, "480x272", document["project"]["device"])



    resultText += "<b>Output</b>:<p>Output files: " + outDir + "</p> <p>Project files: " + outDir + os.path.sep + name + "</p><p>\"" + name + HALProjectName + os.path.sep + "ReadMe.txt\" details the project folder structure.</p>"

    #outDir = outDir + "/" + name

    if sys.platform.startswith('darwin'):
        subprocess.call(('open', outName))
    elif os.name == 'nt':
        subprocess.call(['explorer', outDir])
    elif os.name == 'posix':
        subprocess.call(('xdg-open', outName))
    #print resultText
    return resultText
