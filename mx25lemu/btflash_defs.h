/*
BT8XX Emulator Library
Copyright (C) 2017  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef MX25LEMU_BTFLASH_DEFS_H
#define MX25LEMU_BTFLASH_DEFS_H

// SPI Signals
#define BTFLASH_SPI4_MOSI (1 << 0)
#define BTFLASH_SPI4_MISO (1 << 1)
#define BTFLASH_SPI4_D0 (1 << 0)
#define BTFLASH_SPI4_D1 (1 << 1)
#define BTFLASH_SPI4_D2 (1 << 2)
#define BTFLASH_SPI4_WP (1 << 2)
#define BTFLASH_SPI4_D3 (1 << 3)
#define BTFLASH_SPI4_CS (1 << 4)
#define BTFLASH_SPI4_SCK (1 << 5)

// SPI Data Masks
#define BTFLASH_SPI4_MASK_SI (BTFLASH_SPI4_MOSI)
#define BTFLASH_SPI4_MASK_SO (BTFLASH_SPI4_MISO)
#define BTFLASH_SPI4_MASK_D2 (0x3)
#define BTFLASH_SPI4_MASK_D4 (0xF)
#define BTFLASH_SPI4_MASK_NONE (0)

// SPI Signals to Data
#define BTFLASH_SPI4_GET_SI(signal) ((signal) & BTFLASH_SPI4_MASK_SI)
#define BTFLASH_SPI4_GET_SO(signal) (((signal) & BTFLASH_SPI4_MASK_SO) >> 1)
#define BTFLASH_SPI4_GET_D4(signal) ((signal) & BTFLASH_SPI4_MASK_D4)

// Commands
#define BTFLASH_CMD_WREN (0x06) /* Write Enable */
#define BTFLASH_CMD_WRDI (0x04) /* Write Disable */
#define BTFLASH_CMD_RDID (0x9F) /* Read Identification */
#define BTFLASH_CMD_RDSR (0x05) /* Read Status Register */
#define BTFLASH_CMD_WRSR (0x01) /* Write Status Register */
#define BTFLASH_CMD_FASTDTRD (0x0D) /* Fast DT Read */
#define BTFLASH_CMD_2DTRD (0xBD) /* Dual I/O DT Read */
#define BTFLASH_CMD_4DTRD (0xED) /* Quad I/O DT Read */
#define BTFLASH_CMD_READ (0x03) /* Read Data */
#define BTFLASH_CMD_FAST_READ (0x0B) /* Fast Read Data */
#define BTFLASH_CMD_RDSFDP (0x5A) /* Read SFDP */
#define BTFLASH_CMD_2READ (0xBB) /* 2x IO Read */
#define BTFLASH_CMD_4READ (0xEB) /* 4x IO Read */
#define BTFLASH_CMD_4PP (0x38) /* Quad Page Program */
#define BTFLASH_CMD_SE (0x20) /* Sector Erase */
#define BTFLASH_CMD_BE (0xD8) /* Block Erase */
#define BTFLASH_CMD_BE32K (0x52) /* Block Erase 32kB */
#define BTFLASH_CMD_CE_60 (0x60) /* Chip Erase */
#define BTFLASH_CMD_CE_C7 (0xC7) /* Chip Erase */
#define BTFLASH_CMD_PP (0x02) /* Page Program */
#define BTFLASH_CMD_CP (0xAD) /* Continuously Program mode */
#define BTFLASH_CMD_DP (0xB9) /* Deep Power Down */
#define BTFLASH_CMD_RDP (0xAB) /* Release from Deep Power Down */
#define BTFLASH_CMD_RES (0xAB) /* Read Electronic ID */
#define BTFLASH_CMD_REMS (0x90) /* Read Electronic Manufacturer and Device ID */
#define BTFLASH_CMD_REMS2 (0xEF) /* Read ID for 2x IO Mode */
#define BTFLASH_CMD_REMS4 (0xDF) /* Read ID for 4x IO Mode */
#define BTFLASH_CMD_REMS4D (0xCF) /* Read ID for 4x IO DT Mode */
#define BTFLASH_CMD_ENSO (0xB1) /* Enter Secured OTP */
#define BTFLASH_CMD_EXSO (0xC1) /* Exit Secured OTP */
#define BTFLASH_CMD_RDSCUR (0x2B) /* Read Security Register */
#define BTFLASH_CMD_WRSCUR (0x2F) /* Write Security Register */
#define BTFLASH_CMD_ESRY (0x70) /* Enable SO to output RY/BY# */
#define BTFLASH_CMD_DSRY (0x80) /* Disable SO to output RY/BY# */
#define BTFLASH_CMD_CLSR (0x30) /* Clear SR Fail Flags */
#define BTFLASH_CMD_HPM (0xA3) /* High Performance Enable Mode */
#define BTFLASH_CMD_WPSEL (0x68) /* Write Protection Selection */
#define BTFLASH_CMD_SBLK (0x36) /* Single Block Lock */
#define BTFLASH_CMD_SBULK (0x39) /* Single Block Unlock */
#define BTFLASH_CMD_RDBLOCK (0x3C) /* Block Protect Read */
#define BTFLASH_CMD_GBLK (0x7E) /* Gang Block Lock */
#define BTFLASH_CMD_GBULK (0x98) /* Gang Block Unlock */

