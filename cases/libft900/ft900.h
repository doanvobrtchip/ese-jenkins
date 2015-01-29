# ifndef FT900_H
# define FT900_H

#define FT900_IO_OFFSET	0x10000

#define V3_IO_OFFSET	0x10000


#define GPIO_BASE		V3_IO_OFFSET + 0x0
#define gpio_conf		((uint16_t volatile *) GPIO_BASE + 0xbc/2)
#define gpio_data		((uint16_t volatile *) GPIO_BASE + 0xb8/2)
#define eth_led_conf0   	((uint16_t volatile *) GPIO_BASE + 0x60/2)
#define eth_led_conf1   	((uint16_t volatile *) GPIO_BASE + 0x62/2)
#define eth_led_data 		((uint16_t volatile *) GPIO_BASE + 0x84/2)

// FT900 system registers
#define SYS_REG_BASE		V3_IO_OFFSET + 0x00
#define sys_reg_base		((uint8_t volatile *) SYS_REG_BASE + 0x00)
#define sys_regchipid		((uint32_t volatile *) SYS_REG_BASE + 0x00)
#define sys_regefcfg		((uint32_t volatile *) SYS_REG_BASE + 0x01)
#define sys_regclkcfg		((uint32_t volatile *) SYS_REG_BASE + 0x02)
#define sys_regpmcfg		((uint32_t volatile *) SYS_REG_BASE + 0x03)
#define sys_regtstnset		((uint32_t volatile *) SYS_REG_BASE + 0x04)
#define sys_regtstnsetr		((uint32_t volatile *) SYS_REG_BASE + 0x05)
#define sys_regmsc0cfg		((uint32_t volatile *) SYS_REG_BASE + 0x06)
#define sys_regpad0003		((uint32_t volatile *) SYS_REG_BASE + 0x07)
#define sys_regpad0407		((uint32_t volatile *) SYS_REG_BASE + 0x08)

#define sys_regchipid_wl	((uint16_t volatile *) SYS_REG_BASE + 0x00)
#define sys_regchipid_wh	((uint16_t volatile *) SYS_REG_BASE + 0x01)
#define sys_regefcfg_wl		((uint16_t volatile *) SYS_REG_BASE + 0x02)
#define sys_regefcfg_wh		((uint16_t volatile *) SYS_REG_BASE + 0x03)
#define sys_regclkcfg_wl	((uint16_t volatile *) SYS_REG_BASE + 0x04)
#define sys_regclkcfg_wh	((uint16_t volatile *) SYS_REG_BASE + 0x05)

