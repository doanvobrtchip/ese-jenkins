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

Author : FTDI 

Revision History: 
0.1 - date 2013.04.24 - Initial Version
0.2 - date 2013.08.19 - added few APIs
1.0 - date 2015.03.16 - Added new APIs for FT81X
*/

#include "FT_Platform.h"

/* API to initialize the SPI interface */
ft_bool_t  Ft_Gpu_Hal_Init(Ft_Gpu_HalInit_t *halinit)
{
#ifdef FT900_PLATFORM
	
	
	// Initialize SPIM HW
	SPISysInit(SPIM);

	GPIOSetPinMode(FT800_PD_N,GPIO_MODE_OUTPUT);//power down pin
	GPIOWrite(FT800_PD_N, 1);

#endif

#ifdef ARDUINO_PLATFORM_SPI
            pinMode(FT_ARDUINO_PRO_SPI_CS, OUTPUT);
      pinMode(FT800_PD_N, OUTPUT);
      digitalWrite(FT_ARDUINO_PRO_SPI_CS, HIGH);
            digitalWrite(FT800_PD_N, HIGH);
#endif

#ifdef MSVC_PLATFORM_SPI
	/* Initialize the libmpsse */
        Init_libMPSSE();	
	SPI_GetNumChannels(&halinit->TotalChannelNum);
	/* By default i am assuming only one mpsse cable is connected to PC and channel 0 of that mpsse cable is used for spi transactions */
	if(halinit->TotalChannelNum > 0)
	{
        FT_DEVICE_LIST_INFO_NODE devList;
		SPI_GetChannelInfo(0,&devList);
		printf("Information on channel number %d:\n",0);
		/* print the dev info */
		printf(" Flags=0x%x\n",devList.Flags);
		printf(" Type=0x%x\n",devList.Type);
		printf(" ID=0x%x\n",devList.ID);
		printf(" LocId=0x%x\n",devList.LocId);
		printf(" SerialNumber=%s\n",devList.SerialNumber);
		printf(" Description=%s\n",devList.Description);
		printf(" ftHandle=0x%x\n",devList.ftHandle);/*is 0 unless open*/
	}
#endif
	return TRUE;
}
ft_bool_t    Ft_Gpu_Hal_Open(Ft_Gpu_Hal_Context_t *host)
{
#ifdef FT900_PLATFORM
	/* Configure the SPI with 6.25MHz clock as default and single channel */
	TSPIBasicConfig      SPIMConfig;
	
	// Initialize SPIM HW
	SPISysInit(SPIM);
	// Configure SPIM
	SPIMConfig.enable         = DQSPI_SYS_ON;					// Enable SPI
    SPIMConfig.intEnable      = DQSPI_INT_DISABLED;				// Disable interrupt
    SPIMConfig.mode           = DQSPI_MODE_MASTER;				// Master mode
    SPIMConfig.format         = DQSPI_TRANSFER_STD;				// Single data transfer
    SPIMConfig.ratio          = DQSPI_CR16;           			// CLK/8
    SPIMConfig.polarity       = DQSPI_SCK_CPOL_0;        		// SCK polarity 0
    SPIMConfig.phase          = DQSPI_SCK_CPHA_0;           	// SCK phase 0
    SPIMConfig.slaveAutoSS    = DQSPI_AUTO_SS_DISABLED;     	// Disable SS auto assertion
	SPIMConfig.fifoEnable     = DQSPI_FIFO_DISABLED;      		// Enable FIFO
    SPIMConfig.fifoMode       = DQSPI_FIFO_DEPTH_64;        	// FIFO depth 64 bytes
	SPIMConfig.fifoTO         = DQSPI_FIFO_TIMEOUT_DISABLED;	// FIFO timeout interrupt disabled
    SPIMConfig.fifoRxTrigger  = DQSPI_FR_FIRST;   				// FIFO receive trigger at 1st byte
    SPIMConfig.txEmpty        = DQSPI_TX_EMPTY_DISABLED;		// Disable transmitter empty interrupt
    SPIMConfig.modeDMA        = False;							// No DMA
    dqspiInit(SPIM, &SPIMConfig);
	// Reset XMIT and RCVR FIFO on both master and slave
	dqspiResetFIFO(SPIM, DQSPI_RX_TX_FIFO);
	
	//dqspiEnableSS(SPIM,FT800_SEL_PIN);//slave select pin
	
	dqspiDisableSS(SPIM,FT800_SEL_PIN);
#endif
#ifdef MSVC_FT800EMU
	Ft_GpuEmu_SPII2C_begin();
#endif
#ifdef ARDUINO_PLATFORM_SPI
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
#endif
#ifdef MSVC_PLATFORM_SPI
        ChannelConfig channelConf;			//channel configuration
	FT_STATUS status;					
		
	/* configure the spi settings */
	channelConf.ClockRate = host->hal_config.spi_clockrate_khz * 1000; 
	channelConf.LatencyTimer= 2;       
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x00000000;	/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

	/* Open the first available channel */
	SPI_OpenChannel(host->hal_config.channel_no,(FT_HANDLE *)&host->hal_handle);
	status = SPI_InitChannel((FT_HANDLE)host->hal_handle,&channelConf);
	printf("\nhandle=0x%x status=0x%x\n",host->hal_handle,status);	
#endif

	/* Initialize the context valriables */
	host->ft_cmd_fifo_wp = host->ft_dl_buff_wp = 0;
	host->spinumdummy = 1;//by default ft800/801/810/811 goes with single dummy byte for read
	host->spichannel = 0;
	host->status = FT_GPU_HAL_OPENED;
	return TRUE;
}
ft_void_t  Ft_Gpu_Hal_Close(Ft_Gpu_Hal_Context_t *host)
{
	host->status = FT_GPU_HAL_CLOSED;
#ifdef MSVC_PLATFORM_SPI	
	/* Close the channel*/
	SPI_CloseChannel(host->hal_handle);
#endif
#ifdef ARDUINO_PLATFORM_SPI
        SPI.end();
#endif
#ifdef MSVC_FT800EMU
	Ft_GpuEmu_SPII2C_end();
#endif
}

