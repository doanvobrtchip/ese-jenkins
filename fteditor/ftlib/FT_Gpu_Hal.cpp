#include "FT_DataTypes.h"
#include "FT_Gpu_Hal.h"
//#include "ft800emu_vc.h"
#include "vc2.h"
#include "libMPSSE_spi.h"
#include <stdio.h>



#define FT_DL_SIZE           (8*1024)  //8KB Display List buffer size
#define FT_CMD_FIFO_SIZE     (4*1024)  //4KB coprocessor Fifo size
#define FT_CMD_SIZE          (4)       //4 byte per coprocessor command of EVE


ft_bool_t    Ft_Gpu_Hal_Open_MPSSEDev(Ft_Gpu_Hal_Context_t *host)
{
//#if defined(MSVC_PLATFORM_SPI_LIBMPSSE)
    ChannelConfig channelConf;			//channel configuration
    FT_STATUS status;

    Init_libMPSSE();

    /* configure the spi settings */
    channelConf.ClockRate = host->hal_config.spi_clockrate_khz * 1000;
    channelConf.LatencyTimer= 2;

    channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channelConf.Pin = 0x00000000;	/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

    /* Open the first available channel */
    SPI_OpenChannel(host->hal_config.channel_no,(FT_HANDLE *)&host->hal_handle);
    status = SPI_InitChannel((FT_HANDLE)host->hal_handle,&channelConf);
    //printf("\nhandle=0x%x status=0x%x\n",host->hal_handle,status);

    host->spichannel = 0;
    return true;
//#endif
}

ft_bool_t    Ft_Gpu_Hal_Open_FT4222Dev(Ft_Gpu_Hal_Context_t *host)
{
    ft_bool_t ret = TRUE;

    FT_STATUS status;

    FT_HANDLE fthandle;
    FT4222_Version pversion;
    FT4222_ClockRate ftclk = SYS_CLK_80;

    FT4222_ClockRate selclk = SYS_CLK_80;
    FT4222_SPIClock seldiv = CLK_DIV_4;

    /* GPIO0		 , GPIO1	  , GPIO2	   , GPIO3		 } */
    GPIO_Dir gpio_dir[4] = { GPIO_OUTPUT , GPIO_INPUT , GPIO_INPUT, GPIO_INPUT };

    /* obtain handle */
    status = FT_OpenEx("FT4222 A", FT_OPEN_BY_DESCRIPTION, &fthandle);
    if (status != FT_OK)
    {
        printf("FT_OpenEx failed %d\n", status);
        ret = FALSE;
    }
    else
    {
        host->hal_handle = fthandle; //first interface is SPI
    }


    /* obtain handle */
    status = FT_OpenEx("FT4222 B", FT_OPEN_BY_DESCRIPTION, &fthandle);
    if (status != FT_OK)
    {
        printf("FT_OpenEx failed %d\n", status);
        ret = FALSE;
    }
    else
    {
        host->hal_handle2 = fthandle; //second interface is GPIO
    }

    if (ret)
    {
        status = FT4222_GetVersion(host->hal_handle, &pversion);
        if (status != FT4222_OK)
            printf("FT4222_GetVersion failed\n");
        else
            printf("SPI:chipversion = 0x%x\t dllversion = 0x%x\n", pversion.chipVersion, pversion.dllVersion);
    }

    if (ret)
    {
        //Set default Read timeout 5s and Write timeout 5sec
        status = FT_SetTimeouts(host->hal_handle, FT4222_ReadTimeout, FT4222_WriteTimeout);
        if (FT_OK != status)
        {
            printf("FT_SetTimeouts failed!\n");
            ret = FALSE;
        }
    }

    if (ret)
    {
        // no latency to usb
        status = FT_SetLatencyTimer(host->hal_handle, FT4222_LatencyTime);
        if (FT_OK != status)
        {
            printf("FT_SetLatencyTimerfailed!\n");
            ret = FALSE;
        }
    }

    if (ret)
    {
        if (!Ft_Gpu_Hal_FT4222_ComputeCLK(host, &selclk, &seldiv))
        {
            printf("Requested clock %d KHz is not supported in FT4222 \n", host->hal_config.spi_clockrate_khz);
            ret = FALSE;
        }
    }

    if(ret)
    {
        status = FT4222_SetClock(host->hal_handle, selclk);
        if (FT_OK != status)
        {
            printf("FT4222_SetClock!\n");
            ret = FALSE;
        }

        status = FT4222_GetClock(host->hal_handle, &ftclk);

        if (FT_OK != status)
            printf("FT4222_SetClock failed\n");
        else
            printf("FT4222 clk = %d\n", ftclk);

    }

    if(ret)
    {
        /* Interface 1 is SPI master */
        status = FT4222_SPIMaster_Init(
                                        host->hal_handle,
                                        SPI_IO_SINGLE,
                                        seldiv,
                                        CLK_IDLE_LOW, //,CLK_IDLE_HIGH
                                        CLK_LEADING,// CLK_LEADING CLK_TRAILING
                                        0x01       //ssoMap,Slave Selection output pin sso0
                                      );	/* slave selection output pins */
        if (FT_OK != status)
        {
            printf("Init FT4222 as SPI master device failed!\n");
            ret = FALSE;
        }
        else
            host->spichannel = FT_GPU_SPI_SINGLE_CHANNEL; //SPI_IO_SINGLE;

        status = FT4222_SPI_SetDrivingStrength(host->hal_handle, DS_16MA, DS_16MA, DS_16MA);
        if (FT4222_OK != status)
            printf("FT4222_SPI_SetDrivingStrength failed!\n");

        Ft_Gpu_Hal_Sleep(20);

        status = FT4222_SetSuspendOut(host->hal_handle2, FALSE);
        if (FT_OK != status)
        {
            printf("Disable suspend out function on GPIO2 failed!\n");
            ret = FALSE;
        }

        status = FT4222_SetWakeUpInterrupt(host->hal_handle2, FALSE);
        if (FT_OK != status)
        {
            printf("Disable wakeup/interrupt feature on GPIO3 failed!\n");
            ret = FALSE;
        }
        /* Interface 2 is GPIO */
        status = FT4222_GPIO_Init(host->hal_handle2, gpio_dir);
        if (FT_OK != status)
        {
            printf("Init FT4222 as GPIO interface failed!\n");
            ret = FALSE;
        }
    }

    /* dedicated write buffer used for SPI write. Max size is 2^uint16 */
    host->spiwrbuf_ptr = (ft_uint8_t *)malloc(FT4222_DYNAMIC_ALLOCATE_SIZE);


    host->ft_cmd_fifo_wp = host->ft_dl_buff_wp = 0;
    host->spinumdummy = FT_GPU_SPI_ONEDUMMY; //by default ft800/801/810/811 goes with single dummy byte for read
    host->status = FT_GPU_HAL_OPENED;

    return ret;
}