#define sys_regchipid_b0	((uint8_t volatile *) SYS_REG_BASE + 0x00)
#define sys_regchipid_b1	((uint8_t volatile *) SYS_REG_BASE + 0x01)
#define sys_regchipid_b2	((uint8_t volatile *) SYS_REG_BASE + 0x02)
#define sys_regchipid_b3	((uint8_t volatile *) SYS_REG_BASE + 0x03)
#define sys_regefcfg_b0		((uint8_t volatile *) SYS_REG_BASE + 0x04)
#define sys_regefcfg_b1		((uint8_t volatile *) SYS_REG_BASE + 0x05)
#define sys_regefcfg_b2		((uint8_t volatile *) SYS_REG_BASE + 0x06)
#define sys_regefcfg_b3		((uint8_t volatile *) SYS_REG_BASE + 0x07)
#define sys_regclkcfg_b0	((uint8_t volatile *) SYS_REG_BASE + 0x08)
#define sys_regclkcfg_b1	((uint8_t volatile *) SYS_REG_BASE + 0x09)
#define sys_regclkcfg_b2	((uint8_t volatile *) SYS_REG_BASE + 0x0a)
#define sys_regclkcfg_b3	((uint8_t volatile *) SYS_REG_BASE + 0x0b)
#define sys_regpmcfg_b0		((uint8_t volatile *) SYS_REG_BASE + 0x0c)
#define sys_regpmcfg_b1		((uint8_t volatile *) SYS_REG_BASE + 0x0d)
#define sys_regpmcfg_b2		((uint8_t volatile *) SYS_REG_BASE + 0x0e)
#define sys_regpmcfg_b3		((uint8_t volatile *) SYS_REG_BASE + 0x0f)
#define sys_regptstnset_b0	((uint8_t volatile *) SYS_REG_BASE + 0x10)
#define sys_regptstnset_b1	((uint8_t volatile *) SYS_REG_BASE + 0x11)
#define sys_regptstnset_b2	((uint8_t volatile *) SYS_REG_BASE + 0x12)
#define sys_regptstnset_b3	((uint8_t volatile *) SYS_REG_BASE + 0x13)
#define sys_regptstnsetr_b0	((uint8_t volatile *) SYS_REG_BASE + 0x14)
#define sys_regptstnsetr_b1	((uint8_t volatile *) SYS_REG_BASE + 0x15)
#define sys_regptstnsetr_b2	((uint8_t volatile *) SYS_REG_BASE + 0x16)
#define sys_regptstnsetr_b3	((uint8_t volatile *) SYS_REG_BASE + 0x17)
#define sys_regmsc0cfg_b0	((uint8_t volatile *) SYS_REG_BASE + 0x18)	// BCD function
#define sys_regmsc0cfg_b1	((uint8_t volatile *) SYS_REG_BASE + 0x19)	// DMAC, USB Device, USB Host function
#define sys_regmsc0cfg_b2	((uint8_t volatile *) SYS_REG_BASE + 0x1a)	// UART2 and UART1 function
#define sys_regmsc0cfg_b3	((uint8_t volatile *) SYS_REG_BASE + 0x1b)	// PWM, I2C, Peripheral soft reset
#define sys_regpad00		((uint8_t volatile *) SYS_REG_BASE + 0x1c)
#define sys_regpad01		((uint8_t volatile *) SYS_REG_BASE + 0x1d)
#define sys_regpad02		((uint8_t volatile *) SYS_REG_BASE + 0x1e)
#define sys_regpad03		((uint8_t volatile *) SYS_REG_BASE + 0x1f)
#define sys_regpad04		((uint8_t volatile *) SYS_REG_BASE + 0x20)
#define sys_regpad05		((uint8_t volatile *) SYS_REG_BASE + 0x21)
#define sys_regpad06		((uint8_t volatile *) SYS_REG_BASE + 0x22)
#define sys_regpad07		((uint8_t volatile *) SYS_REG_BASE + 0x23)
#define sys_regpad08		((uint8_t volatile *) SYS_REG_BASE + 0x24)
#define sys_regpad09		((uint8_t volatile *) SYS_REG_BASE + 0x25)
#define sys_regpad10		((uint8_t volatile *) SYS_REG_BASE + 0x26)
#define sys_regpad11		((uint8_t volatile *) SYS_REG_BASE + 0x27)
#define sys_regpad12		((uint8_t volatile *) SYS_REG_BASE + 0x28)
#define sys_regpad13		((uint8_t volatile *) SYS_REG_BASE + 0x29)
#define sys_regpad14		((uint8_t volatile *) SYS_REG_BASE + 0x2a)
#define sys_regpad15		((uint8_t volatile *) SYS_REG_BASE + 0x2b)
#define sys_regpad16		((uint8_t volatile *) SYS_REG_BASE + 0x2c)
#define sys_regpad17		((uint8_t volatile *) SYS_REG_BASE + 0x2d)
#define sys_regpad18		((uint8_t volatile *) SYS_REG_BASE + 0x2e)
#define sys_regpad19		((uint8_t volatile *) SYS_REG_BASE + 0x2f)
#define sys_regpad20		((uint8_t volatile *) SYS_REG_BASE + 0x30)
#define sys_regpad21		((uint8_t volatile *) SYS_REG_BASE + 0x31)
#define sys_regpad22		((uint8_t volatile *) SYS_REG_BASE + 0x32)
#define sys_regpad23		((uint8_t volatile *) SYS_REG_BASE + 0x33)
#define sys_regpad24		((uint8_t volatile *) SYS_REG_BASE + 0x34)
#define sys_regpad25		((uint8_t volatile *) SYS_REG_BASE + 0x35)
#define sys_regpad26		((uint8_t volatile *) SYS_REG_BASE + 0x36)
#define sys_regpad27		((uint8_t volatile *) SYS_REG_BASE + 0x37)
#define sys_regpad28		((uint8_t volatile *) SYS_REG_BASE + 0x38)
#define sys_regpad29		((uint8_t volatile *) SYS_REG_BASE + 0x39)
#define sys_regpad30		((uint8_t volatile *) SYS_REG_BASE + 0x3a)
#define sys_regpad31		((uint8_t volatile *) SYS_REG_BASE + 0x3b)
#define sys_regpad32		((uint8_t volatile *) SYS_REG_BASE + 0x3c)
#define sys_regpad33		((uint8_t volatile *) SYS_REG_BASE + 0x3d)
#define sys_regpad34		((uint8_t volatile *) SYS_REG_BASE + 0x3e)
#define sys_regpad35		((uint8_t volatile *) SYS_REG_BASE + 0x3f)
#define sys_regpad36		((uint8_t volatile *) SYS_REG_BASE + 0x40)
#define sys_regpad37		((uint8_t volatile *) SYS_REG_BASE + 0x41)
#define sys_regpad38		((uint8_t volatile *) SYS_REG_BASE + 0x42)
#define sys_regpad39		((uint8_t volatile *) SYS_REG_BASE + 0x43)
#define sys_regpad40		((uint8_t volatile *) SYS_REG_BASE + 0x44)
#define sys_regpad41		((uint8_t volatile *) SYS_REG_BASE + 0x45)
#define sys_regpad42		((uint8_t volatile *) SYS_REG_BASE + 0x46)
#define sys_regpad43		((uint8_t volatile *) SYS_REG_BASE + 0x47)
#define sys_regpad44		((uint8_t volatile *) SYS_REG_BASE + 0x48)
#define sys_regpad45		((uint8_t volatile *) SYS_REG_BASE + 0x49)
#define sys_regpad46		((uint8_t volatile *) SYS_REG_BASE + 0x4a)
#define sys_regpad47		((uint8_t volatile *) SYS_REG_BASE + 0x4b)
#define sys_regpad48		((uint8_t volatile *) SYS_REG_BASE + 0x4c)
#define sys_regpad49		((uint8_t volatile *) SYS_REG_BASE + 0x4d)
#define sys_regpad50		((uint8_t volatile *) SYS_REG_BASE + 0x4e)
#define sys_regpad51		((uint8_t volatile *) SYS_REG_BASE + 0x4f)
#define sys_regpad52		((uint8_t volatile *) SYS_REG_BASE + 0x50)
#define sys_regpad53		((uint8_t volatile *) SYS_REG_BASE + 0x51)
#define sys_regpad54		((uint8_t volatile *) SYS_REG_BASE + 0x52)
#define sys_regpad55		((uint8_t volatile *) SYS_REG_BASE + 0x53)	// UART1 TXD (F3) [7:6]
#define sys_regpad56		((uint8_t volatile *) SYS_REG_BASE + 0x54)	// UART1 RXD (F3) [7:6]
#define sys_regpad57		((uint8_t volatile *) SYS_REG_BASE + 0x55)	// UART1 RTS (F3) [7:6]
#define sys_regpad58		((uint8_t volatile *) SYS_REG_BASE + 0x56)	// UART1 CTS (F3) [7:6]
#define sys_regpad59		((uint8_t volatile *) SYS_REG_BASE + 0x57)	// UART1 DTR (F3) [7:6]
#define sys_regpad60		((uint8_t volatile *) SYS_REG_BASE + 0x58)	// UART1 DSR (F3) [7:6]
#define sys_regpad61		((uint8_t volatile *) SYS_REG_BASE + 0x59)	// UART1 DCD (F3) [7:6]
#define sys_regpad62		((uint8_t volatile *) SYS_REG_BASE + 0x5a)	// UART1 RI (F3) [7:6]
#define sys_regpad63		((uint8_t volatile *) SYS_REG_BASE + 0x5b)
#define sys_regpad64		((uint8_t volatile *) SYS_REG_BASE + 0x5c)
#define sys_regpad65		((uint8_t volatile *) SYS_REG_BASE + 0x5d)
#define sys_regpad66		((uint8_t volatile *) SYS_REG_BASE + 0x5e)
#define sys_regpad67		((uint8_t volatile *) SYS_REG_BASE + 0x5f)
#define sys_regp0001		((uint8_t volatile *) SYS_REG_BASE + 0x60)
#define sys_regp0203		((uint8_t volatile *) SYS_REG_BASE + 0x61)
#define sys_regp0405		((uint8_t volatile *) SYS_REG_BASE + 0x62)
#define sys_regp0607		((uint8_t volatile *) SYS_REG_BASE + 0x63)
#define sys_regp0809		((uint8_t volatile *) SYS_REG_BASE + 0x64)
#define sys_regp1011		((uint8_t volatile *) SYS_REG_BASE + 0x65)
#define sys_regp1213		((uint8_t volatile *) SYS_REG_BASE + 0x66)
#define sys_regp1415		((uint8_t volatile *) SYS_REG_BASE + 0x67)
#define sys_regp1617		((uint8_t volatile *) SYS_REG_BASE + 0x68)
#define sys_regp1819		((uint8_t volatile *) SYS_REG_BASE + 0x69)
#define sys_regp2021		((uint8_t volatile *) SYS_REG_BASE + 0x6a)
#define sys_regp2223		((uint8_t volatile *) SYS_REG_BASE + 0x6b)
#define sys_regp2425		((uint8_t volatile *) SYS_REG_BASE + 0x6c)
#define sys_regp2627		((uint8_t volatile *) SYS_REG_BASE + 0x6d)
#define sys_regp2829		((uint8_t volatile *) SYS_REG_BASE + 0x6e)
#define sys_regp3031		((uint8_t volatile *) SYS_REG_BASE + 0x6f)
#define sys_regp3233		((uint8_t volatile *) SYS_REG_BASE + 0x70)
#define sys_regp3435		((uint8_t volatile *) SYS_REG_BASE + 0x71)
#define sys_regp3637		((uint8_t volatile *) SYS_REG_BASE + 0x72)
#define sys_regp3839		((uint8_t volatile *) SYS_REG_BASE + 0x73)
#define sys_regp4041		((uint8_t volatile *) SYS_REG_BASE + 0x74)
#define sys_regp4243		((uint8_t volatile *) SYS_REG_BASE + 0x75)
#define sys_regp4445		((uint8_t volatile *) SYS_REG_BASE + 0x76)
#define sys_regp4647		((uint8_t volatile *) SYS_REG_BASE + 0x77)
#define sys_regp4849		((uint8_t volatile *) SYS_REG_BASE + 0x78)
#define sys_regp5051		((uint8_t volatile *) SYS_REG_BASE + 0x79)
#define sys_regp5253		((uint8_t volatile *) SYS_REG_BASE + 0x7a)
#define sys_regp5455		((uint8_t volatile *) SYS_REG_BASE + 0x7b)
#define sys_regp5657		((uint8_t volatile *) SYS_REG_BASE + 0x7c)
#define sys_regp5859		((uint8_t volatile *) SYS_REG_BASE + 0x7d)
#define sys_regp6061		((uint8_t volatile *) SYS_REG_BASE + 0x7e)
#define sys_regp6263		((uint8_t volatile *) SYS_REG_BASE + 0x7f)
#define sys_regp6465		((uint8_t volatile *) SYS_REG_BASE + 0x80)
#define sys_regp6667		((uint8_t volatile *) SYS_REG_BASE + 0x81)
#define sys_regp6869		((uint8_t volatile *) SYS_REG_BASE + 0x82)
#define sys_regp7071		((uint8_t volatile *) SYS_REG_BASE + 0x83)
#define sys_regval0_b0		((uint8_t volatile *) SYS_REG_BASE + 0x84)
#define sys_regval0_b1		((uint8_t volatile *) SYS_REG_BASE + 0x85)
#define sys_regval0_b2		((uint8_t volatile *) SYS_REG_BASE + 0x86)
#define sys_regval0_b3		((uint8_t volatile *) SYS_REG_BASE + 0x87)
#define sys_regval1_b0		((uint8_t volatile *) SYS_REG_BASE + 0x88)
#define sys_regval1_b1		((uint8_t volatile *) SYS_REG_BASE + 0x89)
#define sys_regval1_b2		((uint8_t volatile *) SYS_REG_BASE + 0x8a)
#define sys_regval1_b3		((uint8_t volatile *) SYS_REG_BASE + 0x8b)
#define sys_regval2_b0		((uint8_t volatile *) SYS_REG_BASE + 0x8c)
#define sys_regval2_b1		((uint8_t volatile *) SYS_REG_BASE + 0x8d)
#define sys_regval2_b2		((uint8_t volatile *) SYS_REG_BASE + 0x8e)
#define sys_regval2_b3		((uint8_t volatile *) SYS_REG_BASE + 0x8f)
#define sys_regioie0_b0		((uint8_t volatile *) SYS_REG_BASE + 0x90)
#define sys_regioie0_b1		((uint8_t volatile *) SYS_REG_BASE + 0x91)
#define sys_regioie0_b2		((uint8_t volatile *) SYS_REG_BASE + 0x92)
#define sys_regioie0_b3		((uint8_t volatile *) SYS_REG_BASE + 0x93)
#define sys_regioie1_b0		((uint8_t volatile *) SYS_REG_BASE + 0x94)
#define sys_regioie1_b1		((uint8_t volatile *) SYS_REG_BASE + 0x95)
#define sys_regioie1_b2		((uint8_t volatile *) SYS_REG_BASE + 0x96)
#define sys_regioie1_b3		((uint8_t volatile *) SYS_REG_BASE + 0x97)
#define sys_regioie2_b0		((uint8_t volatile *) SYS_REG_BASE + 0x98)
#define sys_regioie2_b1		((uint8_t volatile *) SYS_REG_BASE + 0x99)
#define sys_regioie2_b2		((uint8_t volatile *) SYS_REG_BASE + 0x9a)
#define sys_regioie2_b3		((uint8_t volatile *) SYS_REG_BASE + 0x9b)
#define sys_regioopen0_b0	((uint8_t volatile *) SYS_REG_BASE + 0x9c)
#define sys_regioopen0_b1	((uint8_t volatile *) SYS_REG_BASE + 0x9d)
#define sys_regioopen0_b2	((uint8_t volatile *) SYS_REG_BASE + 0x9e)
#define sys_regioopen0_b3	((uint8_t volatile *) SYS_REG_BASE + 0x9f)
#define sys_regioopen1_b0	((uint8_t volatile *) SYS_REG_BASE + 0xa0)
#define sys_regioopen1_b1	((uint8_t volatile *) SYS_REG_BASE + 0xa1)
#define sys_regioopen1_b2	((uint8_t volatile *) SYS_REG_BASE + 0xa2)
#define sys_regioopen1_b3	((uint8_t volatile *) SYS_REG_BASE + 0xa3)
#define sys_regioopen2_b0	((uint8_t volatile *) SYS_REG_BASE + 0xa4)
#define sys_regioopen2_b1	((uint8_t volatile *) SYS_REG_BASE + 0xa5)
#define sys_regioopen2_b2	((uint8_t volatile *) SYS_REG_BASE + 0xa6)
#define sys_regioopen2_b3	((uint8_t volatile *) SYS_REG_BASE + 0xa7)
#define sys_regphymsc_b0	((uint8_t volatile *) SYS_REG_BASE + 0xa8)
#define sys_regphymsc_b1	((uint8_t volatile *) SYS_REG_BASE + 0xa9)
#define sys_regphymsc_b2	((uint8_t volatile *) SYS_REG_BASE + 0xaa)
#define sys_regphymsc_b3	((uint8_t volatile *) SYS_REG_BASE + 0xab)
#define sys_regphyid_b0		((uint8_t volatile *) SYS_REG_BASE + 0xac)
#define sys_regphyid_b1		((uint8_t volatile *) SYS_REG_BASE + 0xad)
#define sys_regphyid_b2		((uint8_t volatile *) SYS_REG_BASE + 0xae)
#define sys_regphyid_b3		((uint8_t volatile *) SYS_REG_BASE + 0xaf)

// FT900 Interrupt Controller

