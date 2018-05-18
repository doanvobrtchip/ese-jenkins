/*

Copyright (c) Bridgetek Pte Ltd

THIS SOFTWARE IS PROVIDED BY BRIDGETEK PTE LTD "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
BRIDGETEK PTE LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BRIDGETEK DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON BRIDGETEK PARTS.

BRIDGETEK DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Abstract:

Application to demonstrate function of EVE.

Author : Bridgetek

Revision History:
0.1 - date 2017.03.24 - Initial version
*/

#include "Platform.h"
#include "App_Common.h"
#include "Skeleton.h"
#include "Assets.h"

#ifdef MSVC_PLATFORM
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <time.h>
#endif

#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
	//#define HAL_CLK_COMPUTE_TEST 0
	#define getClock() clock()
#elif ( defined(FT900_PLATFORM) || defined(FT93X_PLATFORM) )
	#include <ft900.h>
	#define getClock() get_millis()
	#ifndef CLOCKS_PER_SEC
	#define CLOCKS_PER_SEC 1000
	#endif
#else /* ARDUINO_PLATFORM */
	#define getClock() millis()     //Arduino, MSVC have their own millis()
	#define CLOCKS_PER_SEC (1000)   //Need divide 1000 to get millisecond
#endif

#if defined(ARDUINO_PLATFORM)
const PROGMEM char * const info[] =
#else
char *info[] =
#endif
{ "EVE Skeleton Application",
"",
"",
""
};

/* Global used for buffer optimization */
Gpu_Hal_Context_t host;

#define SCRATCH_BUFF_SZ (8 * 1024)

void loadDataToCoprocessorCMDfifo(Gpu_Hal_Context_t *phost, char8_t *fileName)
{
#if defined(FT900_PLATFORM)
    uint32_t fResult, fileLen;
    uint32_t bytesread;
    FIL CurFile;
    uint8_t pBuff[SCRATCH_BUFF_SZ];
    fResult = f_open(&CurFile, fileName, FA_READ);

    if (fResult == FR_OK)
    {
        fileLen = f_size(&CurFile);
        while (fileLen > 0) {
            uint32_t blocklen = fileLen>SCRATCH_BUFF_SZ ? SCRATCH_BUFF_SZ : fileLen;
            fResult = f_read(&CurFile, pBuff, blocklen, &bytesread);
            fileLen -= bytesread;
            Gpu_Hal_WrCmdBuf(phost, pBuff, bytesread);//alignment is already taken care by this api
        }
        f_close(&CurFile);
    }
    else
    {
        printf("Unable to open file\\n");
    }

#else
    FILE *fp;
    uint32_t fileLen;
    uint8_t pBuff[SCRATCH_BUFF_SZ];
    fp = fopen(fileName, "rb+");

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        fileLen = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        while (fileLen > 0)
        {
            uint32_t blocklen = fileLen>SCRATCH_BUFF_SZ ? SCRATCH_BUFF_SZ : fileLen;
            fread(pBuff, 1, blocklen, fp);
            fileLen -= blocklen;
            Gpu_Hal_WrCmdBuf(phost, pBuff, blocklen);//alignment is already taken care by this api
        }
        fclose(fp);
    }
    else
    {
        printf("Unable to open file: %s\\n", fileName);
    }
#endif
}