ft_bool_t    Ft_Gpu_Hal_Open(Ft_Gpu_Hal_Context_t *host)
{
    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
        return Ft_Gpu_Hal_Open_MPSSEDev(host);

    if (host->spi_host == SPIHOST_FT4222_SPI)
        return Ft_Gpu_Hal_Open_FT4222Dev(host);

    return FALSE;
}


ft_void_t  Ft_Gpu_Hal_Close(Ft_Gpu_Hal_Context_t *host)
{
    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        SPI_CloseChannel(host->hal_handle);
        Cleanup_libMPSSE();
    }

    if (host->spi_host == SPIHOST_FT4222_SPI)
    {
        FT_STATUS ftstatus;
        FT4222_STATUS status;

        free(host->spiwrbuf_ptr);

        if (FT4222_OK != (status = FT4222_UnInitialize(host->hal_handle)))
            printf("FT4222_UnInitialize failed %d \n",status);

        if (FT4222_OK != (status = FT4222_UnInitialize(host->hal_handle2)))
            printf("FT4222_UnInitialize failed %d \n", status);

        if(FT_OK != (ftstatus = FT_Close(host->hal_handle)))
            printf("FT_CLOSE failed %d \n", ftstatus);

        if (FT_OK != (ftstatus = FT_Close(host->hal_handle2)))
            printf("FT_CLOSE failed %d \n", ftstatus);
    }
    host->status = FT_GPU_HAL_CLOSED;
}



/*The APIs for reading/writing transfer continuously only with small buffer system*/
ft_void_t  Ft_Gpu_Hal_StartTransfer(Ft_Gpu_Hal_Context_t *host,FT_GPU_TRANSFERDIR_T rw,ft_uint32_t addr)
{
	if (FT_GPU_READ == rw){

        if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI){
            ft_uint8_t Transfer_Array[4];
            ft_uint32_t SizeTransfered;

            /* Compose the read packet */
            Transfer_Array[0] = addr >> 16;
            Transfer_Array[1] = addr >> 8;
            Transfer_Array[2] = addr;

            Transfer_Array[3] = 0; //Dummy Read byte
            SPI_Write((FT_HANDLE)host->hal_handle,Transfer_Array,
                      sizeof(Transfer_Array),(uint32*)&SizeTransfered,
                      SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
        }

		host->status = FT_GPU_HAL_READING;
	}else{
        if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI){

            ft_uint8_t Transfer_Array[3];
            ft_uint32_t SizeTransfered;

            /* Compose the read packet */
            Transfer_Array[0] = (0x80 | (addr >> 16));
            Transfer_Array[1] = addr >> 8;
            Transfer_Array[2] = addr;
            FT_STATUS status = SPI_Write((FT_HANDLE)host->hal_handle,Transfer_Array,
                      3,(uint32*)&SizeTransfered,
                      SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);

            printf("status is %d\n",status);
        }
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
    size_t length = strlen(string);
    while(length --){
       Ft_Gpu_Hal_Transfer8(host,*string);
       string ++;
    }
    //Append one null as ending flag
    Ft_Gpu_Hal_Transfer8(host,0);
	return 0;
}