#define FT900_INTR_BASE		V3_IO_OFFSET + 0xC0
#define ft900_intr_asgn0to3	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x00)  // For 32 bit access
#define ft900_intr_asgn4to7	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x01)  // For 32 bit access
#define ft900_intr_asgn8to11	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x02)  // For 32 bit access
#define ft900_intr_asgn12to15	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x03)  // For 32 bit access
#define ft900_intr_asgn16to19	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x04)  // For 32 bit access
#define ft900_intr_asgn20to23	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x05)  // For 32 bit access
#define ft900_intr_asgn24to27	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x06)  // For 32 bit access
#define ft900_intr_asgn28to31	((uint32_t volatile *) FT900_INTR_BASE/4 + 0x07)  // For 32 bit access
#define ft900_intr_ctrl		((uint32_t volatile *) FT900_INTR_BASE/4 + 0x08)  // For 32 bit access
#define ft900_intr_asgn0	((uint8_t volatile *) FT900_INTR_BASE + 0x00)  // For 8 bit access
#define ft900_intr_asgn1	((uint8_t volatile *) FT900_INTR_BASE + 0x01)  // For 8 bit access
#define ft900_intr_asgn2	((uint8_t volatile *) FT900_INTR_BASE + 0x02)  // For 8 bit access
#define ft900_intr_asgn3	((uint8_t volatile *) FT900_INTR_BASE + 0x03)  // For 8 bit access
#define ft900_intr_asgn4	((uint8_t volatile *) FT900_INTR_BASE + 0x04)  // For 8 bit access
#define ft900_intr_asgn5	((uint8_t volatile *) FT900_INTR_BASE + 0x05)  // For 8 bit access
#define ft900_intr_asgn6	((uint8_t volatile *) FT900_INTR_BASE + 0x06)  // For 8 bit access
#define ft900_intr_asgn7	((uint8_t volatile *) FT900_INTR_BASE + 0x07)  // For 8 bit access
#define ft900_intr_asgn8	((uint8_t volatile *) FT900_INTR_BASE + 0x08)  // For 8 bit access
#define ft900_intr_asgn9	((uint8_t volatile *) FT900_INTR_BASE + 0x09)  // For 8 bit access
#define ft900_intr_asgn10	((uint8_t volatile *) FT900_INTR_BASE + 0x0a)  // For 8 bit access
#define ft900_intr_asgn11	((uint8_t volatile *) FT900_INTR_BASE + 0x0b)  // For 8 bit access
#define ft900_intr_asgn12	((uint8_t volatile *) FT900_INTR_BASE + 0x0c)  // For 8 bit access
#define ft900_intr_asgn13	((uint8_t volatile *) FT900_INTR_BASE + 0x0d)  // For 8 bit access
#define ft900_intr_asgn14	((uint8_t volatile *) FT900_INTR_BASE + 0x0e)  // For 8 bit access
#define ft900_intr_asgn15	((uint8_t volatile *) FT900_INTR_BASE + 0x0f)  // For 8 bit access
#define ft900_intr_asgn16	((uint8_t volatile *) FT900_INTR_BASE + 0x10)  // For 8 bit access
#define ft900_intr_asgn17	((uint8_t volatile *) FT900_INTR_BASE + 0x11)  // For 8 bit access
#define ft900_intr_asgn18	((uint8_t volatile *) FT900_INTR_BASE + 0x12)  // For 8 bit access
#define ft900_intr_asgn19	((uint8_t volatile *) FT900_INTR_BASE + 0x13)  // For 8 bit access
#define ft900_intr_asgn20	((uint8_t volatile *) FT900_INTR_BASE + 0x14)  // For 8 bit access
#define ft900_intr_asgn21	((uint8_t volatile *) FT900_INTR_BASE + 0x15)  // For 8 bit access
#define ft900_intr_asgn22	((uint8_t volatile *) FT900_INTR_BASE + 0x16)  // For 8 bit access
#define ft900_intr_asgn23	((uint8_t volatile *) FT900_INTR_BASE + 0x17)  // For 8 bit access
#define ft900_intr_asgn24	((uint8_t volatile *) FT900_INTR_BASE + 0x18)  // For 8 bit access
#define ft900_intr_asgn25	((uint8_t volatile *) FT900_INTR_BASE + 0x19)  // For 8 bit access
#define ft900_intr_asgn26	((uint8_t volatile *) FT900_INTR_BASE + 0x1a)  // For 8 bit access
#define ft900_intr_asgn27	((uint8_t volatile *) FT900_INTR_BASE + 0x1b)  // For 8 bit access
#define ft900_intr_asgn28	((uint8_t volatile *) FT900_INTR_BASE + 0x1c)  // For 8 bit access
#define ft900_intr_asgn29	((uint8_t volatile *) FT900_INTR_BASE + 0x1d)  // For 8 bit access
#define ft900_intr_asgn30	((uint8_t volatile *) FT900_INTR_BASE + 0x1e)  // For 8 bit access
#define ft900_intr_asgn31	((uint8_t volatile *) FT900_INTR_BASE + 0x1f)  // For 8 bit access
#define ft900_intr_ctrl_0	((uint8_t volatile *) FT900_INTR_BASE + 0x20)  // For 8 bit access
#define ft900_intr_ctrl_1	((uint8_t volatile *) FT900_INTR_BASE + 0x21)  // For 8 bit access
#define ft900_intr_ctrl_2	((uint8_t volatile *) FT900_INTR_BASE + 0x22)  // For 8 bit access
#define ft900_intr_ctrl_3	((uint8_t volatile *) FT900_INTR_BASE + 0x23)  // For 8 bit access


//EHCI registers
#define EHCI_BASE	V3_IO_OFFSET + 0x100
#define ehci_hccaplength	((uint32_t volatile *) EHCI_BASE/4 + 0x00/4)  // For 32 bit access
#define ehci_hcsparams		((uint32_t volatile *) EHCI_BASE/4 + 0x04/4)  // For 32 bit access
#define ehci_hccparams		((uint32_t volatile *) EHCI_BASE/4 + 0x08/4)  // For 32 bit access
#define ehci_usbcmd		((uint32_t volatile *) EHCI_BASE/4 + 0x10/4)  // For 32 bit access
#define ehci_usbsts		((uint32_t volatile *) EHCI_BASE/4 + 0x14/4)  // For 32 bit access
#define ehci_usbintr		((uint32_t volatile *) EHCI_BASE/4 + 0x18/4)  // For 32 bit access
#define ehci_frindex		((uint32_t volatile *) EHCI_BASE/4 + 0x1C/4)  // For 32 bit access
#define ehci_periodiclistaddr	((uint32_t volatile *) EHCI_BASE/4 + 0x24/4)  // For 32 bit access
#define ehci_asynclistaddr	((uint32_t volatile *) EHCI_BASE/4 + 0x28/4)  // For 32 bit access
#define ehci_portsc		((uint32_t volatile *) EHCI_BASE/4 + 0x30/4)  // For 32 bit access
#define ehci_eoftime		((uint32_t volatile *) EHCI_BASE/4 + 0x34/4)  // For 32 bit access

#define ehci_busctrl		((uint32_t volatile *) EHCI_BASE/4 + 0x40/4)  // For 32 bit access
#define ehci_busintstatus	((uint32_t volatile *) EHCI_BASE/4 + 0x44/4)  // For 32 bit access
#define ehci_businten		((uint32_t volatile *) EHCI_BASE/4 + 0x48/4)  // For 32 bit access
#define ehci_testregister	((uint32_t volatile *) EHCI_BASE/4 + 0x50/4)  // For 32 bit access
#define ehci_dmactrlpmset1	((uint32_t volatile *) EHCI_BASE/4 + 0x70/4)  // For 32 bit access
#define ehci_dmactrlpmset2	((uint32_t volatile *) EHCI_BASE/4 + 0x74/4)  // For 32 bit access


#define EHCI_RAM_BASE	V3_IO_OFFSET + 0x1000
#define ehci_rambase		((uint32_t volatile *) EHCI_RAM_BASE/4 + 0x00)  // For 32 bit access


// USBD registers
#define DUSB_BASE	V3_IO_OFFSET + 0x180
#define  usbReg_CMIF              ((uint8_t volatile *) DUSB_BASE + 0x00)
#define  usbReg_EPIFL             ((uint8_t volatile *) DUSB_BASE + 0x04)
#define  usbReg_EPIFH             ((uint8_t volatile *) DUSB_BASE + 0x05)
#define  usbReg_CMIE              ((uint8_t volatile *) DUSB_BASE + 0x08)
#define  usbReg_EPIEL             ((uint8_t volatile *) DUSB_BASE + 0x0c)
#define  usbReg_EPIEH             ((uint8_t volatile *) DUSB_BASE + 0x0d)
#define  usbReg_FCTRL             ((uint8_t volatile *) DUSB_BASE + 0x10)
#define  usbReg_FRAMEL            ((uint8_t volatile *) DUSB_BASE + 0x14)
#define  usbReg_FRAMEH            ((uint8_t volatile *) DUSB_BASE + 0x15)
#define  usbReg_FADDR             ((uint8_t volatile *) DUSB_BASE + 0x18)
#define  usbReg_EP0CR             ((uint8_t volatile *) DUSB_BASE + 0x1c)
#define  usbReg_EP0SR             ((uint8_t volatile *) DUSB_BASE + 0x20)
#define  usbReg_EP0CNT            ((uint8_t volatile *) DUSB_BASE + 0x24)
#define  usbReg_EP0FIFO           ((uint8_t volatile *) DUSB_BASE + 0x28)
#define  usbReg_EP1CR             ((uint8_t volatile *) DUSB_BASE + 0x2c)
#define  usbReg_EP1SR             ((uint8_t volatile *) DUSB_BASE + 0x30)
#define  usbReg_EP1CNTH           ((uint8_t volatile *) DUSB_BASE + 0x34)
#define  usbReg_EP1CNTL           ((uint8_t volatile *) DUSB_BASE + 0x35)
#define  usbReg_EP1FIFO           ((uint8_t volatile *) DUSB_BASE + 0x38)
#define  usbReg_EP2CR             ((uint8_t volatile *) DUSB_BASE + 0x3c)
#define  usbReg_EP2SR             ((uint8_t volatile *) DUSB_BASE + 0x40) 
#define  usbReg_EP2CNTH           ((uint8_t volatile *) DUSB_BASE + 0x44)
#define  usbReg_EP2CNTL           ((uint8_t volatile *) DUSB_BASE + 0x45)
#define  usbReg_EP2FIFO           ((uint8_t volatile *) DUSB_BASE + 0x48)
#define  usbReg_EP3CR             ((uint8_t volatile *) DUSB_BASE + 0x4c)
#define  usbReg_EP3SR             ((uint8_t volatile *) DUSB_BASE + 0x50)
#define  usbReg_EP3CNTH           ((uint8_t volatile *) DUSB_BASE + 0x54) 
#define  usbReg_EP3CNTL           ((uint8_t volatile *) DUSB_BASE + 0x55) 
#define  usbReg_EP3FIFO           ((uint8_t volatile *) DUSB_BASE + 0x58) 
#define  usbReg_EP4CR             ((uint8_t volatile *) DUSB_BASE + 0x5c)
#define  usbReg_EP4SR             ((uint8_t volatile *) DUSB_BASE + 0x60)
#define  usbReg_EP4CNTH           ((uint8_t volatile *) DUSB_BASE + 0x64)
#define  usbReg_EP4CNTL           ((uint8_t volatile *) DUSB_BASE + 0x65)
#define  usbReg_EP4FIFO           ((uint8_t volatile *) DUSB_BASE + 0x68)
#define  usbReg_EP5CR             ((uint8_t volatile *) DUSB_BASE + 0x6c) 
#define  usbReg_EP5SR             ((uint8_t volatile *) DUSB_BASE + 0x70)
#define  usbReg_EP5CNTH           ((uint8_t volatile *) DUSB_BASE + 0x74)
#define  usbReg_EP5CNTL           ((uint8_t volatile *) DUSB_BASE + 0x75)
#define  usbReg_EP5FIFO           ((uint8_t volatile *) DUSB_BASE + 0x78) 
#define  usbReg_EP6CR             ((uint8_t volatile *) DUSB_BASE + 0x7c)
#define  usbReg_EP6SR             ((uint8_t volatile *) DUSB_BASE + 0x80)
#define  usbReg_EP6CNTH           ((uint8_t volatile *) DUSB_BASE + 0x84)
#define  usbReg_EP6CNTL           ((uint8_t volatile *) DUSB_BASE + 0x85)
#define  usbReg_EP6FIFO           ((uint8_t volatile *) DUSB_BASE + 0x88)
#define  usbReg_EP7CR             ((uint8_t volatile *) DUSB_BASE + 0x8c)
#define  usbReg_EP7SR             ((uint8_t volatile *) DUSB_BASE + 0x90)
#define  usbReg_EP7CNTH           ((uint8_t volatile *) DUSB_BASE + 0x94) 
#define  usbReg_EP7CNTL           ((uint8_t volatile *) DUSB_BASE + 0x95)
#define  usbReg_EP7FIFO           ((uint8_t volatile *) DUSB_BASE + 0x98)

