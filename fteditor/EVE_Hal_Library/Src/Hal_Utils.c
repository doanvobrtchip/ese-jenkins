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
0.1 - date 2017.03.20 - Initial Version
*/

#pragma warning(disable: 4244 4018 4477 4101)

#include "Platform.h"
#include "Hal_Utils.h"
#include "App_Common.h"

/* API to give fadeout effect by changing the display PWM from 100 till 0 */
void fadeout(Gpu_Hal_Context_t *phost)
{
    int32_t i;

    for (i = 100; i >= 0; i -= 3)
    {
        Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);

        Gpu_Hal_Sleep(2);//sleep for 2 ms
    }
}


/* API to perform display fadein effect by changing the display PWM from 0 till 100 and finally 128 */
void fadein(Gpu_Hal_Context_t *phost)
{
    int32_t i;

    for (i = 0; i <=100 ; i += 3)
    {
        Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
        Gpu_Hal_Sleep(2);//sleep for 2 ms
    }
    /* Finally make the PWM 100% */
    i = 128;
    Gpu_Hal_Wr8(phost,REG_PWM_DUTY,i);
}

/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
void GPU_DLSwap(Gpu_Hal_Context_t *phost, uint8_t DL_Swap_Type)
{
    uint8_t Swap_Type = DLSWAP_FRAME,Swap_Done = DLSWAP_FRAME;

    if(DL_Swap_Type == DLSWAP_LINE)
    {
        Swap_Type = DLSWAP_LINE;
    }

    /* Perform a new DL swap */
    Gpu_Hal_Wr8(phost,REG_DLSWAP,Swap_Type);

    /* Wait till the swap is done */
    while(Swap_Done)
    {
        Swap_Done = Gpu_Hal_Rd8(phost,REG_DLSWAP);

        if(DLSWAP_DONE != Swap_Done)
        {
            Gpu_Hal_Sleep(10);//wait for 10ms
        }
    }
}

float_t cal_average(float_t * ptr_elements , uint16_t elements)
{
    float_t average = 0.0, sum = 0.0;
    uint16_t i = 0;

    for (i = 0; i < elements; i++)
    sum += *(ptr_elements + i);

    average = sum / elements;

    return(average);
}



#ifdef POLAR_UTIL

/* Optimized implementation of sin and cos table - precision is 16 bit */
PROGMEM prog_uint16_t sintab[] = {
    0, 402, 804, 1206, 1607, 2009, 2410, 2811, 3211, 3611, 4011, 4409, 4807, 5205, 5601, 5997, 6392,
    6786, 7179, 7571, 7961, 8351, 8739, 9126, 9511, 9895, 10278, 10659, 11038, 11416, 11792, 12166, 12539,
    12909, 13278, 13645, 14009, 14372, 14732, 15090, 15446, 15799, 16150, 16499, 16845, 17189, 17530, 17868,
    18204, 18537, 18867, 19194, 19519, 19840, 20159, 20474, 20787, 21096, 21402, 21705, 22004, 22301, 22594,
    22883, 23169, 23452, 23731, 24006, 24278, 24546, 24811, 25072, 25329, 25582, 25831, 26077, 26318, 26556, 26789,
    27019, 27244, 27466, 27683, 27896, 28105, 28309, 28510, 28706, 28897, 29085, 29268, 29446, 29621, 29790, 29955,
    30116, 30272, 30424, 30571, 30713, 30851, 30984, 31113, 31236, 31356, 31470, 31580, 31684, 31785, 31880, 31970,
    32056, 32137, 32213, 32284, 32350, 32412, 32468, 32520, 32567, 32609, 32646, 32678, 32705, 32727, 32744, 32757,
    32764, 32767, 32764};


int16_t qsin(uint16_t a)
{
    uint8_t f;
    int16_t s0,s1;

    if (a & 32768)
    return -qsin(a & 32767);
    if (a & 16384)
    a = 32768 - a;
    f = a & 127;
    s0 = pgm_read_word(sintab + (a >> 7));
    s1 = pgm_read_word(sintab + (a >> 7) + 1);
    return (s0 + ((int32_t)f * (s1 - s0) >> 7));
}

/* cos funtion */
int16_t qcos(uint16_t a)
{
    return (qsin(a + 16384));
}

void polarxy(int32_t r, float_t th, int32_t *x, int32_t *y, int32_t ox, int32_t oy)
{
    *x = (16 * ox) + (((long)r * qsin(th)) >> 11) + 16 ;
    *y = (16 * oy) - (((long)r * qcos(th)) >> 11);
}