ft_uint8_t    Ft_Gpu_Hal_Transfer8(Ft_Gpu_Hal_Context_t *host,ft_uint8_t value)
{
    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI){
        ft_uint32_t SizeTransfered;
        if (host->status == FT_GPU_HAL_WRITING){
            SPI_Write(host->hal_handle,&value,sizeof(value),(uint32*)&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
        }else{
            SPI_Read(host->hal_handle,&value,sizeof(value),(uint32*)&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
        }

        if (SizeTransfered != sizeof(value))
            host->status = FT_GPU_HAL_STATUS_ERROR;
        return value;
    }

	return 0;
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
extern "C" void SPI_ToggleCS(FT_HANDLE handle, BOOL high);
ft_void_t   Ft_Gpu_Hal_EndTransfer(Ft_Gpu_Hal_Context_t *host)
{
    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        SPI_ToggleCS((FT_HANDLE)host->hal_handle,FALSE);
    }

	host->status = FT_GPU_HAL_OPENED;
}


ft_uint8_t  Ft_Gpu_Hal_Rd8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint8_t value = 0;


    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
        value = Ft_Gpu_Hal_Transfer8(host,0);
		Ft_Gpu_Hal_EndTransfer(host);
    }

    if (host->spi_host == SPIHOST_FT4222_SPI)
    {
        if (!Ft_Gpu_Hal_FT4222_Rd(host, addr, &value, sizeof(value)))
        {
            printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
        }
    }


	return value;
}
ft_uint16_t Ft_Gpu_Hal_Rd16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint16_t value=0;


    if (host->spi_host == SPIHOST_FT4222_SPI){
	   if (!Ft_Gpu_Hal_FT4222_Rd(host, addr, (ft_uint8_t *)&value, sizeof(value)))
        {
              printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
        value = Ft_Gpu_Hal_Transfer16(host,0);
		Ft_Gpu_Hal_EndTransfer(host);
    }


	return value;
}
ft_uint32_t Ft_Gpu_Hal_Rd32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr)
{
	ft_uint32_t value = 0;
	if (host->spi_host == SPIHOST_FT4222_SPI){
        if (!Ft_Gpu_Hal_FT4222_Rd(host, addr, (ft_uint8_t *)&value, sizeof(value)))
        {
            printf("Ft_Gpu_Hal_FT4222_Rd failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI){
		Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);
        value = Ft_Gpu_Hal_Transfer32(host,0);
		
     	Ft_Gpu_Hal_EndTransfer(host);
    }

	return value;
}

ft_void_t Ft_Gpu_Hal_Wr8(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t v)
{

    if (host->spi_host == SPIHOST_FT4222_SPI){
        if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, &v, sizeof(v)))
        {
            printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI){
        Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
        Ft_Gpu_Hal_Transfer8(host,v);
        Ft_Gpu_Hal_EndTransfer(host);
    }
}
ft_void_t Ft_Gpu_Hal_Wr16(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint16_t v)
{

    if (host->spi_host == SPIHOST_FT4222_SPI){
        ft_uint8_t v_arr[2] = {0,0};
        v_arr[0] = v & 0xFF ; //LSB first in the array
        v_arr[1] = (v >> 8) & 0xFF;

        if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, v_arr, sizeof(v_arr)))
        {
            printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
        Ft_Gpu_Hal_Transfer16(host,v);
        Ft_Gpu_Hal_EndTransfer(host);
    }

}
ft_void_t Ft_Gpu_Hal_Wr32(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint32_t v)
{
    if (host->spi_host == SPIHOST_FT4222_SPI)
    {
        ft_uint8_t v_arr[4] = { 0 };
        v_arr[0] = v & 0xFF; //LSB first in the array
        v_arr[1] = (v >> 8) & 0xFF;
        v_arr[2] = (v >> 16) & 0xFF;
        v_arr[3] = (v >> 24) & 0xFF;

        if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, v_arr, sizeof(v_arr)))
        {
            printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
        Ft_Gpu_Hal_Transfer32(host,v);
        Ft_Gpu_Hal_EndTransfer(host);
    }
}