// DMAC (Ethernet) registers

#define DMAC_BASE	V3_IO_OFFSET + 0x220
#define dmac_isr_iack	((uint8_t volatile *) DMAC_BASE + 0x00)
#define dmac_imr	((uint8_t volatile *) DMAC_BASE + 0x01)
#define dmac_rcr	((uint8_t volatile *) DMAC_BASE + 0x02)
#define dmac_tcr	((uint8_t volatile *) DMAC_BASE + 0x03)
#define dmac_dr		((uint32_t volatile *) DMAC_BASE/4 + 0x04/4)  // For 32 bit access
#define dmac_dr_w	((uint16_t volatile *) DMAC_BASE/2 + 0x04/2)  // For 16 bit access
#define dmac_dr0	((uint8_t volatile *) DMAC_BASE + 0x04)
#define dmac_dr1	((uint8_t volatile *) DMAC_BASE + 0x05)
#define dmac_dr2	((uint8_t volatile *) DMAC_BASE + 0x06)
#define dmac_dr3	((uint8_t volatile *) DMAC_BASE + 0x07)
#define dmac_iar	((uint32_t volatile *) DMAC_BASE/4 + 0x08/4)  // For 32 bit access
#define dmac_iar0	((uint8_t volatile *) DMAC_BASE + 0x08)
#define dmac_iar1	((uint8_t volatile *) DMAC_BASE + 0x09)
#define dmac_iar2	((uint8_t volatile *) DMAC_BASE + 0x0a)
#define dmac_iar3	((uint8_t volatile *) DMAC_BASE + 0x0b)
#define dmac_iar4	((uint8_t volatile *) DMAC_BASE + 0x0c)
#define dmac_iar5	((uint8_t volatile *) DMAC_BASE + 0x0d)
#define dmac_thr	((uint8_t volatile *) DMAC_BASE + 0x0e)
#define dmac_mcr	((uint8_t volatile *) DMAC_BASE + 0x0f)
#define dmac_mdvr	((uint8_t volatile *) DMAC_BASE + 0x10)
#define dmac_mar	((uint8_t volatile *) DMAC_BASE + 0x11)
#define dmac_mdtx	((uint16_t volatile *) DMAC_BASE/2 + 0x12/2)  // For 16 bit access
#define dmac_mdtx0	((uint8_t volatile *) DMAC_BASE + 0x12)
#define dmac_mdtx1	((uint8_t volatile *) DMAC_BASE + 0x13)
#define dmac_mdrx	((uint16_t volatile *) DMAC_BASE/2 + 0x14/2)  // For 16 bit access
#define dmac_mdrx0	((uint8_t volatile *) DMAC_BASE + 0x14)
#define dmac_mdrx1	((uint8_t volatile *) DMAC_BASE + 0x15)
#define dmac_npr	((uint8_t volatile *) DMAC_BASE + 0x16)
#define dmac_trr	((uint8_t volatile *) DMAC_BASE + 0x17)

// CAN1 registers

#define CAN1_BASE	V3_IO_OFFSET + 0x240
#define can1_mr		((uint8_t volatile *) CAN1_BASE + 0x00)
#define can1_cmr	((uint8_t volatile *) CAN1_BASE + 0x01)
#define can1_sr		((uint8_t volatile *) CAN1_BASE + 0x02)
#define can1_isr_iack	((uint8_t volatile *) CAN1_BASE + 0x03)
#define can1_imr	((uint8_t volatile *) CAN1_BASE + 0x04)
#define can1_rmc	((uint8_t volatile *) CAN1_BASE + 0x05)
#define can1_btr0	((uint8_t volatile *) CAN1_BASE + 0x06)
#define can1_btr1	((uint8_t volatile *) CAN1_BASE + 0x07)
#define can1_txbuf	((uint32_t volatile *) CAN1_BASE/4 + 0x08/4)  // For 32 bit access
#define can1_rxbuf	((uint32_t volatile *) CAN1_BASE/4 + 0x0c/4)  // For 32 bit access
#define can1_acr0	((uint8_t volatile *) CAN1_BASE + 0x10)
#define can1_acr1	((uint8_t volatile *) CAN1_BASE + 0x11)
#define can1_acr2	((uint8_t volatile *) CAN1_BASE + 0x12)
#define can1_acr3	((uint8_t volatile *) CAN1_BASE + 0x13)
#define can1_amr0	((uint8_t volatile *) CAN1_BASE + 0x14)
#define can1_amr1	((uint8_t volatile *) CAN1_BASE + 0x15)
#define can1_amr2	((uint8_t volatile *) CAN1_BASE + 0x16)
#define can1_amr3	((uint8_t volatile *) CAN1_BASE + 0x17)
#define can1_ecc	((uint8_t volatile *) CAN1_BASE + 0x18)
#define can1_txerr	((uint8_t volatile *) CAN1_BASE + 0x19)
#define can1_rxerr	((uint8_t volatile *) CAN1_BASE + 0x1a)
#define can1_alc	((uint8_t volatile *) CAN1_BASE + 0x1b)

// CAN2 registers

#define CAN2_BASE	V3_IO_OFFSET + 0x260
#define can2_mr		((uint8_t volatile *) CAN2_BASE + 0x00)
#define can2_cmr	((uint8_t volatile *) CAN2_BASE + 0x01)
#define can2_sr		((uint8_t volatile *) CAN2_BASE + 0x02)
#define can2_isr_iack	((uint8_t volatile *) CAN2_BASE + 0x03)
#define can2_imr	((uint8_t volatile *) CAN2_BASE + 0x04)
#define can2_rmc	((uint8_t volatile *) CAN2_BASE + 0x05)
#define can2_btr0	((uint8_t volatile *) CAN2_BASE + 0x06)
#define can2_btr1	((uint8_t volatile *) CAN2_BASE + 0x07)
#define can2_txbuf	((uint32_t volatile *) CAN2_BASE/4 + 0x08/4)  // For 32 bit access
#define can2_rxbuf	((uint32_t volatile *) CAN2_BASE/4 + 0x0c/4)  // For 32 bit access
#define can2_acr0	((uint8_t volatile *) CAN2_BASE + 0x10)
#define can2_acr1	((uint8_t volatile *) CAN2_BASE + 0x11)
#define can2_acr2	((uint8_t volatile *) CAN2_BASE + 0x12)
#define can2_acr3	((uint8_t volatile *) CAN2_BASE + 0x13)
#define can2_amr0	((uint8_t volatile *) CAN2_BASE + 0x14)
#define can2_amr1	((uint8_t volatile *) CAN2_BASE + 0x15)
#define can2_amr2	((uint8_t volatile *) CAN2_BASE + 0x16)
#define can2_amr3	((uint8_t volatile *) CAN2_BASE + 0x17)
#define can2_ecc	((uint8_t volatile *) CAN2_BASE + 0x18)
#define can2_txerr	((uint8_t volatile *) CAN2_BASE + 0x19)
#define can2_rxerr	((uint8_t volatile *) CAN2_BASE + 0x1a)
#define can2_alc	((uint8_t volatile *) CAN2_BASE + 0x1b)

// RTC registers

#define RTC_BASE	V3_IO_OFFSET + 0x280
#define rtc_ccvr	((uint32_t volatile *) RTC_BASE/4 + 0x00/4)
#define rtc_cmr		((uint32_t volatile *) RTC_BASE/4 + 0x04/4)
#define rtc_clr		((uint32_t volatile *) RTC_BASE/4 + 0x08/4)
#define rtc_ccr		((uint32_t volatile *) RTC_BASE/4 + 0x0c/4)
#define rtc_stat	((uint32_t volatile *) RTC_BASE/4 + 0x10/4)
#define rtc_rstat	((uint32_t volatile *) RTC_BASE/4 + 0x14/4)
#define rtc_eoi		((uint32_t volatile *) RTC_BASE/4 + 0x18/4)
#define rtc_comp_ver	((uint32_t volatile *) RTC_BASE/4 + 0x1c/4)

// SPIM registers
#define SPIM_BASE 	V3_IO_OFFSET + 0x2A0
#define spim_spcr	((uint32_t volatile *) SPIM_BASE/4 + 0x00/4)
#define spim_spsr	((uint32_t volatile *) SPIM_BASE/4 + 0x04/4)
#define spim_spdr	((uint32_t volatile *) SPIM_BASE/4 + 0x08/4)
#define spim_sscr	((uint32_t volatile *) SPIM_BASE/4 + 0x0C/4)
#define spim_sfcr	((uint32_t volatile *) SPIM_BASE/4 + 0x10/4)
#define spim_stfcr	((uint32_t volatile *) SPIM_BASE/4 + 0x14/4)
#define spim_spdr_bits 	((uint32_t volatile *) SPIM_BASE/4 + 0x18/4)