#define BTFLASH_CMD_RSTEN (0x66) /* Reset Enable */
#define BTFLASH_CMD_RST (0x99) /* Reset */
#define BTFLASH_CMD_EN4B (0xB7) /* Enter 4-byte mode */
#define BTFLASH_CMD_EX4B (0xE9) /* Exit 4-byte mode */
#define BTFLASH_CMD_4READ_TOP (0xEA) /* 4x IO Read, top 128 Mbit */

#define BTFLASH_CMD_NOOP_EXITQPI (0xFF) /* Exit QPI Mode */

// Status Registers
#define BTFLASH_STATUS_WIP_FLAG (1 << 0) /* Write In Progress */
#define BTFLASH_STATUS_WEL_FLAG (1 << 1) /* Write Enable Latch */
#define BTFLASH_STATUS_BP0_FLAG (1 << 2) /* Block Protect */
#define BTFLASH_STATUS_BP1_FLAG (1 << 3) /* Block Protect */
#define BTFLASH_STATUS_BP2_FLAG (1 << 4) /* Block Protect */
#define BTFLASH_STATUS_BP3_FLAG (1 << 5) /* Block Protect */
#define BTFLASH_STATUS_QE_FLAG (1 << 6) /* Quad Enable */
#define BTFLASH_STATUS_SRWD_FLAG (1 << 7) /* Status Register Write Protect */

// Configuration Registers
#define BTFLASH_CONFIGURATION_ODS0 (1 << 0) /* Output Driver Strength 0 */
#define BTFLASH_CONFIGURATION_ODS1 (1 << 1) /* Output Driver Strength 1 */
#define BTFLASH_CONFIGURATION_ODS2 (1 << 2) /* Output Driver Strength 2 */
#define BTFLASH_CONFIGURATION_TB (1 << 3) /* Top/Bottom Selected */
#define BTFLASH_CONFIGURATION_4BYTE (1 << 5) /* 4BYTE Indicator bit */
#define BTFLASH_CONFIGURATION_DC0 (1 << 6) /* Dummy Cycle 0 */
#define BTFLASH_CONFIGURATION_DC1 (1 << 7) /* Dummy Cycle 1 */

#define BTFLASH_CONFIGRATION_GET_DC(v) ((v >> 6) & 0x03)

// Clock edge select
#define BTFLASH_CLOCK_RISING 1
#define BTFLASH_CLOCK_FALLING 2

// Run state
#define BTFLASH_STATE_UNKNOWN 0
#define BTFLASH_STATE_COMMAND 1
#define BTFLASH_STATE_IGNORE 2
#define BTFLASH_STATE_UNSUPPORTED 2
#define BTFLASH_STATE_BLANK 3
#define BTFLASH_STATE_OUT_U8 4
#define BTFLASH_STATE_OUT_U16 5
#define BTFLASH_STATE_OUT_U24 6
#define BTFLASH_STATE_OUT_U32 7
#define BTFLASH_STATE_OUT_U8_ARRAY 8
#define BTFLASH_STATE_OUT_U8_ARRAY_DT 9
#define BTFLASH_STATE_NEXT 11
#define BTFLASH_STATE_WRSR 12
#define BTFLASH_STATE_REMS_ADDR 13
#define BTFLASH_STATE_RDSFDP_ADDR 14
#define BTFLASH_STATE_READ_ADDR 15
#define BTFLASH_STATE_FAST_READ_ADDR 16
#define BTFLASH_STATE_FASTDTRD_ADDR 17
#define BTFLASH_STATE_CS_HIGH_COMMAND 18 /* Execute specified command exactly when CS goes high */
#define BTFLASH_STATE_4READ_ADDR 19
#define BTFLASH_STATE_4READ_PE 20
#define BTFLASH_STATE_PP_ADDR 21
#define BTFLASH_STATE_PP_READ 22
#define BTFLASH_STATE_SE_ADDR 23
#define BTFLASH_STATE_4DTRD_ADDR 24
#define BTFLASH_STATE_4DTRD_PE 25

#endif /* #ifndef MX25LEMU_BTFLASH_DEFS_H */

/* end of file */