ft_void_t Ft_Gpu_HostCommand(Ft_Gpu_Hal_Context_t *host,ft_uint8_t cmd)
{
    ft_uint8_t Transfer_Array[3];
    ft_uint32_t SizeTransfered;
    ft_uint8_t dummy_read;

    Transfer_Array[0] = cmd;
    Transfer_Array[1] = 0;
    Transfer_Array[2] = 0;


    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        SPI_Write(host->hal_handle,Transfer_Array,sizeof(Transfer_Array),(uint32*)&SizeTransfered,
                  SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    }
    if (host->spi_host == SPIHOST_FT4222_SPI)
    {
        FT4222_STATUS status;
        switch(host->spichannel)
        {
            case FT_GPU_SPI_SINGLE_CHANNEL:
                /* FYI : All HOST CMDs should only be executed in single channel mode
                */
                status = FT4222_SPIMaster_SingleWrite(
                                                        host->hal_handle,
                                                        Transfer_Array,
                                                        sizeof(Transfer_Array),
                                                        (ft_uint16_t *)&SizeTransfered,
                                                        TRUE
                                                     );
                if (FT4222_OK != status)
                    printf("SPI write failed = %d\n",status);
            break;
            case FT_GPU_SPI_DUAL_CHANNEL:
            case FT_GPU_SPI_QUAD_CHANNEL:
                /* only reset command among host commands can be executed in multi channel mode*/
                status = FT4222_SPIMaster_MultiReadWrite(
                                                            host->hal_handle,
                                                            &dummy_read,
                                                            Transfer_Array,
                                                            0,
                                                            sizeof(Transfer_Array),
                                                            0,
                                                            &SizeTransfered
                                                        );
                if (FT4222_OK != status)
                    printf("SPI write failed = %d\n", status);
            break;
            default:
                printf("No transfer\n");
        }
    }
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
   Ft_Gpu_HostCommand(host,0x68);
}


ft_void_t Ft_Gpu_Hal_Updatecmdfifo(Ft_Gpu_Hal_Context_t *host,ft_uint16_t count)
{
	host->ft_cmd_fifo_wp  = (host->ft_cmd_fifo_wp + count) & 4095;

	//4 byte alignment
	host->ft_cmd_fifo_wp = (host->ft_cmd_fifo_wp + 3) & 0xffc;
	Ft_Gpu_Hal_Wr16(host,REG_CMD_WRITE,host->ft_cmd_fifo_wp);
}


ft_uint16_t Ft_Gpu_Cmdfifo_Freespace(Ft_Gpu_Hal_Context_t *host)
{
	ft_uint16_t fullness,retval;

	fullness = (host->ft_cmd_fifo_wp - Ft_Gpu_Hal_Rd16(host,REG_CMD_READ)) & 4095;
	retval = (FT_CMD_FIFO_SIZE - 4) - fullness;
	return (retval);
}

ft_void_t Ft_Gpu_Hal_WrCmdBuf(Ft_Gpu_Hal_Context_t *host,ft_uint8_t *buffer,ft_uint16_t count)
{
	ft_uint32_t length =0, SizeTransfered = 0,availablefreesize;
    

	do 
	{                
		length = count;
        availablefreesize = Ft_Gpu_Cmdfifo_Freespace(host);

		if (length > availablefreesize)
		{
		    length = availablefreesize;
		}

        Ft_Gpu_Hal_CheckCmdBuffer(host,length);

        if (host->spi_host == SPIHOST_FT4222_SPI)
        {
            FT4222_STATUS status;
            ft_uint8_t * wrpktptr;
            ft_uint8_t dummy_read;
            wrpktptr = host->spiwrbuf_ptr;  //Using global buf , FT4222_DYNAMIC_ALLOCATE_SIZE
            *(wrpktptr + 0) = (ft_uint8_t) ((host->ft_cmd_fifo_wp + RAM_CMD) >> 16) | 0x80;
            *(wrpktptr + 1) = (ft_uint8_t) ((host->ft_cmd_fifo_wp + RAM_CMD) >> 8);
            *(wrpktptr + 2) = (ft_uint8_t) (host->ft_cmd_fifo_wp + RAM_CMD) & 0xff;
            memcpy((wrpktptr + 3), buffer, length);

            //Ft_Gpu_Hal_SlaveSelect(host, TRUE);
            //Ft_Gpu_Hal_SlaveSelect(host, TRUE);

            if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
            {
                status = FT4222_SPIMaster_SingleWrite(
                    host->hal_handle,
                    wrpktptr,
                    (length + 3), //3 for RAM_CMD address
                    (ft_uint16_t *) &SizeTransfered,
                    TRUE
                    );
                if ((FT4222_OK != status) || (SizeTransfered != (length + 3)))
                {
                    printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, SizeTransfered, status);
                    break;
                }
            }
            else
            {	/* DUAL and QAUD */
                status = FT4222_SPIMaster_MultiReadWrite(
                    host->hal_handle,
                    &dummy_read,
                    wrpktptr,
                    0,
                    (length + 3),
                    0,
                    &SizeTransfered
                    );
            }

            //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
            //Ft_Gpu_Hal_SlaveSelect(host, FALSE);

            buffer += length;
        }


        if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
        {
		    Ft_Gpu_Hal_StartCmdTransfer(host,FT_GPU_WRITE,length); 
			    {   
				    SPI_Write(host->hal_handle,buffer,length,&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
				    length = SizeTransfered;
   				    buffer += SizeTransfered;
			    }		

            Ft_Gpu_Hal_EndTransfer(host);
        }

		Ft_Gpu_Hal_Updatecmdfifo(host,length);

		Ft_Gpu_Hal_WaitCmdfifo_empty(host);

		count -= length;
	}while (count > 0);
}