// SPI 1 slave registers
#define SPIS1_BASE 	V3_IO_OFFSET + 0x2C0
#define spis1_spcr	((uint32_t volatile *) SPIS1_BASE/4 + 0x00/4)
#define spis1_spsr	((uint32_t volatile *) SPIS1_BASE/4 + 0x04/4)
#define spis1_spdr	((uint32_t volatile *) SPIS1_BASE/4 + 0x08/4)
#define spis1_sscr	((uint32_t volatile *) SPIS1_BASE/4 + 0x0C/4)
#define spis1_sfcr	((uint32_t volatile *) SPIS1_BASE/4 + 0x10/4)
#define spis1_stfcr	((uint32_t volatile *) SPIS1_BASE/4 + 0x14/4)
#define spis1_spdr_bits 	((uint32_t volatile *) SPIS1_BASE/4 + 0x18/4)


// SPI 2 slave registers
#define SPIS2_BASE 	V3_IO_OFFSET + 0x2E0
#define spis2_spcr	((uint32_t volatile *) SPIS2_BASE/4 + 0x00/4)
#define spis2_spsr	((uint32_t volatile *) SPIS2_BASE/4 + 0x04/4)
#define spis2_spdr	((uint32_t volatile *) SPIS2_BASE/4 + 0x08/4)
#define spis2_sscr	((uint32_t volatile *) SPIS2_BASE/4 + 0x0C/4)
#define spis2_sfcr	((uint32_t volatile *) SPIS2_BASE/4 + 0x10/4)
#define spis2_stfcr	((uint32_t volatile *) SPIS2_BASE/4 + 0x14/4)
#define spis2_spdr_bits 	((uint32_t volatile *) SPIS2_BASE/4 + 0x18/4)

// I2C master registers
#define I2CM_BASE	V3_IO_OFFSET + 0x300
#define i2cm_msa	((uint8_t volatile *) I2CM_BASE + 0x00)
#define i2cm_mcr	((uint8_t volatile *) I2CM_BASE + 0x01)
#define i2cm_msr	((uint8_t volatile *) I2CM_BASE + 0x01)
#define i2cm_buf	((uint8_t volatile *) I2CM_BASE + 0x02)
#define i2cm_mtp	((uint8_t volatile *) I2CM_BASE + 0x03)
#define i2cm_bl_id	((uint8_t volatile *) I2CM_BASE + 0x04)
#define i2cm_ie_id	((uint8_t volatile *) I2CM_BASE + 0x05)
#define i2cm_pend_id	((uint8_t volatile *) I2CM_BASE + 0x06)
#define i2cm_fifo_id	((uint8_t volatile *) I2CM_BASE + 0x07)
#define i2cm_string_id	((uint8_t volatile *) I2CM_BASE + 0x08)

// I2C slave registers
#define I2CS_BASE	V3_IO_OFFSET + 0x310
#define i2cs_soa	((uint8_t volatile *) I2CS_BASE + 0x00)
#define i2cs_scr	((uint8_t volatile *) I2CS_BASE + 0x01)
#define i2cs_ssr	((uint8_t volatile *) I2CS_BASE + 0x01)
#define i2cs_sbuf	((uint8_t volatile *) I2CS_BASE + 0x02)
#define i2cs_stat_id	((uint8_t volatile *) I2CS_BASE + 0x04)
#define i2cs_ie_id	((uint8_t volatile *) I2CS_BASE + 0x05)
#define i2cs_pend_id	((uint8_t volatile *) I2CS_BASE + 0x06)
#define i2cs_fifo_id	((uint8_t volatile *) I2CS_BASE + 0x07)

// UART1 registers

#define UART1_BASE	V3_IO_OFFSET + 0x320
// Standard  550 compatible registers
#define uart1_rhr	((uint8_t volatile *) UART1_BASE + 0x00)
#define uart1_thr	((uint8_t volatile *) UART1_BASE + 0x00)
#define uart1_ier	((uint8_t volatile *) UART1_BASE + 0x01)
#define uart1_isr_reg	((uint8_t volatile *) UART1_BASE + 0x02)
#define uart1_fcr	((uint8_t volatile *) UART1_BASE + 0x02)
#define uart1_lcr	((uint8_t volatile *) UART1_BASE + 0x03)
#define uart1_mcr	((uint8_t volatile *) UART1_BASE + 0x04)
#define uart1_lsr	((uint8_t volatile *) UART1_BASE + 0x05)
#define uart1_msr	((uint8_t volatile *) UART1_BASE + 0x06)
#define uart1_spr	((uint8_t volatile *) UART1_BASE + 0x07)
#define uart1_dll	((uint8_t volatile *) UART1_BASE + 0x00)
#define uart1_dlm	((uint8_t volatile *) UART1_BASE + 0x01)
// 650 compatible registers
#define uart1_efr	((uint8_t volatile *) UART1_BASE + 0x02)
#define uart1_xon1	((uint8_t volatile *) UART1_BASE + 0x04)
#define uart1_xon2	((uint8_t volatile *) UART1_BASE + 0x05)
#define uart1_xoff1	((uint8_t volatile *) UART1_BASE + 0x06)
#define uart1_xoff2	((uint8_t volatile *) UART1_BASE + 0x07)
// 950 compatible registers
#define uart1_asr	((uint8_t volatile *) UART1_BASE + 0x01)
#define uart1_rfl	((uint8_t volatile *) UART1_BASE + 0x03)
#define uart1_tfl	((uint8_t volatile *) UART1_BASE + 0x04)
#define uart1_icr	((uint8_t volatile *) UART1_BASE + 0x05)
// Indexed control registers
#define uart1_acr	((uint8_t volatile *) UART1_BASE + 0x00)
#define uart1_cpr	((uint8_t volatile *) UART1_BASE + 0x01)
#define uart1_tcr	((uint8_t volatile *) UART1_BASE + 0x02)
#define uart1_cks	((uint8_t volatile *) UART1_BASE + 0x03)
#define uart1_ttl	((uint8_t volatile *) UART1_BASE + 0x04)
#define uart1_rtl	((uint8_t volatile *) UART1_BASE + 0x05)
#define uart1_fcl	((uint8_t volatile *) UART1_BASE + 0x06)
#define uart1_fch	((uint8_t volatile *) UART1_BASE + 0x07)
#define uart1_id1	((uint8_t volatile *) UART1_BASE + 0x08)
#define uart1_id2	((uint8_t volatile *) UART1_BASE + 0x09)
#define uart1_id3	((uint8_t volatile *) UART1_BASE + 0x0a)
#define uart1_rev	((uint8_t volatile *) UART1_BASE + 0x0b)
#define uart1_csr	((uint8_t volatile *) UART1_BASE + 0x0c)
#define uart1_nmr	((uint8_t volatile *) UART1_BASE + 0x0d)
#define uart1_mdm	((uint8_t volatile *) UART1_BASE + 0x0e)
#define uart1_rfc	((uint8_t volatile *) UART1_BASE + 0x0f)
#define uart1_gds	((uint8_t volatile *) UART1_BASE + 0x10)
#define uart1_dms	((uint8_t volatile *) UART1_BASE + 0x11)
#define uart1_pidx	((uint8_t volatile *) UART1_BASE + 0x12)
#define uart1_cka	((uint8_t volatile *) UART1_BASE + 0x13)

#define UART2_BASE	V3_IO_OFFSET + 0x330
// Standard  550 compatible registers
#define uart2_rhr	((uint8_t volatile *) UART2_BASE + 0x00)
#define uart2_thr	((uint8_t volatile *) UART2_BASE + 0x00)
#define uart2_ier	((uint8_t volatile *) UART2_BASE + 0x01)
#define uart2_isr_reg	((uint8_t volatile *) UART2_BASE + 0x02)
#define uart2_fcr	((uint8_t volatile *) UART2_BASE + 0x02)
#define uart2_lcr	((uint8_t volatile *) UART2_BASE + 0x03)
#define uart2_mcr	((uint8_t volatile *) UART2_BASE + 0x04)
#define uart2_lsr	((uint8_t volatile *) UART2_BASE + 0x05)
#define uart2_msr	((uint8_t volatile *) UART2_BASE + 0x06)
#define uart2_spr	((uint8_t volatile *) UART2_BASE + 0x07)
#define uart2_dll	((uint8_t volatile *) UART2_BASE + 0x00)
#define uart2_dlm	((uint8_t volatile *) UART2_BASE + 0x01)
// 650 compatible registers
#define uart2_efr	((uint8_t volatile *) UART2_BASE + 0x02)
#define uart2_xon1	((uint8_t volatile *) UART2_BASE + 0x04)
#define uart2_xon2	((uint8_t volatile *) UART2_BASE + 0x05)
#define uart2_xoff1	((uint8_t volatile *) UART2_BASE + 0x06)
#define uart2_xoff2	((uint8_t volatile *) UART2_BASE + 0x07)
// 950 compatible registers
#define uart2_asr	((uint8_t volatile *) UART2_BASE + 0x01)
#define uart2_rfl	((uint8_t volatile *) UART2_BASE + 0x03)
#define uart2_tfl	((uint8_t volatile *) UART2_BASE + 0x04)
#define uart2_icr	((uint8_t volatile *) UART2_BASE + 0x05)
// Indexed control registers
#define uart2_acr	((uint8_t volatile *) UART2_BASE + 0x00)
#define uart2_cpr	((uint8_t volatile *) UART2_BASE + 0x01)
#define uart2_tcr	((uint8_t volatile *) UART2_BASE + 0x02)
#define uart2_cks	((uint8_t volatile *) UART2_BASE + 0x03)
#define uart2_ttl	((uint8_t volatile *) UART2_BASE + 0x04)
#define uart2_rtl	((uint8_t volatile *) UART2_BASE + 0x05)
#define uart2_fcl	((uint8_t volatile *) UART2_BASE + 0x06)
#define uart2_fch	((uint8_t volatile *) UART2_BASE + 0x07)
#define uart2_id1	((uint8_t volatile *) UART2_BASE + 0x08)
#define uart2_id2	((uint8_t volatile *) UART2_BASE + 0x09)
#define uart2_id3	((uint8_t volatile *) UART2_BASE + 0x0a)
#define uart2_rev	((uint8_t volatile *) UART2_BASE + 0x0b)
#define uart2_csr	((uint8_t volatile *) UART2_BASE + 0x0c)
#define uart2_nmr	((uint8_t volatile *) UART2_BASE + 0x0d)
#define uart2_mdm	((uint8_t volatile *) UART2_BASE + 0x0e)
#define uart2_rfc	((uint8_t volatile *) UART2_BASE + 0x0f)
#define uart2_gds	((uint8_t volatile *) UART2_BASE + 0x10)
#define uart2_dms	((uint8_t volatile *) UART2_BASE + 0x11)
#define uart2_pidx	((uint8_t volatile *) UART2_BASE + 0x12)
#define uart2_cka	((uint8_t volatile *) UART2_BASE + 0x13)

