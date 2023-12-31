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

Author : BRIDGETEK

Revision History:
0.1 - date 2017.03.24 - Initial Version
*/
#ifndef _Skeleton_H_
#define _Skeleton_H_

#if  defined(FT900_PLATFORM)
	#define DATASIZETESTING	                        (1024)
	#define DATASIZETESTING_PROGRESSIVE             2048      
	#define DATASIZETESTING_SPIMAXBYTES_PERCALL     4096
	#define HAL_TEST_RAM_G_SIZE                     (8*1024)

	#define TEST_RAM_TRANSFER8_ADDR (4*1024)
	#define TEST_RAM_TRANSFER16_ADDR (5*1024)
	#define TEST_RAM_TRANSFER32_ADDR (6*1024)
	#define TEST_RAM_WR_RD_MEM_ADDR (1024)
#elif  defined(FT93X_PLATFORM)
	#define DATASIZETESTING	                        (1024)
	#define DATASIZETESTING_PROGRESSIVE             256       
	#define DATASIZETESTING_SPIMAXBYTES_PERCALL     4096
	#define HAL_TEST_RAM_G_SIZE                     (6*1024)

	#define TEST_RAM_TRANSFER8_ADDR (4*1024)
	#define TEST_RAM_TRANSFER16_ADDR (5*1024)
	#define TEST_RAM_TRANSFER32_ADDR (6*1024)
	#define TEST_RAM_WR_RD_MEM_ADDR (1024)
#else
	#define DATASIZETESTING	                        (1024*10)
	#define DATASIZETESTING_PROGRESSIVE             32768       //32KB
	#define DATASIZETESTING_SPIMAXBYTES_PERCALL     65535
    #ifdef FT81X_ENABLE
	#define HAL_TEST_RAM_G_SIZE                     (1024*1024UL)
    #else
    #define HAL_TEST_RAM_G_SIZE                     (256*1024UL)
    #endif
	#define TEST_RAM_TRANSFER8_ADDR  (RAM_G)
	#define TEST_RAM_TRANSFER16_ADDR (RAM_G+1024*1)
	#define TEST_RAM_TRANSFER32_ADDR (RAM_G+1024*2)
	#define TEST_RAM_WR_RD_MEM_ADDR  (RAM_G)
    #define RAM_CMD_SIZE (1024*4)

#endif

#define HAL_TEST_RDWR_RAM_G_PROFILING_TRIALS   100


#define HAL_TEST_RDWR_RAMCMD_SERVICE                1   //tests function Gpu_Hal_WrCmdBuf
#define HAL_TEST_RDWR_8BITS                         1   //tests functions Gpu_Hal_Wr8, Gpu_Hal_Rd8
#define HAL_TEST_RDWR_LOOP_1BYTE                    1   //test with 1 byte write followed by 1 byte read in multiple iterations
#define HAL_TEST_RDWR_16BITS                        1	  //tests functions Gpu_Hal_Wr16, Gpu_Hal_Rd16
#define HAL_TEST_RDWR_32BITS                        1   //tests functions Gpu_Hal_Wr32, Gpu_Hal_Rd32
#define HAL_TEST_RDWR_MEMCHUNK                      1   //tests functions Gpu_Hal_WrMem, Gpu_Hal_RdMem
#define HAL_TEST_RDWR_MEMCHUNK_BASIC_COMFUNC        1   //testing functions Gpu_Hal_FT4222_Wr, Gpu_Hal_FT4222_Rd
#define HAL_TEST_RDWR_SIZEPROGRESSIVE_MEMCHUNK      1   //speed of read and write profiling done
#define HAL_TEST_RDWR_RAM_G_PROFILING                1   //1MByte read and write profiling test 
#define HAL_TEST_PWR_MODE                           1   //tests Gpu_HostCommand , Gpu_Hal_DeInit
#define HAL_TEST_RDWR_SPIMAXBYTES_PERCALL           1   //speed of read and write profiling done

#define HAL_TEST_RDWR_RAM_G_PROFILING                1   //1MByte read and write profiling test 

#endif /* _Skeleton_H_ */