#ifdef ARDUINO_PLATFORM_SPI
ft_void_t Ft_Gpu_Hal_WrCmdBufFromFlash(Ft_Gpu_Hal_Context_t *host,FT_PROGMEM ft_prog_uchar8_t *buffer,ft_uint16_t count)
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


ft_void_t Ft_Gpu_Hal_CheckCmdBuffer(Ft_Gpu_Hal_Context_t *host,ft_uint16_t count)
{
   ft_uint16_t getfreespace;
   do{
        getfreespace = Ft_Gpu_Cmdfifo_Freespace(host);
   }while(getfreespace < count);
}
ft_void_t Ft_Gpu_Hal_WaitCmdfifo_empty(Ft_Gpu_Hal_Context_t *host)
{
   while(Ft_Gpu_Hal_Rd16(host,REG_CMD_READ) != Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE));

   host->ft_cmd_fifo_wp = Ft_Gpu_Hal_Rd16(host,REG_CMD_WRITE);
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
           if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
           {
               FT_WriteGPIO(host->hal_handle, 0xBB, 0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
               Ft_Gpu_Hal_Sleep(20);

               FT_WriteGPIO(host->hal_handle, 0xBB, 0x88);//PDN set to 1
               Ft_Gpu_Hal_Sleep(20);
           }
           if (host->spi_host == SPIHOST_FT4222_SPI)
           {
               FT4222_STATUS status = FT4222_OTHER_ERROR;

               if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, (GPIO_Port)host->hal_config.pdn_pin_no, 0)))
                   printf("FT4222_GPIO_Write error = %d\n",status);
               Ft_Gpu_Hal_Sleep(20);


               if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, (GPIO_Port)host->hal_config.pdn_pin_no, 1)))
                   printf("FT4222_GPIO_Write error = %d\n", status);
               Ft_Gpu_Hal_Sleep(20);
           }

     }else
     {
            if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
            {
                FT_WriteGPIO(host->hal_handle, 0xBB, 0x88);//PDN set to 1
                Ft_Gpu_Hal_Sleep(20);

                FT_WriteGPIO(host->hal_handle, 0xBB, 0x08);//PDN set to 0 ,connect BLUE wire of MPSSE to PDN# of FT800 board
                Ft_Gpu_Hal_Sleep(20);
            }
            if (host->spi_host == SPIHOST_FT4222_SPI)
            {
                FT4222_STATUS status = FT4222_OTHER_ERROR;

                if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, (GPIO_Port)host->hal_config.pdn_pin_no, 1)))
                    printf("FT4222_GPIO_Write error = %d\n", status);
                Ft_Gpu_Hal_Sleep(20);

                if (FT4222_OK != (status = FT4222_GPIO_Write(host->hal_handle2, (GPIO_Port)host->hal_config.pdn_pin_no, 0)))
                    printf("FT4222_GPIO_Write error = %d\n", status);
                Ft_Gpu_Hal_Sleep(20);
            }

     }
}
ft_void_t Ft_Gpu_Hal_WrMemFromFlash(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_prog_uchar8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;

	Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);


	{
        if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
             SPI_Write((FT_HANDLE)host->hal_handle,(uint8*)buffer,length,(uint32*)&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
        if (host->spi_host == SPIHOST_FT4222_SPI)
			Ft_Gpu_Hal_FT4222_Wr(host, addr, buffer, length);		
	}



	Ft_Gpu_Hal_EndTransfer(host);
}