// Timers registers

#define TIMER_BASE	V3_IO_OFFSET + 0x340
#define timer_control_0	((uint8_t volatile *) TIMER_BASE + 0x00)
#define timer_control_1	((uint8_t volatile *) TIMER_BASE + 0x01)
#define timer_control_2	((uint8_t volatile *) TIMER_BASE + 0x02)
#define timer_control_3	((uint8_t volatile *) TIMER_BASE + 0x03)
#define timer_control_4	((uint8_t volatile *) TIMER_BASE + 0x04)
#define timer_int	((uint8_t volatile *) TIMER_BASE + 0x05)
#define timer_select	((uint8_t volatile *) TIMER_BASE + 0x06)
#define timer_wdg	((uint8_t volatile *) TIMER_BASE + 0x07)
#define timer_write_ls	((uint8_t volatile *) TIMER_BASE + 0x08)
#define timer_write_ms	((uint8_t volatile *) TIMER_BASE + 0x09)
#define timer_presc_ls	((uint8_t volatile *) TIMER_BASE + 0x0a)
#define timer_presc_ms	((uint8_t volatile *) TIMER_BASE + 0x0b)
#define timer_read_ls	((uint8_t volatile *) TIMER_BASE + 0x0c)
#define timer_read_ms	((uint8_t volatile *) TIMER_BASE + 0x0d)

// Camera registers

#define CAM_BASE	V3_IO_OFFSET + 0x360
#define cam_control	((uint32_t volatile *) CAM_BASE/4 + 0x00/4)
#define cam_fullness	((uint32_t volatile *) CAM_BASE/4 + 0x04/4)
#define cam_data	((uint32_t volatile *) CAM_BASE/4 + 0x08/4)
#define cam_misc	((uint32_t volatile *) CAM_BASE/4 + 0x0c/4)

// Debugger registers

#define DBG_BASE	V3_IO_OFFSET + 0x380
#define dbg_rsaddr0	((uint8_t volatile *) DBG_BASE + 0x00)
#define dbg_rsaddr1	((uint8_t volatile *) DBG_BASE + 0x01)
#define dbg_rsaddr2	((uint8_t volatile *) DBG_BASE + 0x02)
#define dbg_fsaddr0	((uint8_t volatile *) DBG_BASE + 0x03)
#define dbg_fsaddr1	((uint8_t volatile *) DBG_BASE + 0x04)
#define dbg_fsaddr2	((uint8_t volatile *) DBG_BASE + 0x05)
#define dbg_blength0	((uint8_t volatile *) DBG_BASE + 0x06)
#define dbg_blength1	((uint8_t volatile *) DBG_BASE + 0x07)
#define dbg_blength2	((uint8_t volatile *) DBG_BASE + 0x08)
#define dbg_command	((uint8_t volatile *) DBG_BASE + 0x09)
#define dbg_semaphore	((uint8_t volatile *) DBG_BASE + 0x0b)
#define dbg_config	((uint8_t volatile *) DBG_BASE + 0x0c)
#define dbg_status	((uint8_t volatile *) DBG_BASE + 0x0d)
#define dbg_drwdata	((uint8_t volatile *) DBG_BASE + 0x80)

// PWM registers

#define PWM_BASE	V3_IO_OFFSET + 0x3C0
#define pwm_control		((uint8_t volatile *) PWM_BASE + 0x00)
#define pwm_ctrl		((uint8_t volatile *) PWM_BASE + 0x01)
#define pwm_prescaler		((uint8_t volatile *) PWM_BASE + 0x02)
#define pwm_cnt16_lsb		((uint8_t volatile *) PWM_BASE + 0x03)
#define pwm_cnt16_msb		((uint8_t volatile *) PWM_BASE + 0x04)
#define pwm_cmp16_0_lsb		((uint8_t volatile *) PWM_BASE + 0x05)
#define pwm_cmp16_0_msb		((uint8_t volatile *) PWM_BASE + 0x06)
#define pwm_cmp16_1_lsb		((uint8_t volatile *) PWM_BASE + 0x07)
#define pwm_cmp16_1_msb		((uint8_t volatile *) PWM_BASE + 0x08)
#define pwm_cmp16_2_lsb		((uint8_t volatile *) PWM_BASE + 0x09)
#define pwm_cmp16_2_msb		((uint8_t volatile *) PWM_BASE + 0x0a)
#define pwm_cmp16_3_lsb		((uint8_t volatile *) PWM_BASE + 0x0b)
#define pwm_cmp16_3_msb		((uint8_t volatile *) PWM_BASE + 0x0c)
#define pwm_cmp16_4_lsb		((uint8_t volatile *) PWM_BASE + 0x0d)
#define pwm_cmp16_4_msb		((uint8_t volatile *) PWM_BASE + 0x0e)
#define pwm_cmp16_5_lsb		((uint8_t volatile *) PWM_BASE + 0x0f)
#define pwm_cmp16_5_msb		((uint8_t volatile *) PWM_BASE + 0x10)
#define pwm_cmp16_6_lsb		((uint8_t volatile *) PWM_BASE + 0x11)
#define pwm_cmp16_6_msb		((uint8_t volatile *) PWM_BASE + 0x12)
#define pwm_cmp16_7_lsb		((uint8_t volatile *) PWM_BASE + 0x13)
#define pwm_cmp16_7_msb		((uint8_t volatile *) PWM_BASE + 0x14)
#define pwm_out_toggle_en_0	((uint8_t volatile *) PWM_BASE + 0x15)
#define pwm_out_toggle_en_1	((uint8_t volatile *) PWM_BASE + 0x16)
#define pwm_out_toggle_en_2	((uint8_t volatile *) PWM_BASE + 0x17)
#define pwm_out_toggle_en_3	((uint8_t volatile *) PWM_BASE + 0x18)
#define pwm_out_toggle_en_4	((uint8_t volatile *) PWM_BASE + 0x19)
#define pwm_out_toggle_en_5	((uint8_t volatile *) PWM_BASE + 0x1a)
#define pwm_out_toggle_en_6	((uint8_t volatile *) PWM_BASE + 0x1b)
#define pwm_out_toggle_en_7	((uint8_t volatile *) PWM_BASE + 0x1c)
#define pwm_out_clr_en		((uint8_t volatile *) PWM_BASE + 0x1d)
#define pwm_ctrl_bl_cmp8	((uint8_t volatile *) PWM_BASE + 0x1e)
#define pwm_init_reg		((uint8_t volatile *) PWM_BASE + 0x1f)
#define pwm_intmask		((uint8_t volatile *) PWM_BASE + 0x20)
#define pwm_intstatus		((uint8_t volatile *) PWM_BASE + 0x21)
#define pwm_samplefreqh		((uint8_t volatile *) PWM_BASE + 0x22)
#define pwm_samplefreql		((uint8_t volatile *) PWM_BASE + 0x23)
#define pwm_fifo		((uint8_t volatile *) PWM_BASE + 0x3c)
#define pwm_fifo_w		((uint16_t volatile *) PWM_BASE/2 + 0x3c/2)
#define pwm_fifo_dw		((uint32_t volatile *) PWM_BASE/4 + 0x3c/4)

// SD Host registers