ft_void_t Ft_Gpu_Hal_DeInit()
{
#ifdef MSVC_PLATFORM_SPI
   //Cleanup the MPSSE Lib
   Cleanup_libMPSSE();
#endif
}

/*The APIs for reading/writing transfer continuously only with small buffer system*/
ft_void_t  Ft_Gpu_Hal_StartTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw,ft_uint32_t addr)
{
	if (FT_GPU_READ == rw){

#ifdef FT900_PLATFORM
		ft_uint8_t spidata[4];
		spidata[0] = (addr >> 16);
		spidata[1] = (addr >> 8);
		spidata[2] =  addr &0xff;
		dqspiEnableSS(SPIM, FT800_SEL_PIN);
		dqspiWritePool(SPIM, spidata, 3,DQSPI_M_DISCARD_INCOMING_DATA);
#endif

#ifdef MSVC_PLATFORM_SPI
		ft_uint8_t Transfer_Array[4];
		ft_uint32_t SizeTransfered;

		/* Compose the read packet */
		Transfer_Array[0] = addr >> 16;
		Transfer_Array[1] = addr >> 8;
		Transfer_Array[2] = addr;

		Transfer_Array[3] = 0; //Dummy Read byte
		SPI_Write((FT_HANDLE)host->hal_handle,Transfer_Array,sizeof(Transfer_Array),&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
#endif
#ifdef ARDUINO_PLATFORM_SPI
		digitalWrite(FT_ARDUINO_PRO_SPI_CS, LOW);
		SPI.transfer(addr >> 16);
		SPI.transfer(highByte(addr));
		SPI.transfer(lowByte(addr));

		SPI.transfer(0); //Dummy Read Byte
#endif

#ifdef MSVC_FT800EMU
		Ft_GpuEmu_SPII2C_StartRead(addr);
#endif
		host->status = FT_GPU_HAL_READING;
	}else{
	
#ifdef FT900_PLATFORM
		ft_uint8_t spidata[4];
		spidata[0] = (0x80|(addr >> 16));
		spidata[1] = (addr >> 8);
		spidata[2] = addr;
		dqspiEnableSS(SPIM, FT800_SEL_PIN);
		dqspiWritePool(SPIM, spidata, 3,DQSPI_M_DISCARD_INCOMING_DATA);
#endif
	
#ifdef MSVC_PLATFORM_SPI
		ft_uint8_t Transfer_Array[3];
		ft_uint32_t SizeTransfered;

		/* Compose the read packet */
		Transfer_Array[0] = (0x80 | (addr >> 16));
		Transfer_Array[1] = addr >> 8;
		Transfer_Array[2] = addr;
		SPI_Write((FT_HANDLE)host->hal_handle,Transfer_Array,3,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);		
#endif
#ifdef ARDUINO_PLATFORM_SPI
		digitalWrite(FT_ARDUINO_PRO_SPI_CS, LOW);
		SPI.transfer(0x80 | (addr >> 16));
		SPI.transfer(highByte(addr));
		SPI.transfer(lowByte(addr));
#endif
#ifdef MSVC_FT800EMU
		Ft_GpuEmu_SPII2C_StartWrite(addr);
#endif
		host->status = FT_GPU_HAL_WRITING;
	}
}



/*The APIs for writing transfer continuously only*/
ft_void_t  Ft_Gpu_Hal_StartCmdTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw, ft_uint16_t count)
{
	Ft_Gpu_Hal_StartTransfer(host,rw,host->ft_cmd_fifo_wp + RAM_CMD);
}

ft_uint8_t    Ft_Gpu_Hal_TransferString(Ft_Gpu_Hal_Context_t *host,const ft_char8_t *string)
{
    ft_uint16_t length = strlen(string);
    while(length --){
       Ft_Gpu_Hal_Transfer8(host,*string);
       string ++;
    }
    //Append one null as ending flag
    Ft_Gpu_Hal_Transfer8(host,0);
}