ft_void_t Ft_Gpu_Hal_WrMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr,const ft_uint8_t *buffer, ft_uint32_t length)
{
    if (host->spi_host == SPIHOST_FT4222_SPI)
    {
        if (!Ft_Gpu_Hal_FT4222_Wr(host, addr, buffer, length))
        {
            printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        ft_uint32_t SizeTransfered = 0;
        Ft_Gpu_Hal_StartTransfer(host,FT_GPU_WRITE,addr);
        while (length > 0)
        {
            ft_uint32_t bytes2send = length;
            if (length > 16*1024)
                bytes2send = 16*1024;
            SPI_Write((FT_HANDLE)host->hal_handle,(uint8*)buffer,bytes2send,(uint32*)&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
            length -= SizeTransfered;
            buffer += SizeTransfered;
        }
        Ft_Gpu_Hal_EndTransfer(host);
    }
}


ft_void_t Ft_Gpu_Hal_RdMem(Ft_Gpu_Hal_Context_t *host,ft_uint32_t addr, ft_uint8_t *buffer, ft_uint32_t length)
{
	ft_uint32_t SizeTransfered = 0;


    if (host->spi_host == SPIHOST_FT4222_SPI)
    {
        if (!Ft_Gpu_Hal_FT4222_Rd(host, addr, buffer, length))
        {
            printf("Ft_Gpu_Hal_FT4222_Wr failed\n");
        }
    }

    if (host->spi_host == SPIHOST_MPSSE_VA800A_SPI)
    {
        Ft_Gpu_Hal_StartTransfer(host,FT_GPU_READ,addr);


        {
           SPI_Read((FT_HANDLE)host->hal_handle,buffer,length,(uint32*)&SizeTransfered,SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES);
        }


        Ft_Gpu_Hal_EndTransfer(host);
    }
}

ft_int32_t Ft_Gpu_Hal_Dec2Ascii(ft_char8_t *pSrc,ft_int32_t value)
{
    size_t Length;
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
#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
	Sleep(ms);
#endif
#ifdef ARDUINO_PLATFORM
	delay(ms);
#endif
}


/***************************************************************************
* Interface Description    : Function to tranfer HOST MEMORY READ command followed 
*							 by read of data bytes from GPU
* Implementation           : Using FT4222_SPIMaster_SingleRead,
*							 FT4222_SPIMaster_SingleWrite
*							 FT4222_SPIMaster_MultiReadWrite
*							 
* Return Value             : ft_uint8_t
*							 1 - Success
*							 0 - Failure
* Author                   :
****************************************************************************/
ft_uint8_t    Ft_Gpu_Hal_FT4222_Rd(Ft_Gpu_Hal_Context_t *host, ft_uint32_t hrdcmd, ft_uint8_t * rdbufptr, ft_uint32_t exprdbytes)
{
    ft_uint32_t SizeTransfered;    
	FT4222_STATUS status;
	ft_uint8_t hrdpkt[8] = {0,0,0,0,0,0,0,0}; //3 byte addr + 2 or1 byte dummy
	ft_uint8_t retcode = 1;		/* assume successful operation */	
    ft_uint16_t bytes_per_read;    

	if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
	{
        //Ft_Gpu_Hal_SlaveSelect(host, TRUE);        
        //Ft_Gpu_Hal_SlaveSelect(host, TRUE);

		/* Compose the HOST MEMORY READ packet */
		hrdpkt[0] = (ft_uint8_t)(hrdcmd >> 16) & 0xFF;
		hrdpkt[1] = (ft_uint8_t)(hrdcmd >> 8) & 0xFF;
		hrdpkt[2] = (ft_uint8_t)(hrdcmd & 0xFF);		

		status = FT4222_SPIMaster_SingleWrite(
												host->hal_handle,
												hrdpkt,
												3+host->spinumdummy , /* 3 address and chosen dummy bytes */
												(ft_uint16_t *) &SizeTransfered,
												FALSE			/* continue transaction */
											);
		if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != (3+host->spinumdummy) ))
		{
			printf("FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", (ft_uint16_t)SizeTransfered, status);
			retcode = 0;
			if((ft_uint16_t)SizeTransfered != sizeof(hrdpkt))
				host->status = FT_GPU_HAL_STATUS_ERROR;
		}
		else
		{
			/* continue reading data bytes only if HOST MEMORY READ command sent successfully */
			if (rdbufptr != NULL)
			{	
				BOOL disable_cs = FALSE; //assume multi SPI read calls				
                bytes_per_read = exprdbytes;

				while (retcode && exprdbytes)
				{
                    if (exprdbytes <= FT4222_MAX_RD_BYTES_PER_CALL_IN_SINGLE_CH)
                    {                        
                        bytes_per_read = exprdbytes;
                        disable_cs = TRUE; //1 iteration of SPI read adequate
                    }
                    else
                    {
                        bytes_per_read = FT4222_MAX_RD_BYTES_PER_CALL_IN_SINGLE_CH;
                        disable_cs = FALSE;
                    }

					status = FT4222_SPIMaster_SingleRead(
															host->hal_handle,
															rdbufptr,
															bytes_per_read,
															(ft_uint16_t *) &SizeTransfered,
															disable_cs
														);
					if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != bytes_per_read))
					{
						printf("FT4222_SPIMaster_SingleRead failed,SizeTransfered is %d with status %d\n", (ft_uint16_t)SizeTransfered, status);
						retcode = 0;
						if ((ft_uint16_t)SizeTransfered != bytes_per_read)
							host->status = FT_GPU_HAL_STATUS_ERROR;
					}					

					//multiple iterations of SPI read needed
                    bytes_per_read = (ft_uint16_t)SizeTransfered;

					exprdbytes -= bytes_per_read;
					rdbufptr += bytes_per_read;						
										
				}
			}
		}

        //Ft_Gpu_Hal_SlaveSelect(host, FALSE);        
        //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
	}
	else
	{
		/* Multi channel SPI communication */		
		ft_uint32_t t_hrdcmd = hrdcmd;	
        ft_uint32_t read_data_index = 0;
        		
		while (retcode && exprdbytes)
		{
            //Ft_Gpu_Hal_SlaveSelect(host, TRUE);
            //Ft_Gpu_Hal_SlaveSelect(host, TRUE);

			/* Compose the HOST MEMORY READ ADDR packet */
			hrdpkt[0] = (ft_uint8_t)(t_hrdcmd >> 16) & 0xFF;
			hrdpkt[1] = (ft_uint8_t)(t_hrdcmd >> 8) & 0xFF;
			hrdpkt[2] = (ft_uint8_t)(t_hrdcmd & 0xff);			

            if (exprdbytes <= FT4222_MAX_RD_BYTES_PER_CALL_IN_MULTI_CH)
                bytes_per_read = exprdbytes;
            else
                bytes_per_read = FT4222_MAX_RD_BYTES_PER_CALL_IN_MULTI_CH;

			status = FT4222_SPIMaster_MultiReadWrite(
														host->hal_handle,
														rdbufptr + read_data_index,
														hrdpkt,
														0,
														3 + host->spinumdummy , // 3 addr + dummy bytes
														bytes_per_read,
														&SizeTransfered
													);
			if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != bytes_per_read))
			{
				printf("FT4222_SPIMaster_MultiReadWrite failed, SizeTransfered is %d with status %d\n", SizeTransfered, status);
				retcode = 0;
				if ((ft_uint16_t)SizeTransfered != bytes_per_read)
					host->status = FT_GPU_HAL_STATUS_ERROR;
			}

			//its multi SPI read calls
            bytes_per_read = (ft_uint16_t)SizeTransfered;

			exprdbytes -= bytes_per_read;
            read_data_index += bytes_per_read;
			t_hrdcmd += bytes_per_read;	

            //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
            //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
		}
	}	    

	return retcode;
}