#define SDHOST_BASE		V3_IO_OFFSET + 0x400
#define sdhost_sdma		((uint32_t volatile *) SDHOST_BASE/4 + 0x00/4)
#define sdhost_bsr_bcr		((uint32_t volatile *) SDHOST_BASE/4 + 0x04/4)
// #define sdhost_bsr		((uint16_t volatile *) SDHOST_BASE/2 + 0x04/2)
// #define sdhost_bcr		((uint16_t volatile *) SDHOST_BASE/2 + 0x06/2)
#define sdhost_arg1reg		((uint32_t volatile *) SDHOST_BASE/4 + 0x08/4)
#define sdhost_tmr_cmdr		((uint32_t volatile *) SDHOST_BASE/4 + 0x0c/4)
// #define sdhost_tmr		((uint16_t volatile *) SDHOST_BASE/2 + 0x0c/2)
// #define sdhost_cmdr		((uint16_t volatile *) SDHOST_BASE/2 + 0x0e/2)
#define sdhost_response0	((uint32_t volatile *) SDHOST_BASE/4 + 0x10/4)
#define sdhost_response1	((uint32_t volatile *) SDHOST_BASE/4 + 0x14/4)
#define sdhost_response2	((uint32_t volatile *) SDHOST_BASE/4 + 0x18/4)
#define sdhost_response3	((uint32_t volatile *) SDHOST_BASE/4 + 0x1c/4)
#define sdhost_bufferdport	((uint32_t volatile *) SDHOST_BASE/4 + 0x20/4)
#define sdhost_presentstate	((uint32_t volatile *) SDHOST_BASE/4 + 0x24/4)
#define sdhost_hostcontrol1_powercontrol_blockgapcontrol	((uint32_t volatile *) SDHOST_BASE/4 + 0x28/4)
// #define sdhost_hostcontrol1	((uint8_t volatile *) SDHOST_BASE + 0x28)
// #define sdhost_powercontrol	((uint8_t volatile *) SDHOST_BASE + 0x29)
// #define sdhost_blockgapcontrol	((uint8_t volatile *) SDHOST_BASE + 0x2a)
#define sdhost_clockcontrol_timeoutcontrol_softwarereset	((uint32_t volatile *) SDHOST_BASE/4 + 0x2c/4)
// #define sdhost_clockcontrol	((uint8_t volatile *) SDHOST_BASE + 0x2c)
// #define sdhost_timeoutcontrol	((uint8_t volatile *) SDHOST_BASE + 0x2e)
// #define sdhost_softwarereset	((uint8_t volatile *) SDHOST_BASE + 0x2f)
#define sdhost_normintstatus_errintstatus	((uint32_t volatile *) SDHOST_BASE/4 + 0x30/4)
// #define sdhost_normintstatus	((uint16_t volatile *) SDHOST_BASE/2 + 0x30/2)
// #define sdhost_errintstatus	((uint16_t volatile *) SDHOST_BASE/2 + 0x32/2)
#define sdhost_normintstatus_en_errintstatus_en	((uint32_t volatile *) SDHOST_BASE/4 + 0x34/4)
// #define sdhost_normintstatus_en	((uint16_t volatile *) SDHOST_BASE/2 + 0x34/2)
// #define sdhost_errintstatus_en	((uint16_t volatile *) SDHOST_BASE/2 + 0x36/2)
#define sdhost_normintsig_en_errintsig_en	((uint32_t volatile *) SDHOST_BASE/4 + 0x38/4)
// #define sdhost_normintsig_en	((uint16_t volatile *) SDHOST_BASE/2 + 0x38/2)
// #define sdhost_errintsig_en	((uint16_t volatile *) SDHOST_BASE/2 + 0x3a/2)
#define sdhost_autocmd12_hostcontrol2	((uint32_t volatile *) SDHOST_BASE/4 + 0x3c/4)
// #define sdhost_autocmd12	((uint16_t volatile *) SDHOST_BASE/2 + 0x3c/2)
// #define sdhost_hostcontrol2	((uint16_t volatile *) SDHOST_BASE/2 + 0x3e/2)
#define sdhost_capreg0		((uint32_t volatile *) SDHOST_BASE/4 + 0x40/4)
#define sdhost_capreg1		((uint32_t volatile *) SDHOST_BASE/4 + 0x44/4)
#define sdhost_maxcurrentcap0	((uint32_t volatile *) SDHOST_BASE/4 + 0x48/4)
#define sdhost_maxcurrentcap1	((uint32_t volatile *) SDHOST_BASE/4 + 0x4c/4)
#define sdhost_forceeventcmd12_forceeventerrint	((uint32_t volatile *) SDHOST_BASE/4 + 0x50/4)
// #define sdhost_forceeventcmd12	((uint16_t volatile *) SDHOST_BASE/2 + 0x50/2)
// #define sdhost_forceeventerrint	((uint16_t volatile *) SDHOST_BASE/2 + 0x52/2)
#define sdhost_admaerrorstatus	((uint32_t volatile *) SDHOST_BASE/4 + 0x54/4)
// #define sdhost_admaerrorstatus	((uint8_t volatile *) SDHOST_BASE + 0x54)
#define sdhost_admasysaddr0	((uint32_t volatile *) SDHOST_BASE/4 + 0x58/4)
#define sdhost_admasysaddr1	((uint32_t volatile *) SDHOST_BASE/4 + 0x5c/4)
#define sdhost_presetvalue0	((uint32_t volatile *) SDHOST_BASE/4 + 0x60/4)
#define sdhost_presetvalue1	((uint32_t volatile *) SDHOST_BASE/4 + 0x64/4)
#define sdhost_presetvalue2	((uint32_t volatile *) SDHOST_BASE/4 + 0x68/4)
#define sdhost_presetvalue3	((uint32_t volatile *) SDHOST_BASE/4 + 0x6c/4)
#define sdhost_specversion	((uint32_t volatile *) SDHOST_BASE/4 + 0xfc/4)
// #define sdhost_specversion	((uint8_t volatile *) SDHOST_BASE + 0xfe)
#define sdhost_vendor1		((uint32_t volatile *) SDHOST_BASE/4 + 0x100/4)
#define sdhost_vendor2		((uint32_t volatile *) SDHOST_BASE/4 + 0x104/4)
#define sdhost_vendor3		((uint32_t volatile *) SDHOST_BASE/4 + 0x108/4)
#define sdhost_vendor4		((uint32_t volatile *) SDHOST_BASE/4 + 0x10c/4)
#define sdhost_vendor5		((uint32_t volatile *) SDHOST_BASE/4 + 0x110/4)
#define sdhost_vendor6		((uint32_t volatile *) SDHOST_BASE/4 + 0x114/4)
#define sdhost_vendor7		((uint32_t volatile *) SDHOST_BASE/4 + 0x118/4)
#define sdhost_hardware		((uint32_t volatile *) SDHOST_BASE/4 + 0x178/4)
#define sdhost_iprevision	((uint32_t volatile *) SDHOST_BASE/4 + 0x17c/4)
#define sdhost_cipherctrl_status_statusen	((uint32_t volatile *) SDHOST_BASE/4 + 0x180/4)
// #define sdhost_cipherstatus	((uint8_t volatile *) SDHOST_BASE + 0x184)
// #define sdhost_cipherstatusen	((uint8_t volatile *) SDHOST_BASE + 0x188)
#define sdhost_ciphersigen	((uint8_t volatile *) SDHOST_BASE + 0x18a)
#define sdhost_inputdata0	((uint32_t volatile *) SDHOST_BASE/4 + 0x18c/4)
#define sdhost_inputdata1	((uint32_t volatile *) SDHOST_BASE/4 + 0x190/4)
#define sdhost_inputkey0	((uint32_t volatile *) SDHOST_BASE/4 + 0x194/4)
#define sdhost_inputkey1	((uint32_t volatile *) SDHOST_BASE/4 + 0x198/4)
#define sdhost_outputdata0	((uint32_t volatile *) SDHOST_BASE/4 + 0x19c/4)
#define sdhost_outputdata1	((uint32_t volatile *) SDHOST_BASE/4 + 0x1a0/4)
#define sdhost_secrettable	((uint32_t volatile *) SDHOST_BASE/4 + 0x1a4/4)
// #define sdhost_secrettable	((uint8_t volatile *) SDHOST_BASE + 0x1a4)

// EHCI RAM Memory

#define EHCI_RAM_BASE		V3_IO_OFFSET + 0x1000
#define ehci_ram_b		((uint8_t volatile *) EHCI_RAM_BASE + 0x00)
#define ehci_ram_w		((uint16_t volatile *) EHCI_RAM_BASE/2 + 0x00/2)
#define ehci_ram_dw		((uint32_t volatile *) EHCI_RAM_BASE/4 + 0x00/4)

// Global variables for handling DMAC ISR
#define dmac_phy_access_done	((uint32_t volatile *) 0x500)
#define dmac_phy_rd_value	((uint32_t volatile *) 0x504)
#define dmac_phy_address	((uint8_t volatile *) 0x508)

// Global variables for handling UART1 and UART2 ISR
#define uart1_isr_done		((uint32_t volatile *) 0x510)
#define uart2_isr_done		((uint32_t volatile *) 0x514)

// Global variables for handling CAN1 and CAN2 ISR
#define can1_isr_done		((uint32_t volatile *) 0x518)
#define can2_isr_done		((uint32_t volatile *) 0x51c)

// Global variables for handling watchdog reset
#define prepare_for_watchdog	((uint32_t volatile *) 0x520)

// Global variables for handling Timers ISR
//#define timers_isr_done		((uint32_t volatile *) 0x524)

uint32_t *timers_isr_done;
uint32_t *spim_isr_done;
uint8_t spim_buffer[10];


//__flash__ uint8_t *timers_isr_done = (__flash__ uint8_t *)0x3fff0;

// Global variables for handling SD Card ISR
#define sdhost_iflag			((uint32_t volatile *) 0x528)
#define SDHOST_ISR_CMD_COMPLETE		0x01
#define SDHOST_ISR_TRANS_COMPLETE	0x02
#define SDHOST_ISR_BUFFER_WRDY		0x10
#define SDHOST_ISR_BUFFER_RRDY		0x20
#define SDHOST_ISR_CARD_INSERT		0x40
#define SDHOST_ISR_CARD_REMOVE		0x80
#define SDHOST_ISR_CARD_INTERRUPT	0x100

// Global variables for handling RTC Interrupt
#define rtc_isr_done	((uint32_t volatile *) 0x52c)

// Temporary data buffer, data coming from the external TB to DUT
#define ingress_buffer1		((uint8_t volatile *) 0x800)
#define ingress_buffer1_w	((uint16_t volatile *) 0x800)
#define ingress_buffer1_dw	((uint32_t volatile *) 0x800)

// Egress virtual channels. Dword or packets going out from DUT to external TB
// V3 indicates data transfer by setting valid and clk 0 -> 1. First dword data is valid at this point.
// Subsequent data is valid when clk transit 0 -> 1.
// End of transfer is indicated by valid going 0.
#define egress_ch0_valid_dw	((uint32_t volatile *) 0x600)
#define egress_ch0_data_dw	((uint32_t volatile *) 0x604)
#define egress_ch0_clk_dw	((uint32_t volatile *) 0x608)
#define egress_ch1_valid_dw	((uint32_t volatile *) 0x610)
#define egress_ch1_data_dw	((uint32_t volatile *) 0x614)
#define egress_ch1_clk_dw	((uint32_t volatile *) 0x618)
#define egress_ch2_valid_dw	((uint32_t volatile *) 0x620)
#define egress_ch2_data_dw	((uint32_t volatile *) 0x624)
#define egress_ch2_clk_dw	((uint32_t volatile *) 0x628)
#define egress_ch3_valid_dw	((uint32_t volatile *) 0x630)
#define egress_ch3_data_dw	((uint32_t volatile *) 0x634)
#define egress_ch3_clk_dw	((uint32_t volatile *) 0x638)

// Ingress virtual channels. Dword or packets coming into the DUT from external TB
// TB puts valid data and start request transfer 0 -> 1.
// v3 get data and send ack pulse 0 -> 1 -> 0 to acknowledge.
// TB puts valid data while ack goes 1 -> 0.
// v3 get data and send ack pulse 0 -> 1 -> 0 to acknowledge.
// End of transfer: TB set req to 0 when ack transits 1 -> 0. 
#define ingress_ch0_req_dw	((uint32_t volatile *) 0x670)
#define ingress_ch0_ack_dw	((uint32_t volatile *) 0x674)
#define ingress_ch0_data_dw	((uint32_t volatile *) 0x678)
#define ingress_ch1_req_dw	((uint32_t volatile *) 0x680)
#define ingress_ch1_ack_dw	((uint32_t volatile *) 0x684)
#define ingress_ch1_data_dw	((uint32_t volatile *) 0x688)
#define ingress_ch2_req_dw	((uint32_t volatile *) 0x690)
#define ingress_ch2_ack_dw	((uint32_t volatile *) 0x694)
#define ingress_ch2_data_dw	((uint32_t volatile *) 0x698)
#define ingress_ch3_req_dw	((uint32_t volatile *) 0x6a0)
#define ingress_ch3_ack_dw	((uint32_t volatile *) 0x6a4)
#define ingress_ch3_data_dw	((uint32_t volatile *) 0x6a8)

// Interrupt monitor channel.
// When interrupt is trigger, trig goes from 0 -> 1.
// intr_number indicates which reset or interrupt is triggered.
// 0 -> system reset, 1 -> watch dog reset, 2 -> interrupt 0, .... 33 -> interrupt 31
// intr_mod indicates which peripheral triggers the interrupt.
// intr_mod_iflag displays the peripheral interrupt flag.
#define v3_intr_trig		((uint32_t volatile *) 0x700)
#define v3_intr_number		((uint32_t volatile *) 0x704)
#define v3_intr_mod		((uint32_t volatile *) 0x708)
#define v3_intr_mod_iflag	((uint32_t volatile *) 0x70c)

// v3 message channel, for print messages via Testbench
// Indicates to the TB where it has executed in the program
// Message number is valid when valid goes 0->1.
#define msg_valid		((uint32_t volatile *) 0x710)
#define msg_number		((uint32_t volatile *) 0x714)