ft_uint8_t    Ft_Gpu_Hal_Transfer8(Ft_Gpu_Hal_Context_t *host,ft_uint8_t value)
{

#ifdef FT900_PLATFORM
		dqspiWritePool(SPIM, &value, 1,DQSPI_M_DISCARD_INCOMING_DATA);
		return 0;
#endif
#ifdef ARDUINO_PLATFORM_SPI
        return SPI.transfer(value);
#endif
#ifdef MSVC_PLATFORM_SPI
	ft_uint32_t SizeTransfered;
	if (host->status == FT_GPU_HAL_WRITING){
		SPI_Write(host->hal_handle,&value,sizeof(value),&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
	}else{
		SPI_Read(host->hal_handle,&value,sizeof(value),&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
	}

	if (SizeTransfered != sizeof(value))
		host->status = FT_GPU_HAL_STATUS_ERROR;
        return value;
#endif	

#ifdef MSVC_FT800EMU
	return Ft_GpuEmu_SPII2C_transfer(value);
#endif
}


ft_uint16_t  Ft_Gpu_Hal_Transfer16(Ft_Gpu_Hal_Context_t *host,ft_uint16_t value)
{
	ft_uint16_t retVal = 0;

        if (host->status == FT_GPU_HAL_WRITING){
		Ft_Gpu_Hal_Transfer8(host,value & 0xFF);//LSB first
		Ft_Gpu_Hal_Transfer8(host,(value >> 8) & 0xFF);
	}else{
		retVal = Ft_Gpu_Hal_Transfer8(host,0);
		retVal |= (ft_uint16_t)Ft_Gpu_Hal_Transfer8(host,0) << 8;
	}

	return retVal;
}
ft_uint32_t  Ft_Gpu_Hal_Transfer32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t value)
{
	ft_uint32_t retVal = 0;
	if (host->status == FT_GPU_HAL_WRITING){
		Ft_Gpu_Hal_Transfer16(host,value & 0xFFFF);//LSB first
		Ft_Gpu_Hal_Transfer16(host,(value >> 16) & 0xFFFF);
	}else{
		retVal = Ft_Gpu_Hal_Transfer16(host,0);
		retVal |= (ft_uint32_t)Ft_Gpu_Hal_Transfer16(host,0) << 16;
	}
	return retVal;
}

ft_void_t   Ft_Gpu_Hal_EndTransfer(Ft_Gpu_Hal_Context_t *host)
{

#ifdef FT900_PLATFORM
	dqspiDisableSS(SPIM,FT800_SEL_PIN);
#endif
#ifdef MSVC_PLATFORM_SPI  
	//just disbale the CS - send 0 bytes with CS disable
	SPI_ToggleCS((FT_HANDLE)host->hal_handle,FALSE);
#endif
#ifdef ARDUINO_PLATFORM_SPI
	digitalWrite(FT_ARDUINO_PRO_SPI_CS, HIGH);
#endif

#ifdef MSVC_FT800EMU
	Ft_GpuEmu_SPII2C_csHigh();
#endif
	host->status = FT_GPU_HAL_OPENED;
}


ft_uint8_t  Ft_Gpu_Hal_Rd8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint8_t value;
#ifdef FT900_PLATFORM
	unsigned char spiData[4] = {0};
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	dqspiReadPool(SPIM, spiData, host->spinumdummy + 1,DQSPI_M_SEND_DUMMY_AND_READ);
	value = spiData[host->spinumdummy];
#else
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	value = Ft_Gpu_Hal_Transfer8(host,0);
#endif
	Ft_Gpu_Hal_EndTransfer(host);
	return value;
}
ft_uint16_t Ft_Gpu_Hal_Rd16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint16_t value;
#ifdef FT900_PLATFORM
	unsigned char spiData[4] = {0};
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	dqspiReadPool(SPIM, spiData, host->spinumdummy + 2,DQSPI_M_SEND_DUMMY_AND_READ);
	value = spiData[host->spinumdummy] |(spiData[host->spinumdummy+1] << 8) ;
#else
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	value = Ft_Gpu_Hal_Transfer16(host,0);
#endif
	Ft_Gpu_Hal_EndTransfer(host);
	return value;
}
ft_uint32_t Ft_Gpu_Hal_Rd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint32_t value;
#ifdef FT900_PLATFORM
	unsigned char spiData[8] = {0};
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	dqspiReadPool(SPIM, spiData, host->spinumdummy+4,DQSPI_M_SEND_DUMMY_AND_READ);
	value = (spiData[host->spinumdummy+3] << 24) | (spiData[host->spinumdummy+2] << 16) | (spiData[host->spinumdummy+1] << 8) | spiData[host->spinumdummy];
#else
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
	value = Ft_Gpu_Hal_Transfer32(host,0);
#endif
	Ft_Gpu_Hal_EndTransfer(host);
	return value;
}