void polar(Gpu_Hal_Context_t *phost, int32_t r, float_t th, int32_t ox, int32_t oy)
{
    int32_t x, y;
    polarxy(r, th, &x, &y, ox, oy);
    App_WrCoCmd_Buffer(phost,VERTEX2F(x,y));

}

float_t da(float_t i, int16_t degree) /* 1 uint = 0.5 degree. 1 circle = 720 degree*/
{
    return (i - degree)* 32768 /360 ;
}
#endif



uint32_t GET_ASTC_FORMAT(uint16_t w, uint16_t h)
{
#if defined(BT815_ENABLE) || defined(BT816_ENABLE)
	if ((w == 4) && (h == 4))  return COMPRESSED_RGBA_ASTC_4x4_KHR;
	else if ((w == 5) && (h == 4))  return COMPRESSED_RGBA_ASTC_5x4_KHR;
	else if ((w == 5) && (h == 5))  return COMPRESSED_RGBA_ASTC_5x5_KHR;
	else if ((w == 6) && (h == 5))  return COMPRESSED_RGBA_ASTC_6x5_KHR;
	else if ((w == 6) && (h == 6))  return COMPRESSED_RGBA_ASTC_6x6_KHR;
	else if ((w == 8) && (h == 5))  return COMPRESSED_RGBA_ASTC_8x5_KHR;
	else if ((w == 8) && (h == 6))  return COMPRESSED_RGBA_ASTC_8x6_KHR;
	else if ((w == 8) && (h == 8))  return COMPRESSED_RGBA_ASTC_8x8_KHR;
	else if ((w == 10) && (h == 5))  return COMPRESSED_RGBA_ASTC_10x5_KHR;
	else if ((w == 10) && (h == 6))  return COMPRESSED_RGBA_ASTC_10x6_KHR;
	else if ((w == 10) && (h == 8))  return COMPRESSED_RGBA_ASTC_10x8_KHR;
	else if ((w == 10) && (h == 10))  return COMPRESSED_RGBA_ASTC_10x10_KHR;
	else if ((w == 12) && (h == 10))  return COMPRESSED_RGBA_ASTC_12x10_KHR;
	else if ((w == 12) && (h == 12))  return COMPRESSED_RGBA_ASTC_12x12_KHR;
	else return 0;
#else
	return 0;
#endif
}

/* Helper functions for tiling ASTC */
void astc_tile2(uint8_t *iData, uint16_t bw, uint16_t bh, uint32_t size, uint8_t *oData)
{
	uint32_t i, j, next;
	uint8_t *d, *r;
	d = iData;
	r = oData;
	for (j = 0; j < bh - 1; j += 2)
	{
		for (i = 0; i < bw; i += 2)
		{
			if (i < (bw - 1)) {
				next = 16 * (bw * j + i);
				memcpy(r, d + next, 16);
				r += 16;

				next = 16 * (bw * (j + 1) + i);
				memcpy(r, d + next, 16);
				r += 16;

				next = 16 * (bw * (j + 1) + (i + 1));
				memcpy(r, d + next, 16);
				r += 16;

				next = 16 * (bw * j + (i + 1));
				memcpy(r, d + next, 16);
				r += 16;

			}
			else {
				next = 16 * (bw * j + i);
				memcpy(r, d + next, 16);
				r += 16;

				next = 16 * (bw * (j + 1) + i);
				memcpy(r, d + next, 16);
				r += 16;
			}
		}

	}
	if (bh & 1) {
		for (i = bw * (bh - 1); i < (size)/16 ; i += 1)
		{
			next = 16 * i;
			memcpy(r, d + next, 16);
			r += 16;
		}
	}
}

