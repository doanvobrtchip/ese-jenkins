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
#define BTFLASH_SPI4_D3 (1 << 3)
#define BTFLASH_SPI4_CS (1 << 4)
#define BTFLASH_SPI4_SCK (1 << 5)

// SPI Data Masks
#define BTFLASH_SPI4_MASK_SI (BTFLASH_SPI4_MOSI)
#define BTFLASH_SPI4_MASK_SO (BTFLASH_SPI4_MISO)
#define BTFLASH_SPI4_MASK_D4 (0xF)

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
#define BTFLASH_CMD_RDP (0xAB) /* Release from Deep Power Down ¨*/
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

#define BTFLASH_CMD_NOOP_66 (0x66)
#define BTFLASH_CMD_NOOP_FF (0xFF)

// Status Registers
#define BTFLASH_STATUS_WIP_FLAG (1 << 0) /* Write In Progress */
#define BTFLASH_STATUS_WEL_FLAG (1 << 1) /* Write Enable Latch */
#define BTFLASH_STATUS_BP0_FLAG (1 << 2) /* Block Protect */
#define BTFLASH_STATUS_BP1_FLAG (1 << 3) /* Block Protect */
#define BTFLASH_STATUS_BP2_FLAG (1 << 4) /* Block Protect */
#define BTFLASH_STATUS_BP3_FLAG (1 << 5) /* Block Protect */
#define BTFLASH_STATUS_CP_FLAG (1 << 6) /* Continuously Program Mode */
#define BTFLASH_STATUS_SRWD_FLAG (1 << 7) /* Status Register Write Protect */

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
#define BTFLASH_STATE_WRSR 12

#endif /* #ifndef MX25LEMU_BTFLASH_DEFS_H */

/* end of file */