#define MSG_PROG_PM_START	0x0100 // Start of Main Program from Program Memory.
#define MSG_PROG_FL_START	0x0200 // Start of Main Program from Flash Memory.
#define MSG_DEBUGGER_START	0x0300 // Goes to Debugger test routine
#define MSG_DMAC_START		0x0400 // Goes to Ethernet test routine
#define MSG_EHCI_START		0x0500 // Goes to Host USB test routine
#define MSG_EHCI_DEV_START	0x0510 // Goes to Host USB and USB Device test routine
#define MSG_EHCI_RAM_START	0x0580 // Goes to Host USB RAM test routine
#define MSG_DBUS_START		0x0600 // Goes to Device USB test routine
#define MSG_REGTEST_START	0x0700 // Goes to Register test routine
#define MSG_UART1_START		0x0800 // Goes to UART1 test routine
#define MSG_UART1_RECEIVE	0x0810 // Goes to UART1 test routine
#define MSG_UART2_START		0x0900 // Goes to UART2 test routine
#define MSG_UART2_RECEIVE	0x0910 // Goes to UART2 test routine
#define MSG_INTERRUPT_START	0x0A00 // Goes to Interrupt test routine
#define MSG_USB_HOST_START	0x0B00 // Goes to USB Host test routine
#define MSG_USB_DEVICE_START	0x0C00 // Goes to USB Device test routine
#define MSG_CAN1_START		0x0D00 // Goes to CAN1 test routine
#define MSG_CAN1_RECEIVE	0x0D10 // Goes to CAN1 test routine
#define MSG_CAN2_START		0x0E00 // Goes to CAN2 test routine
#define MSG_CAN2_RECEIVE	0x0E10 // Goes to CAN2 test routine
#define MSG_RTC_START		0x0F00 // Goes to RTC test routine
#define MSG_SPIM_START		0x1000 // Goes to SPIM test routine
#define MSG_SPIS1_START		0x1100 // Goes to SPIS1 test routine
#define MSG_SPIS2_START		0x1200 // Goes to SPIS2 test routine
#define MSG_I2CM_START		0x1300 // Goes to I2CM test routine
#define MSG_I2CS_START		0x1400 // Goes to I2CS test routine
#define MSG_TIMERS_START	0x1500 // Goes to Timers test routine
#define MSG_WATCHDOG_START	0x1600 // Goes to Watchdog test routine
#define MSG_SDHOST_START	0x1700 // Goes to SDHOST test routine
#define MSG_SDHOST_CDETECT1	0x1710 // Testbench to set SD Host card detect to 1
#define MSG_SDHOST_CDETECT0	0x1720 // Testbench to set SD Host card detect to 0
#define MSG_SDHOST_WPROTECT1	0x1730 // Testbench to set SD Host write protect to 1
#define MSG_SDHOST_WPROTECT0	0x1740 // Testbench to set SD Host write protect to 0
#define MSG_PWM_START		0x1800 // Goes to PWM test routine
#define MSG_BOOT_FLASH_START	0x1900 // Goes to Boot and Flash control test routine
#define MSG_REGISTER_START	0x1A00 // Goes to Register Access test routine, will indicate pass or fail from v3
#define MSG_RESET_0		0xA000 // Main Reset.
#define MSG_WATCHDOG		0xA010 // Watchdog triggered.
#define MSG_INTERRUPT_0		0xA020 // Interrupt 0 triggered.
#define MSG_INTERRUPT_1		0xA030 // Interrupt 1 triggered.
#define MSG_INTERRUPT_2		0xA040 // Interrupt 2 triggered.
#define MSG_INTERRUPT_3		0xA050 // Interrupt 3 triggered.
#define MSG_INTERRUPT_4		0xA060 // Interrupt 4 triggered.
#define MSG_INTERRUPT_5		0xA070 // Interrupt 5 triggered.
#define MSG_INTERRUPT_6		0xA080 // Interrupt 6 triggered.
#define MSG_INTERRUPT_7		0xA090 // Interrupt 7 triggered.
#define MSG_INTERRUPT_8		0xA0A0 // Interrupt 8 triggered.
#define MSG_INTERRUPT_9		0xA0B0 // Interrupt 9 triggered.
#define MSG_INTERRUPT_10	0xA0C0 // Interrupt 10 triggered.
#define MSG_INTERRUPT_11	0xA0D0 // Interrupt 11 triggered.
#define MSG_INTERRUPT_12	0xA0E0 // Interrupt 12 triggered.
#define MSG_INTERRUPT_13	0xA0F0 // Interrupt 13 triggered.
#define MSG_INTERRUPT_14	0xA100 // Interrupt 14 triggered.
#define MSG_INTERRUPT_15	0xA110 // Interrupt 15 triggered.
#define MSG_INTERRUPT_16	0xA120 // Interrupt 16 triggered.
#define MSG_INTERRUPT_17	0xA130 // Interrupt 17 triggered.
#define MSG_INTERRUPT_18	0xA140 // Interrupt 18 triggered.
#define MSG_INTERRUPT_19	0xA150 // Interrupt 19 triggered.
#define MSG_INTERRUPT_20	0xA160 // Interrupt 20 triggered.
#define MSG_INTERRUPT_21	0xA170 // Interrupt 21 triggered.
#define MSG_INTERRUPT_22	0xA180 // Interrupt 22 triggered.
#define MSG_INTERRUPT_23	0xA190 // Interrupt 23 triggered.
#define MSG_INTERRUPT_24	0xA1A0 // Interrupt 24 triggered.
#define MSG_INTERRUPT_25	0xA1B0 // Interrupt 25 triggered.
#define MSG_INTERRUPT_26	0xA1C0 // Interrupt 26 triggered.
#define MSG_INTERRUPT_27	0xA1D0 // Interrupt 27 triggered.
#define MSG_INTERRUPT_28	0xA1E0 // Interrupt 28 triggered.
#define MSG_INTERRUPT_29	0xA1F0 // Interrupt 29 triggered.
#define MSG_INTERRUPT_30	0xA200 // Interrupt 30 triggered.
#define MSG_INTERRUPT_31	0xA210 // Interrupt 31 triggered.
#define MSG_PROG_DONE		0xfD00 // End of test routine.
#define MSG_PROG_PASS		0xf5a5 // Test routine PASSED.
#define MSG_PROG_FAIL		0xdead // Test routine FAILED!!

// These number corresponds to the actual physical location in ft900_digital_top irq_inputs. 
// This shows the default interrupt assign numbers.
#define	PM_IRQ		0
#define	EHCI_IRQ	1
#define	DUSB2_IRQ	2
#define	DMAC_IRQ	3
#define	SDHOST_IRQ	4
#define	CAN1_IRQ	5
#define	CAN2_IRQ	6
#define	CAM_IRQ		7
#define	SPIM_IRQ	8
#define	SPIS1_IRQ	9
#define	SPIS2_IRQ	10
#define	I2CM_IRQ	11
#define	I2CS_IRQ	12
#define	UART1_IRQ	13
#define	UART2_IRQ	14
#define	I2S_IRQ		15
#define	PWM_IRQ		16
#define	TIMERS_IRQ	17
#define	GPIO_IRQ	18
#define	RTC_IRQ		19
#define	ADC_IRQ		20
#define	DIF_IRQ		31
#define	UNKNOWN_IRQ	255

// Debugger commands
#define DBG_ABORT	0x00
#define DBG_CMDWREN	0x06
#define DBG_CMDWRDI	0x04
#define DBG_CMDWRSR	0x01
#define DBG_CMDRDID	0x9f
#define DBG_CMDRDSR	0x05
#define DBG_CMDRDSFDP	0x5a
#define DBG_CMDSE	0x20
#define DBG_CMDBE1	0x52
#define DBG_CMDBE2	0xd8
#define DBG_CMDCE1	0x60
#define DBG_CMDCE2	0xc7
#define DBG_CMDDP	0xb9
#define DBG_CMDRDP	0xab
#define DBG_CMDDBG2P1	0xe0
#define DBG_CMDDBG2P2	0xe1
#define DBG_CMDDBG2P3	0xe2
#define DBG_CMDDBG2D1	0xe4
#define DBG_CMDDBG2D2	0xe5
#define DBG_CMDDBG2D3	0xe6
#define DBG_CMDDBG2F1	0xe8
#define DBG_CMDDBG2F2	0xe9
#define DBG_CMDDBG2F3	0xea
#define DBG_CMDP2F1	0xf0
#define DBG_CMDP2F2	0xf1
#define DBG_CMDP2F3	0xf2
#define DBG_CMDF2P1	0xf4
#define DBG_CMDF2P2	0xf5
#define DBG_CMDF2P3	0xf6
#define DBG_CMDD2F1	0xf8
#define DBG_CMDD2F2	0xf9
#define DBG_CMDD2F3	0xfa
#define DBG_CMDF2D1	0xfc
#define DBG_CMDF2D2	0xfd
#define DBG_CMDF2D3	0xfe


//#########################################################################
// Functions/ Features define
//
//########################################################################
void uart1_init ();
void uart1_test_routine();

void led_init ();
int leds_display(uint8_t num, uint8_t d);
void ethernetled_init();

void timer_init ();
void timer_start(uint8_t num, uint8_t ms, uint8_t ls);
void timer_stop(uint8_t num);

uint8_t handle_int(void);
uint8_t timers_int(void);

void spi_master_init ();
void spi_master_conf (uint8_t mode);
void spi_master_ss (uint8_t num);
void spi_master_send_command (uint8_t cm);
void spi_master_write_data (uint8_t *data);
void spi_master_read_data (uint8_t *data);
uint8_t spi_write (uint8_t *buffer, uint32_t sizetotransfer, uint32_t *sizetransferred, uint32_t transferoption);
uint8_t spi_read (uint8_t *buffer, uint32_t sizetotransfer, uint32_t *sizetransferred, uint32_t transferoption);
void spi_1_slave_init ();
void spi_1_slave_conf (uint8_t mode);
void spi_2_slave_init ();
void spi_2_slave_conf (uint8_t mode);

uint8_t spi_master_testing(void);
uint8_t spim_spi1_slave_testing(void);
uint8_t spim_spi2_slave_testing(void);
uint8_t spim_ft800_testing(void);

uint8_t usbd_testing (void);
uint8_t usbh_testing (void);

uint8_t can_testing (void);

uint8_t i2c_master_testing();
uint8_t i2c_master_slave_testing();
uint8_t i2c_master_ack_polling();


uint8_t pwm_testing (void);
uint8_t pwm_stereo_audio_testing (void);


# endif