/***************************************************************************
* Interface Description    : Function to tranfer HOST MEMORY WRITE command
*
* Implementation           : Uisng FT4222_SPIMaster_SingleWrite
*							 FT4222_SPIMaster_MultiReadWrite	
*
* Return Value             : ft_uint8_t
*							 1 - Success
*							 0 - Failure
* Author                   :
****************************************************************************/
ft_uint8_t    Ft_Gpu_Hal_FT4222_Wr(Ft_Gpu_Hal_Context_t *host, ft_uint32_t hwraddr, const ft_uint8_t * wrbufptr, ft_uint32_t bytestowr)
{

	FT4222_STATUS status;
	ft_uint8_t * temp_wrpktptr;
    ft_uint16_t per_write = 0;
    BOOL disable_cs = FALSE; //assume multi SPI write calls    
    ft_uint8_t dummy_read;
	ft_uint8_t retcode = 1;		/* assume successful operation */		

    temp_wrpktptr = host->spiwrbuf_ptr; //global host write buffer of size FT4222_MAX_BYTES_PER_CALL
    memset(temp_wrpktptr,0, FT4222_DYNAMIC_ALLOCATE_SIZE);
    
	if (host->spichannel == FT_GPU_SPI_SINGLE_CHANNEL)
	{
        ft_uint16_t SizeTransfered;

	    *(temp_wrpktptr + 0) = (hwraddr >> 16) | 0x80; //MSB bits 10 for WRITE
		*(temp_wrpktptr + 1) = (hwraddr >> 8) & 0xFF;
		*(temp_wrpktptr + 2) = hwraddr & 0xff;        

        status = FT4222_SPIMaster_SingleWrite(
                                                host->hal_handle,
                                                temp_wrpktptr,
                                                3, //3 address bytes
                                                (ft_uint16_t *) &SizeTransfered,
                                                FALSE
                                            );

        if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != 3))
        {
            printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, (ft_uint16_t)SizeTransfered, status);
            retcode = 0;           
        }

        if (retcode)
        {            
            while (retcode && bytestowr)
            {
                if (bytestowr <= FT4222_MAX_WR_BYTES_PER_CALL_IN_SINGLE_CH)
                {                    
                    per_write = bytestowr;
                    disable_cs = TRUE;                    
                }
                else
                {
                    per_write = FT4222_MAX_WR_BYTES_PER_CALL_IN_SINGLE_CH;
                    disable_cs = FALSE;
                }

                memcpy(temp_wrpktptr, wrbufptr, per_write);

                status = FT4222_SPIMaster_SingleWrite(
                                                        host->hal_handle,
                                                        temp_wrpktptr,
                                                        per_write,
                                                        (ft_uint16_t *) &SizeTransfered,
                                                        disable_cs
                                                    );
                if ((FT4222_OK != status) || ((ft_uint16_t)SizeTransfered != per_write))
                {
                    printf("%d FT4222_SPIMaster_SingleWrite failed, SizeTransfered is %d with status %d\n", __LINE__, (ft_uint16_t)SizeTransfered, status);
                    retcode = 0;
                    if ((ft_uint16_t)SizeTransfered != per_write)
                        host->status = FT_GPU_HAL_STATUS_ERROR;
                }               
  
                //continue writing more bytes
                per_write = (ft_uint16_t)SizeTransfered;
                wrbufptr += per_write;
                bytestowr -= per_write;
            }
        }
        //Ft_Gpu_Hal_SlaveSelect(host, FALSE);        
        //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
	}
    else
    {
       ft_uint32_t SizeTransfered;
        //multi channel SPI communication
        while (bytestowr && retcode)
        {
            //Ft_Gpu_Hal_SlaveSelect(host, TRUE);
            //Ft_Gpu_Hal_SlaveSelect(host, TRUE);

            *(temp_wrpktptr + 0) = (hwraddr >> 16) | 0x80; //MSB bits 10 for WRITE
            *(temp_wrpktptr + 1) = (hwraddr >> 8) & 0xFF;
            *(temp_wrpktptr + 2) = hwraddr & 0xff;

            if (bytestowr <= FT4222_MAX_WR_BYTES_PER_CALL_IN_MULTI_CH) //3 for address            
                per_write = bytestowr;                                
            else            
                per_write = FT4222_MAX_WR_BYTES_PER_CALL_IN_MULTI_CH;
                           
            memcpy((temp_wrpktptr + 3), wrbufptr, per_write);

            status = FT4222_SPIMaster_MultiReadWrite(
                                                        host->hal_handle,
                                                        &dummy_read,
                                                        temp_wrpktptr,
                                                        0,
                                                        per_write + 3, // 3 byte of mem address
                                                        0,
                                                        &SizeTransfered
                                                    );
            if (FT4222_OK != status)
            {
                printf("FT4222_SPIMaster_MultiReadWrite failed, status %d\n", status);
                retcode = 0;
                host->status = FT_GPU_HAL_STATUS_ERROR;
            }            

            hwraddr += per_write;
            bytestowr -= per_write;
            wrbufptr += per_write;

            //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
            //Ft_Gpu_Hal_SlaveSelect(host, FALSE);
        }
    }

	return retcode;
}