ft_void_t Ft_Gpu_Hal_Wr8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t v)
{	
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	Ft_Gpu_Hal_Transfer8(host,v);
	Ft_Gpu_Hal_EndTransfer(host);
}
ft_void_t Ft_Gpu_Hal_Wr16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint16_t v)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	Ft_Gpu_Hal_Transfer16(host,v);
	Ft_Gpu_Hal_EndTransfer(host);
}
ft_void_t Ft_Gpu_Hal_Wr32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint32_t v)
{
	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
	Ft_Gpu_Hal_Transfer32(host,v);
	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_HostCommand(Ft_Gpu_Hal_Context_t *host,ft_uint8_t cmd)
{

#ifdef FT900_PLATFORM
	ft_uint8_t hcmd[4] = {0};
	hcmd[0] = cmd;
	hcmd[1] = 0;
	hcmd[2] = 0;
	hcmd[3] = 0;
	dqspiEnableSS(SPIM,FT800_SEL_PIN);
	dqspiWritePool(SPIM, hcmd, 3,DQSPI_M_DISCARD_INCOMING_DATA);
	dqspiDisableSS(SPIM,FT800_SEL_PIN);
#endif
#ifdef MSVC_PLATFORM_SPI
  ft_uint8_t Transfer_Array[3];
  ft_uint32_t SizeTransfered;

  Transfer_Array[0] = cmd;
  Transfer_Array[1] = 0;
  Transfer_Array[2] = 0;

  SPI_Write(host->hal_handle,Transfer_Array,sizeof(Transfer_Array),&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
#endif
#ifdef ARDUINO_PLATFORM_SPI
  digitalWrite(FT_ARDUINO_PRO_SPI_CS, LOW);
  SPI.transfer(cmd);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(FT_ARDUINO_PRO_SPI_CS, HIGH);
#endif
#ifdef MSVC_FT800EMU
  //Not implemented in FT800EMU
#endif
}

ft_void_t Ft_Gpu_ClockSelect(Ft_Gpu_Hal_Context_t *host,FT_GPU_PLL_SOURCE_T pllsource)
{
   Ft_Gpu_HostCommand(host,pllsource);
}
ft_void_t Ft_Gpu_PLL_FreqSelect(Ft_Gpu_Hal_Context_t *host,FT_GPU_PLL_FREQ_T freq)
{
   Ft_Gpu_HostCommand(host,freq);
}
ft_void_t Ft_Gpu_PowerModeSwitch(Ft_Gpu_Hal_Context_t *host,FT_GPU_POWER_MODE_T pwrmode)
{
   Ft_Gpu_HostCommand(host,pwrmode);
}
ft_void_t Ft_Gpu_CoreReset(Ft_Gpu_Hal_Context_t *host)
{
   Ft_Gpu_HostCommand(host,FT_GPU_CORE_RESET);
}


#ifdef FT_81X_ENABLE
//This API can only be called when PLL is stopped(SLEEP mode).  For compatibility, set frequency to the FT_GPU_12MHZ option in the FT_GPU_SETPLLSP1_T table.
ft_void_t Ft_Gpu_81X_SelectSysCLK(Ft_Gpu_Hal_Context_t *host, FT_GPU_81X_PLL_FREQ_T freq){
		if(FT_GPU_SYSCLK_72M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x06 << 8)); 
		else if(FT_GPU_SYSCLK_60M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x05 << 8)); 
		else if(FT_GPU_SYSCLK_48M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x40 << 8) | (0x04 << 8)); 
		else if(FT_GPU_SYSCLK_36M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x03 << 8)); 
		else if(FT_GPU_SYSCLK_24M == freq)
			Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x61 | (0x02 << 8)); 
		else if(FT_GPU_SYSCLK_DEFAULT == freq)//default clock
			Ft_Gpu_HostCommand_Ext3(host, 0x61); 
}

//Power down or up ROMs and ADCs.  Specified one or more elements in the FT_GPU_81X_ROM_AND_ADC_T table to power down, unspecified elements will be powered up.  The application must retain the state of the ROMs and ADCs as they're not readable from the device.
ft_void_t Ft_GPU_81X_PowerOffComponents(Ft_Gpu_Hal_Context_t *host, ft_uint8_t val){
		Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x49 | (val<<8));
}

//this API sets the current strength of supported GPIO/IO group(s)
ft_void_t Ft_GPU_81X_PadDriveStrength(Ft_Gpu_Hal_Context_t *host, FT_GPU_81X_GPIO_DRIVE_STRENGTH_T strength, FT_GPU_81X_GPIO_GROUP_T group){
		Ft_Gpu_HostCommand_Ext3(host, (ft_uint32_t)0x70 | (group << 8) | (strength << 8));
}

//this API will hold the system reset active, Ft_Gpu_81X_ResetRemoval() must be called to release the system reset.
ft_void_t Ft_Gpu_81X_ResetActive(Ft_Gpu_Hal_Context_t *host){
	Ft_Gpu_HostCommand_Ext3(host, FT_GPU_81X_RESET_ACTIVE); 
}

//This API will release the system reset, and the system will exit reset and behave as after POR, settings done through SPI commands will not be affected.
ft_void_t Ft_Gpu_81X_ResetRemoval(Ft_Gpu_Hal_Context_t *host){
	Ft_Gpu_HostCommand_Ext3(host, FT_GPU_81X_RESET_REMOVAL); 
}
#endif


