/**
* This source code ("the Software") is provided by Bridgetek Pte Ltd
* ("Bridgetek") subject to the licence terms set out
*   http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
* You must read the Licence Terms before downloading or using the Software.
* By installing or using the Software you agree to the Licence Terms. If you
* do not agree to the Licence Terms then do not download or use the Software.
*
* Without prejudice to the Licence Terms, here is a summary of some of the key
* terms of the Licence Terms (and in the event of any conflict between this
* summary and the Licence Terms then the text of the Licence Terms will
* prevail).
*
* The Software is provided "as is".
* There are no warranties (or similar) in relation to the quality of the
* Software. You use it at your own risk.
* The Software should not be used in, or for, any medical device, system or
* appliance. There are exclusions of Bridgetek liability for certain types of loss
* such as: special loss or damage; incidental loss or damage; indirect or
* consequential loss or damage; loss of income; loss of business; loss of
* profits; loss of revenue; loss of contracts; business interruption; loss of
* the use of money or anticipated savings; loss of information; loss of
* opportunity; loss of goodwill or reputation; and/or loss of, damage to or
* corruption of data.
* There is a monetary cap on Bridgetek's liability.
* The Software may have subsequently been amended by another user and then
* distributed by that other user ("Adapted Software").  If so that user may
* have additional licence terms that apply to those amendments. However, Bridgetek
* has no liability in relation to those amendments.
*/

#ifndef EVE_UTIL__H
#define EVE_UTIL__H
#include "EVE_HalDefs.h"

/* Command line device selection utility */
#if defined(WIN32)
EVE_HAL_EXPORT void EVE_Util_selectDeviceInteractive(_Out_ EVE_CHIPID_T *chipId, _Out_ size_t *deviceIdx);
#else
static inline void EVE_Util_selectDeviceInteractive(EVE_CHIPID_T *chipId, size_t *deviceIdx)
{
	chipId = EVE_SUPPORT_CHIPID;
	deviceIdx = -1;
}
#endif

/* Command line device selection utility.
Provides selection of flash file, and option to write the flash file to the device.
Parameter `flashFile` is only relevant for Windows build.
Falls back to no interactivity on FT9XX platform */
EVE_HAL_EXPORT bool EVE_Util_openDeviceInteractive(_Out_ EVE_HalContext *phost, _In_opt_z_ wchar_t *flashFile);

typedef struct EVE_BootupParameters
{
#if (EVE_SUPPORT_CHIPID >= EVE_FT810) || defined(EVE_MULTI_TARGET)
	/* Clock PLL multiplier (ft81x: 5, 60MHz, bt81x: 6, 72MHz) */
	EVE_81X_PLL_FREQ_T SystemClock;
#endif

	/* External oscillator (default: false) */
	bool ExternalOsc;

	/* SPI */
#if (EVE_SUPPORT_CHIPID >= EVE_FT810) || defined(EVE_MULTI_TARGET)
	EVE_SPI_CHANNELS_T SpiChannels; /* Variable to contain single/dual/quad channels */
	uint8_t SpiDummyBytes; /* Number of dummy bytes as 1 or 2 for SPI read */
#endif

} EVE_BootupParameters;

typedef struct EVE_ConfigParameters
{
	/* Display */
	int16_t Width;
	int16_t Height;
	int16_t HCycle;
	int16_t HOffset;
	int16_t HSync0;
	int16_t HSync1;
	int16_t VCycle;
	int16_t VOffset;
	int16_t VSync0;
	int16_t VSync1;
	uint8_t PCLK;
	int8_t Swizzle;
	int8_t PCLKPol;
	int8_t CSpread;
	uint16_t OutBits;
	bool Dither;
	/* TODO: 
	AdaptiveFramerate
	*/

#ifdef EVE_SUPPORT_HSF
	/* Physical horizontal pixels. Set to 0 to disable HSF. */
	int16_t WidthHsf;
#endif

} EVE_ConfigParameters;

/* Get the default bootup parameters. */
EVE_HAL_EXPORT void EVE_Util_bootupDefaults(EVE_HalContext *phost, EVE_BootupParameters *bootup);

/* Boot up the device. Obtains the chip Id. Sets up clock and SPI speed. */
EVE_HAL_EXPORT bool EVE_Util_bootup(EVE_HalContext *phost, EVE_BootupParameters *bootup);

/* Get the default configuration parameters. */
EVE_HAL_EXPORT void EVE_Util_configDefaults(EVE_HalContext *phost, EVE_ConfigParameters *config);

/* Boot up the device. Configures the display, resets coprocessor state if necessary. */
EVE_HAL_EXPORT bool EVE_Util_config(EVE_HalContext *phost, EVE_ConfigParameters *config);

/* Complementary of bootup. Does not close the HAL context. */
EVE_HAL_EXPORT void EVE_Util_shutdown(EVE_HalContext *phost);

/* Sets the display list to a blank cleared screen. */
EVE_HAL_EXPORT void EVE_Util_clearScreen(EVE_HalContext *phost);

/* Resets the coprocessor.
To be used after a coprocessor fault, or to exit CMD_LOGO. 
After a reset, flash will be in attached state (not in full speed).
Coprocessor will be set to the latest API level. */
EVE_HAL_EXPORT bool EVE_Util_resetCoprocessor(EVE_HalContext *phost);

/* Calls EVE_Util_bootup and EVE_Util_config using the default parameters */
EVE_HAL_EXPORT bool EVE_Util_bootupConfig(EVE_HalContext *phost);

#endif /* #ifndef EVE_HAL_INCL__H */

/* end of file */