void loadDataToCoprocessorCMDfifo_nowait(Gpu_Hal_Context_t *phost, char8_t *fileName)
{
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    uint32_t filesz, currchunk, bytesread, cmdrd, cmdwr;
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
    else {
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
        int32_t availfreesz = 0, freadbufffill = 0, chunkfilled = 0;
        uint8_t *pbuff = g_scratch;
        while (filesz > 0)
        {
            availfreesz = Gpu_Hal_Rd32(phost, REG_CMDB_SPACE);
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
                    Gpu_Hal_WrMem(phost, REG_CMDB_WRITE, pbuff, availfreesz);
#elif defined(FT900_PLATFORM)
                    Gpu_Hal_StartTransfer(phost, GPU_WRITE, REG_CMDB_WRITE);
                    spi_writen(SPIM, pbuff, availfreesz);
                    Gpu_Hal_EndTransfer(phost);
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

void loadDataToCoprocessorMediafifo(Gpu_Hal_Context_t *phost, char8_t *fileName, uint32_t mediaFIFOAddr, uint32_t mediaFIOFLen)
{
    uint8_t g_scratch[SCRATCH_BUFF_SZ];
    Fifo_t stFifo;
    uint32_t i;
    uint32_t filesz, currchunk, bytesread, cmdrd, cmdwr;
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
    FILE *pFile;
    pFile = fopen(fileName, "rb");
    if (pFile == NULL)
#elif defined(FT900_PLATFORM)
    FIL CurFile;
    fResult = f_open(&CurFile, fileName, FA_READ | FA_OPEN_EXISTING);
    if (fResult != FR_OK)
#endif
    {
        printf("Unable to open file.\\n");
    }
    else
    {
        //initialize application media fifo structure
        Fifo_Init(&stFifo, mediaFIFOAddr, mediaFIOFLen, REG_MEDIAFIFO_READ, REG_MEDIAFIFO_WRITE);
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        fseek(pFile, 0, SEEK_END);
        filesz = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
#elif defined(FT900_PLATFORM)
        fResult = f_lseek(&CurFile, 0);
        filesz = f_size(&CurFile);
#endif

        /* fill the complete fifo buffer before entering into steady state */
#if defined(FT900_PLATFORM)
        fResult = f_lseek(&CurFile, 0);
#endif

        filesz -= stFifo.fifo_wp;
        cmdrd = Gpu_Hal_Rd16(phost, REG_CMD_READ);
        cmdwr = Gpu_Hal_Rd16(phost, REG_CMD_WRITE);
        while ((cmdrd != cmdwr) || (filesz > 0))  //loop till end of the file
        {
            if (filesz > 0) {
                if (filesz > SCRATCH_BUFF_SZ) {
                    currchunk = SCRATCH_BUFF_SZ;
                }
                else {
                    currchunk = filesz;
                }
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
                bytesread = fread(g_scratch, 1, currchunk, pFile);
#elif defined(FT900_PLATFORM)
                fResult = f_read(&CurFile, g_scratch, currchunk, &bytesread);
#endif
                Fifo_WriteWait(phost, &stFifo, g_scratch, bytesread); //download the whole chunk into ring buffer - blocking call

                filesz -= currchunk;
            }

            cmdrd = Gpu_Hal_Rd16(phost, REG_CMD_READ);
            cmdwr = Gpu_Hal_Rd16(phost, REG_CMD_WRITE);
        }
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
        fclose(pFile);
#elif defined(FT900_PLATFORM)
        f_close(&CurFile);
#endif
    }
}


void Skeleton(Gpu_Hal_Context_t *phost)
{
/*RESERVED_FOR_EXPORTING_FROM_ESE*/
}

#if defined (MSVC_PLATFORM) || defined (FT900_PLATFORM) || defined (FT93X_PLATFORM)
/* Main entry point */
int32_t main(int32_t argc,char8_t *argv[])
#endif
#if defined(ARDUINO_PLATFORM)||defined(MSVC_FT800EMU)
void setup()
#endif
{
    /* Init HW Hal */
    App_Common_Init(&host);
    
    /* Our main application */
    Skeleton(&host);
    
    App_WrCoCmd_Buffer(&host, DISPLAY());
    //swap the current display list with the new display list
    Gpu_CoCmd_Swap(&host);
    App_Flush_Co_Buffer(&host);
    Gpu_Hal_WaitCmdfifo_empty(&host);
    
    /* End of command sequence */
    Gpu_CoCmd_Dlstart(&host);
    Gpu_Copro_SendCmd(&host, CMD_STOP);
    App_Flush_Co_Buffer(&host);
    Gpu_Hal_WaitCmdfifo_empty(&host);
    Gpu_Hal_Sleep(1000);
    
    /* Close all the opened handles */
    App_Common_Close(&host);

	#if defined (MSVC_PLATFORM) || defined (FT900_PLATFORM) || defined (FT93X_PLATFORM)
    return 0;
    #endif
}


void loop()
{
}
/* Nothing beyond this */