//This API sends a 3byte command to the host
ft_void_t Ft_Gpu_HostCommand_Ext3(Ft_Gpu_Hal_Context_t *host,ft_uint32_t cmd)
{
	#ifdef FT900_PLATFORM
		ft_uint8_t hcmd[4] = {0};
		hcmd[0] = cmd & 0xff;
		hcmd[1] = (cmd>>8) & 0xff;
		hcmd[2] = (cmd>>16) & 0xff;
		hcmd[3] = 0;
		dqspiEnableSS(SPIM,FT800_SEL_PIN);
		dqspiWritePool(SPIM, hcmd, 3,DQSPI_M_DISCARD_INCOMING_DATA);
		dqspiDisableSS(SPIM,FT800_SEL_PIN);
	#endif
	#ifdef MSVC_PLATFORM_SPI
	  ft_uint8_t Transfer_Array[3];
	  ft_uint32_t SizeTransfered;
	
	  Transfer_Array[0] = cmd;
	  Transfer_Array[1] = (cmd>>8) & 0xff;
	  Transfer_Array[2] = (cmd>>16) & 0xff;
	
	  SPI_Write(host->hal_handle,Transfer_Array,sizeof(Transfer_Array),&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	#endif
	#ifdef ARDUINO_PLATFORM_SPI
	  digitalWrite(FT_ARDUINO_PRO_SPI_CS, LOW);
	  SPI.transfer(cmd);
	  SPI.transfer((cmd>>8) & 0xff);
	  SPI.transfer((cmd>>16) & 0xff);
	  digitalWrite(FT_ARDUINO_PRO_SPI_CS, HIGH);
	#endif
	#ifdef MSVC_FT800EMU
	  //Not implemented in FT800EMU
	#endif
}



ft_void_t Ft_Gpu_Hal_Updatecmdfifo(Ft_Gpu_Hal_Context_t *host,ft_uint32_t count)
{
	host->ft_cmd_fifo_wp  = (host->ft_cmd_fifo_wp + count) & 4095;

	//4 byte alignment
	host->ft_cmd_fifo_wp = (host->ft_cmd_fifo_wp + 3) & 0xffc;
	Ft_Gpu_Hal_Wr16(host,REG_CMD_WRITE,host->ft_cmd_fifo_wp);
}


ft_uint16_t Ft_Gpu_Cmdfifo_Freespace(Ft_Gpu_Hal_Context_t *host)
{
#if defined(FT_81X_ENABLE)
	return Ft_Gpu_Hal_Rd32(host,REG_CMDB_SPACE);
#else

	ft_uint16_t fullness,retval;

	fullness = (host->ft_cmd_fifo_wp - Ft_Gpu_Hal_Rd16(host,REG_CMD_READ)) & 4095;
	retval = (FT_CMD_FIFO_SIZE - 4) - fullness;
	return (retval);
#endif
}

ft_void_t Ft_Gpu_Hal_WrCmdBuf(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		if (length > MAX_CMD_FIFO_TRANSFER){
		    length = MAX_CMD_FIFO_TRANSFER;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);
#ifdef FT900_PLATFORM
        dqspiWritePool(SPIM, buffer, length,DQSPI_M_DISCARD_INCOMING_DATA);
        buffer += length;
#endif
#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,*buffer);
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;
#endif

#ifdef MSVC_PLATFORM_SPI
		{   
		    SPI_Write(host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
                    length = SizeTransfered;
   		    buffer += SizeTransfered;
		}
#endif

		Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

#if defined (ARDUINO_PLATFORM_SPI) || defined (FT900_PLATFORM)
ft_void_t Ft_Gpu_Hal_WrCmdBufFromFlash(Ft_Gpu_Hal_Context_t *host,FT_PROGMEM ft_prog_uchar8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		if (length > MAX_CMD_FIFO_TRANSFER){
		    length = MAX_CMD_FIFO_TRANSFER;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);


                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,ft_pgm_read_byte_near(buffer));
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;

    	        Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}
#endif


ft_void_t Ft_Gpu_Hal_CheckCmdBuffer(Ft_Gpu_Hal_Context_t *host,ft_uint32_t count)
{
   ft_uint16_t getfreespace;
   do{
        getfreespace = Ft_Gpu_Cmdfifo_Freespace(host);
   }while(getfreespace < count);
}
ft_void_t Ft_Gpu_Hal_WaitCmdfifo_empty(Ft_Gpu_Hal_Context_t *host)
{
#if defined(FT_81X_ENABLE)
	  while(Ft_Gpu_Hal_Rd32(host,REG_CMDB_SPACE) != (FT_CMD_FIFO_SIZE - 4));
	  host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
#else
   while(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE));
   
   host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
#endif

}

ft_void_t Ft_Gpu_Hal_WrCmdBuf_nowait(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint32_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0;   

#define MAX_CMD_FIFO_TRANSFER   Ft_Gpu_Cmdfifo_Freespace(host)  
	do {                
		length = count;
		if (length > MAX_CMD_FIFO_TRANSFER){
		    length = MAX_CMD_FIFO_TRANSFER;
		}
      	        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

                Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length);
#ifdef FT900_PLATFORM
		dqspiWritePool(SPIM, buffer, length,DQSPI_M_DISCARD_INCOMING_DATA);
		buffer += length;


#endif
//#ifdef ARDUINO_PLATFORM_SPI
#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
                SizeTransfered = 0;
		while (length--) {
                    Ft_Gpu_Hal_Transfer8(host,*buffer);
		    buffer++;
                    SizeTransfered ++;
		}
                length = SizeTransfered;
#endif

#ifdef MSVC_PLATFORM_SPI
		{   
		    SPI_Write(host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
                    length = SizeTransfered;
   		    buffer += SizeTransfered;
		}
#endif

		Ft_Gpu_Hal_EndTransfer(host);
		Ft_Gpu_Hal_Updatecmdfifo(host,length);

	//	Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

ft_uint8_t Ft_Gpu_Hal_WaitCmdfifo_empty_status(Ft_Gpu_Hal_Context_t *host)
{
#if defined(FT_81X_ENABLE)
	if(Ft_Gpu_Hal_Rd32(host,REG_CMDB_SPACE) != (FT_CMD_FIFO_SIZE - 4)){
		return 0;
	}
	else{
	     host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
	     return 1;
	}
#else
   if(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE))
   {
     return 0;
   }
   else
   {
     host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
     return 1;
   }
#endif
}

ft_void_t Ft_Gpu_Hal_WaitLogo_Finish(Ft_Gpu_Hal_Context_t *host)
{
    ft_int16_t cmdrdptr,cmdwrptr;

    do{
         cmdrdptr = Ft_Gpu_Hal_Rd16(host,REG_CMD_READ);
         cmdwrptr = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
    }while ((cmdwrptr != cmdrdptr) || (cmdrdptr != 0));
    host->ft_cmd_fifo_wp = 0;
}


ft_void_t Ft_Gpu_Hal_ResetCmdFifo(Ft_Gpu_Hal_Context_t *host)
{
   host->ft_cmd_fifo_wp = 0;
}


ft_void_t Ft_Gpu_Hal_WrCmd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t cmd)
{
         Ft_Gpu_Hal_CheckCmdBuffer(host,sizeof(cmd));
      
         Ft_Gpu_Hal_Wr32(host,RAM_CMD + host->ft_cmd_fifo_wp,cmd);
      
         Ft_Gpu_Hal_Updatecmdfifo(host,sizeof(cmd));
}


ft_void_t Ft_Gpu_Hal_ResetDLBuffer(Ft_Gpu_Hal_Context_t *host)
{
           host->ft_dl_buff_wp = 0;
}
/* Toggle PD_N pin of FT800 board for a power cycle*/
ft_void_t Ft_Gpu_Hal_Powercycle(Ft_Gpu_Hal_Context_t *host, ft_bool_t up)
{
	if (up)
	{
#ifdef MSVC_PLATFORM
            FT_WriteGPIO(host->hal_handle, 0xBB, 0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
            Ft_Gpu_Hal_Sleep(20);

            FT_WriteGPIO(host->hal_handle, 0xBB, 0x88);//PDN set to 1
            Ft_Gpu_Hal_Sleep(20);
#endif
#ifdef ARDUINO_PLATFORM      
            digitalWrite(FT800_PD_N, LOW);
            Ft_Gpu_Hal_Sleep(20);

            digitalWrite(FT800_PD_N, HIGH);
            Ft_Gpu_Hal_Sleep(20);
#endif
#ifdef FT900_PLATFORM
            GPIOWrite(FT800_PD_N, 0);
            ft_delay(20);
            GPIOWrite(FT800_PD_N, 1);
            ft_delay(20);
#endif
	}else
	{
#ifdef MSVC_PLATFORM
	        FT_WriteGPIO(host->hal_handle, 0xBB, 0x88);//PDN set to 1
            Ft_Gpu_Hal_Sleep(20);
            
            FT_WriteGPIO(host->hal_handle, 0xBB, 0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
            Ft_Gpu_Hal_Sleep(20);
#endif
#ifdef ARDUINO_PLATFORM
            digitalWrite(FT800_PD_N, HIGH);
            Ft_Gpu_Hal_Sleep(20);
            
            digitalWrite(FT800_PD_N, LOW);
            Ft_Gpu_Hal_Sleep(20);
#endif
#ifdef FT900_PLATFORM
            GPIOWrite(FT800_PD_N, 1);
            ft_delay(20);
            GPIOWrite(FT800_PD_N, 0);
            ft_delay(20);
#endif

	}
}
ft_void_t Ft_Gpu_Hal_WrMemFromFlash(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_prog_uchar8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;      

	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);

#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU) || defined(FT900_PLATFORM)
	while (length--) {
            Ft_Gpu_Hal_Transfer8(host,ft_pgm_read_byte_near(buffer));
	    buffer++;
	}
#endif

#ifdef MSVC_PLATFORM_SPI
	{
	    SPI_Write((FT_HANDLE)host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
	}
#endif


	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_Hal_WrMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_uint8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;      

	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
#ifdef FT900_PLATFORM
	dqspiWritePool(SPIM,buffer, length,DQSPI_M_DISCARD_INCOMING_DATA);

#endif
#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
	while (length--) {
            Ft_Gpu_Hal_Transfer8(host,*buffer);
	    buffer++;
	}
#endif

#ifdef MSVC_PLATFORM_SPI
	{
	    SPI_Write((FT_HANDLE)host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
	}
#endif

	Ft_Gpu_Hal_EndTransfer(host);
}


ft_void_t Ft_Gpu_Hal_RdMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;      

	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
#ifdef FT900_PLATFORM
	unsigned char spiData[2] = {0};
	dqspiReadPool(SPIM, spiData, host->spinumdummy,DQSPI_M_SEND_DUMMY_AND_READ);//read out the dummy byte

	dqspiReadPool(SPIM, buffer, length,DQSPI_M_SEND_DUMMY_AND_READ);

#endif
#if defined(ARDUINO_PLATFORM_SPI) || defined(MSVC_FT800EMU)
	while (length--) {
	   *buffer = Ft_Gpu_Hal_Transfer8(host,0);
	   buffer++;
	}
#endif

#ifdef MSVC_PLATFORM_SPI
	{
	   SPI_Read((FT_HANDLE)host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
	}
#endif

	Ft_Gpu_Hal_EndTransfer(host);
}

ft_int32_t Ft_Gpu_Hal_Dec2Ascii(ft_char8_t *pSrc,ft_int32_t value)
{
	ft_int16_t Length;
	ft_char8_t *pdst,charval;
	ft_int32_t CurrVal = value,tmpval,i;
	ft_char8_t tmparray[16],idx = 0;

	Length = strlen(pSrc);
	pdst = pSrc + Length;

	if(0 == value)
	{
		*pdst++ = '0';
		*pdst++ = '\0';
		return 0;
	}

	if(CurrVal < 0)
	{
		*pdst++ = '-';
		CurrVal = - CurrVal;
	}
	/* insert the value */
	while(CurrVal > 0){
		tmpval = CurrVal;
		CurrVal /= 10;
		tmpval = tmpval - CurrVal*10;
		charval = '0' + tmpval;
		tmparray[idx++] = charval;
	}

	for(i=0;i<idx;i++)
	{
		*pdst++ = tmparray[idx - i - 1];
	}
	*pdst++ = '\0';

	return 0;
}



ft_void_t Ft_Gpu_Hal_Sleep(ft_uint16_t ms)
{
#ifdef FT900_PLATFORM
	usleep(ms*1000);
#endif
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
	Sleep(ms);
#endif
#ifdef ARDUINO_PLATFORM
	delay(ms);
#endif
}
#ifdef FT_81X_ENABLE
ft_int16_t Ft_Gpu_Hal_SetSPI(Ft_Gpu_Hal_Context_t *host,FT_GPU_SPI_NUMCHANNELS_T numchnls,FT_GPU_SPI_NUMDUMMYBYTES numdummy)
{
	ft_uint8_t writebyte = 0;
	/* error check */
	if((numchnls > FT_GPU_SPI_QUAD_CHANNEL) || (numdummy > FT_GPU_SPI_TWODUMMY) || (numdummy < FT_GPU_SPI_ONEDUMMY))
	{
		return -1;//error
	}

	host->spichannel = numchnls;
	writebyte = host->spichannel;
	host->spinumdummy = numdummy;

	if(FT_GPU_SPI_TWODUMMY == host->spinumdummy)
	{
		writebyte |= FT_SPI_TWO_DUMMY_BYTE;
	}
	Ft_Gpu_Hal_Wr8(host,REG_SPI_WIDTH,writebyte);
	/* set the parameters in hal context and also set into ft81x */
	return 0;
}
#endif
/* FIFO related apis */
//Init all the parameters of fifo buffer
ft_void_t Ft_Fifo_Init(Ft_Fifo_t *pFifo,ft_uint32_t StartAddress,ft_uint32_t Length,ft_uint32_t HWReadRegAddress,ft_uint32_t HWWriteRegAddress)
{
	/* update the context parameters */
	pFifo->fifo_buff = StartAddress;
	pFifo->fifo_len = Length;
	pFifo->fifo_rp = pFifo->fifo_wp = 0;

	/* update the hardware register addresses - specific to FT800 series chips */
	pFifo->HW_Read_Reg = HWReadRegAddress;
	pFifo->HW_Write_Reg = HWWriteRegAddress;
}

//update both the read and write pointers
ft_void_t Ft_Fifo_Update(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo)
{
	pFifo->fifo_rp = Ft_Gpu_Hal_Rd32(host,pFifo->HW_Read_Reg);
	//Ft_Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);
}

//just write and update the write register
ft_uint32_t Ft_Fifo_Write(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint8_t *buffer,ft_uint32_t NumbytetoWrite)
{
	ft_uint32_t FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo),TotalBytes = NumbytetoWrite;

	if(NumbytetoWrite > FreeSpace)
	{
		/* update the read pointer and get the free space */
		Ft_Fifo_Update(host,pFifo);
		FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);

		if(NumbytetoWrite > FreeSpace)
		{
			TotalBytes = FreeSpace;
		}
	}

	/* sanity check */
	if(TotalBytes <= 0)
	{
		printf("no space in fifo write %d %d %d %d\n",TotalBytes,FreeSpace,pFifo->fifo_wp,pFifo->fifo_rp);
		return 0;//error condition

	}
	/* check for the loopback conditions */
	if(pFifo->fifo_wp + TotalBytes >= pFifo->fifo_len)
	{
		ft_uint32_t partialchunk = pFifo->fifo_len - pFifo->fifo_wp,secpartialchunk = TotalBytes - partialchunk;

		Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,partialchunk);
		if(secpartialchunk > 0)
		{
			Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff,buffer + partialchunk,secpartialchunk);
		}
		pFifo->fifo_wp = secpartialchunk;
		//printf("partial chunks %d %d %d %d\n",partialchunk,secpartialchunk,pFifo->fifo_wp,pFifo->fifo_rp);

	}
	else
	{
		Ft_Gpu_Hal_WrMem(host,pFifo->fifo_buff + pFifo->fifo_wp,buffer,TotalBytes);
		pFifo->fifo_wp += TotalBytes;
	}

	/* update the write pointer address in write register */
	Ft_Gpu_Hal_Wr32(host,pFifo->HW_Write_Reg,pFifo->fifo_wp);

	return TotalBytes;
}
//just write one word and update the write register
ft_void_t Ft_Fifo_Write32(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint32_t WriteWord)
{
	Ft_Fifo_Write(host,pFifo,(ft_uint8_t *)&WriteWord,4);
}
//write and wait for the fifo to be empty. handle cases even if the Numbytes are more than freespace
ft_void_t Ft_Fifo_WriteWait(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo,ft_uint8_t *buffer,ft_uint32_t Numbyte)
{
	ft_uint32_t TotalBytes = Numbyte,currchunk = 0,FreeSpace;
	ft_uint8_t *pbuff = buffer;
	/* blocking call, manage to check for the error case and break in case of error */
	while(TotalBytes > 0)
	{
		currchunk = TotalBytes;
		FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);

		//printf("CMDB_SPACE: %d , CMDB_WRITE: %d , CMD_READ: %d , CMD_WRITE: %d\n",Ft_Gpu_Hal_Rd32(host,REG_CMDB_SPACE),Ft_Gpu_Hal_Rd32(host,REG_CMDB_WRITE), Ft_Gpu_Hal_Rd16(host,REG_CMD_READ), Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE));
		//printf("ring fifo rd wr %x %x %x %x %x %x\n",Ft_Gpu_Hal_Rd32(host,REG_MEDIAFIFO_READ),Ft_Gpu_Hal_Rd32(host,REG_MEDIAFIFO_WRITE),pFifo->fifo_rp,pFifo->fifo_wp,Ft_Gpu_Hal_Rd32(host,REG_CMD_READ),Ft_Gpu_Hal_Rd32(host,REG_CMD_WRITE));
		/*
		while(FreeSpace==0){
			FreeSpace = Ft_Fifo_GetFreeSpace(host,pFifo);
			ft_delay(5000);
		}
		*/

		if(currchunk > FreeSpace)
		{
			currchunk = FreeSpace;
		}

		Ft_Fifo_Write(host,pFifo,pbuff,currchunk);
		//printf("CMDB_SPACE: %d , CMDB_WRITE: %d , CMD_READ: %d , CMD_WRITE: %d\n",Ft_Gpu_Hal_Rd32(host,REG_CMDB_SPACE),Ft_Gpu_Hal_Rd32(host,REG_CMDB_WRITE), Ft_Gpu_Hal_Rd16(host,REG_CMD_READ), Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE));
		pbuff += currchunk;
		TotalBytes -= currchunk;
		/* does it makes sense to have a delay for the engine to consume? */
		ft_delay(5);//hack - remove this at later point of time
	}
}

#if defined(FT900_PLATFORM)
ft_void_t getFlashTextString(char __flash__ *str, ft_uchar8_t *destArray, ft_uint16_t numOfChars){
		ft_uint16_t i;
		for(i=0;i<numOfChars;i++)
			destArray[i] = str[i];
}
#endif

//get the free space in the fifo - make sure the return value is maximum of (LENGTH - 4)
ft_uint32_t Ft_Fifo_GetFreeSpace(Ft_Gpu_Hal_Context_t *host,Ft_Fifo_t *pFifo)
{

/*
  ft_uint32_t FreeSpace = Ft_Gpu_Hal_Rd32(host,REG_CMDB_SPACE);

 */
	ft_uint32_t FreeSpace = 0;

	Ft_Fifo_Update(host,pFifo);

	if(pFifo->fifo_wp >= pFifo->fifo_rp)
	{
		FreeSpace = pFifo->fifo_len - pFifo->fifo_wp + pFifo->fifo_rp;
	}
	else
	{
		FreeSpace = pFifo->fifo_rp - pFifo->fifo_wp;
	}

	if(FreeSpace >= 4)
	{
		FreeSpace -= 4;//make sure 1 word space is maintained between rd and wr pointers
	}
	return FreeSpace;
}