/***************************************************************************
* Interface Description    : Function to compute FT4222 sys clock and divisor
*                            to obtain user requested SPI communication clock
*                            Available FT4222_ClockRate (FT4222 system clock):
*                               SYS_CLK_60,
*                               SYS_CLK_24,
*                               SYS_CLK_48,
*                               SYS_CLK_80 
*                            Divisors available (FT4222_SPIClock):
*                               CLK_NONE,
*                               CLK_DIV_2,
*                               CLK_DIV_4,
*                               CLK_DIV_8,
*                               CLK_DIV_16,
*                               CLK_DIV_32,
*                               CLK_DIV_64,
*                               CLK_DIV_128,
*                               CLK_DIV_256,
*                               CLK_DIV_512 
* Implementation           : Good performance is observed with divisors other than CLK_DIV_2
*                            and CLK_DIV_4 from test report by firmware developers.
*                            Hence supporting the following clocks for SPI communication
*                               5000KHz
*                               10000KHz
*                               15000KHz
*                               20000KHz
*                               25000KHz 
*                               30000KHz
*                            Global variable host->hal_config.spi_clockrate_khz is
*                            updated accodingly   
* Return Value             : ft_bool_t
*                               TRUE : Supported by FT4222
*                               FALSE : Not supported by FT4222
*
* Author                   :
****************************************************************************/
ft_bool_t Ft_Gpu_Hal_FT4222_ComputeCLK(Ft_Gpu_Hal_Context_t *host, FT4222_ClockRate *sysclk, FT4222_SPIClock *sysdivisor)
{
    //host->hal_config.spi_clockrate_khz is the user requested SPI communication clock      

    if (host->hal_config.spi_clockrate_khz <= 5000)
    {  //set to 5000 KHz              
        *sysclk = SYS_CLK_80;
        *sysdivisor = CLK_DIV_16;
    }
    else if (host->hal_config.spi_clockrate_khz > 5000 && host->hal_config.spi_clockrate_khz <= 10000)
    {
        //set to 10000 KHz
        *sysclk = SYS_CLK_80;
        *sysdivisor = CLK_DIV_8;
    }
    else if (host->hal_config.spi_clockrate_khz > 10000 && host->hal_config.spi_clockrate_khz <= 15000)
    {
        //set to 15000 KHz
        *sysclk = SYS_CLK_60;
        *sysdivisor = CLK_DIV_4; 
    }
    else
    {
        //set to 20000 KHz : Maximum throughput is obeserved with this clock combination
        *sysclk = SYS_CLK_80;
        *sysdivisor = CLK_DIV_4; 
    }
    printf("User Selected SPI clk : %d KHz \n", host->hal_config.spi_clockrate_khz);
    printf("Configured clk :  Ft4222 sys clk enum = %d , divisor enum = %d \n",*sysclk, *sysdivisor);        
    return(TRUE);     
}