#if defined(FT_81X_ENABLE) || defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
#ifdef FT9XX_PLATFORM
#include "ff.h"
void Screen_Snapshot2(Gpu_Hal_Context_t *phost, FIL *pfile, uint8_t *pdlarray, int32_t dlsize, uint8_t rotate_index, uint16_t width, uint16_t height)
{

	//int32_t i, width = DispWidth, height = DispHeight, snapshot;
	//int32_t i, width = 32, height = 32, snapshot;
	uint32_t sn_w, sn_h;
	uint8_t *dlarray;
	uint16_t dlcmd_index, i;
	uint32_t p;
	bool_t landscape = TRUE;
	switch (rotate_index) {
	case 0:
	case 1:
	case 4:
	case 5: /* Landscape */
		landscape = TRUE;
		break;
	default: /* Portrait */
		landscape = FALSE;
		break;
	}

	if (NULL == pfile)
	{
		/* error in file operations*/
		return;
	}
	if (landscape) {
		sn_w = width;
		sn_h = height;
	}
	else {
		sn_w = height;
		sn_h = width;
	}
#define RAM_SNAPSHOT (RAM_G_SIZE - 20*1024)

	dlcmd_index = Gpu_Hal_Rd16(phost, REG_CMD_DL);

	for (i = 0;i<sn_h;i++) {
		dlarray = malloc(sn_w * 2 * 2);
		if (NULL == dlarray)
		{
			printf("Error in memory allocation %s %s", __FUNCTION__, __LINE__);
			return;
		}
		Gpu_CoCmd_Snapshot2(phost, 0x20, RAM_SNAPSHOT, 0, i, sn_w * 2, 1);
		App_Flush_Co_Buffer(phost);
		Gpu_Hal_WaitCmdfifo_empty(phost);

		Gpu_Hal_RdMem(phost, RAM_SNAPSHOT, dlarray, sn_w * 2 * 2);


		f_write(pfile, dlarray, sn_w * 2 * 2, &p);
		free(dlarray);
	}

}
#else
void Screen_Snapshot2(Gpu_Hal_Context_t *phost, FILE *pfile, uint8_t *pdlarray, int32_t dlsize, uint8_t rotate_index, uint16_t width, uint16_t height)
{
#ifndef RAM_SNAPSHOT
#define RAM_SNAPSHOT (20*1024)
#endif
	//int32_t i, width = DispWidth, height = DispHeight, snapshot;
	//int32_t i, width = 32, height = 32, snapshot;
	uint32_t size_image;
	uint8_t *dlarray;
	uint16_t dlcmd_index;
	bool_t landscape = TRUE;
	switch (rotate_index) {
	case 0:
	case 1:
	case 4:
	case 5: /* Landscape */
		landscape = TRUE;
		break;
	default: /* Portrait */
		landscape = FALSE;
		break;
	}

	dlarray = malloc(width * 2 * height);
	if (NULL == dlarray)
	{
		printf("Error in memory allocation %s %s", __FUNCTION__, __LINE__);
		return;
	}

	if (NULL == pfile)
	{
		/* error in file operations*/
		return;
	}
	if (landscape) {
		Gpu_CoCmd_Snapshot2(phost, 0x20, RAM_SNAPSHOT, 0, 0, width * 2, height / 2);
	}
	else {
		Gpu_CoCmd_Snapshot2(phost, 0x20, RAM_SNAPSHOT, 0, 0, height * 2, width / 2);
	}
	App_Flush_Co_Buffer(phost);
	Gpu_Hal_WaitCmdfifo_empty(phost);
	dlcmd_index = Gpu_Hal_Rd16(phost, REG_CMD_DL);
	Gpu_Hal_RdMem(phost, RAM_SNAPSHOT, dlarray, width * 2 * height);
	fwrite(dlarray, 1, width * 2 * height, pfile);


	if (landscape) {
		Gpu_CoCmd_Snapshot2(phost, 0x20, RAM_SNAPSHOT, 0, height / 2, width * 2, height / 2);
	}
	else {
		Gpu_CoCmd_Snapshot2(phost, 0x20, RAM_SNAPSHOT, 0, width / 2, height * 2, width / 2);
	}
	App_Flush_Co_Buffer(phost);
	Gpu_Hal_WaitCmdfifo_empty(phost);
	dlcmd_index = Gpu_Hal_Rd16(phost, REG_CMD_DL);
	Gpu_Hal_RdMem(phost, RAM_SNAPSHOT, dlarray, width * 2 * height);
	fwrite(dlarray, 1, width * 2 * height, pfile);
	free(dlarray);

}
#endif
#endif

#if defined(FT_81X_ENABLE) || defined(BT_81X_ENABLE) || defined(BT_81XA_ENABLE)
void Set_GpuClock(Gpu_Hal_Context_t *phost)
{
	static uint32_t x = 1;
	Gpu_CoCmd_Sync(phost);
	Gpu_CoCmd_Memcpy(phost, x * 4, REG_CLOCK, 4);
	App_Flush_Co_Buffer(phost);
	Gpu_Hal_WaitCmdfifo_empty(phost);
	x = x ^ 1;
}

uint32_t Get_GpuClock(Gpu_Hal_Context_t *phost)
{
	uint32_t a = Gpu_Hal_Rd32(phost, 0);
	uint32_t b = Gpu_Hal_Rd32(phost, 4);
	return (a < b) ? (b - a) : (a - b);
}
#endif
