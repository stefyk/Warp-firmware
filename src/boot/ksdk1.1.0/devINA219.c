#include <stdlib.h>
#include <stdint.h>

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"


extern volatile WarpI2CDeviceState	deviceINA219State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;


void 
initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer)
{
	deviceStatePointer->i2cAddress	= i2cAddress;
	return;
}

WarpStatus
writeSensorRegisterINA219(uint8_t deviceRegister, uint16_t payload )
{
	uint8_t readSensorRegisterValueMSB = payload >> 8;
	uint8_t readSensorRegisterValueLSB = payload & 0xFF;
	uint8_t payloadBytes[2] = {readSensorRegisterValueMSB, readSensorRegisterValueLSB};
	uint8_t commandByte[1];
	
	i2c_status_t	status;

	switch (deviceRegister)
	{
		case 0x00: case 0x05:
		{
			/* OK */
			break;
		}
		
		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};
	
	commandByte[0] = deviceRegister;

	status = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C instance */,
							&slave,
							commandByte,
							1,
							payloadBytes,
							2,
							gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		
        SEGGER_RTT_printf(0, "write failed 0x%02x \n", status);
		return kWarpStatusDeviceCommunicationFailed;
	}
	

	return kWarpStatusOK;
}

WarpStatus
readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	status;

	//SEGGER_RTT_WriteString(0, " Started reading,");
	
	USED(numberOfBytes);
	switch (deviceRegister)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: 
		case 0x04: case 0x05:
		{
			/* OK */
			break;
		}
		
		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}


	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};
	
	cmdBuf[0] = deviceRegister;

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceINA219State.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		SEGGER_RTT_printf(0, "read in read function failed 0x%02x \n", status);
		return kWarpStatusDeviceCommunicationFailed;
	}
	else
	{
	//SEGGER_RTT_WriteString(0, "read success\n");
	return kWarpStatusOK;
	}
}



void
printSensorDataINA219(bool hexModeFlag)
{
	uint8_t	    readSensorRegisterValueLSB;
	uint8_t	    readSensorRegisterValueMSB;
	uint16_t	readSensorRegisterValueCombined;
	uint16_t    Rawcurrent;//	
	uint16_t 	current;
	int ina219_currentDivider_uA;
	
	WarpStatus	i2cReadStatus;


	
i2cReadStatus = readSensorRegisterINA219(0x04,2); /* numberOfBytes */
readSensorRegisterValueMSB = deviceINA219State.i2cBuffer[0];
readSensorRegisterValueLSB = deviceINA219State.i2cBuffer[1];
readSensorRegisterValueCombined = ((readSensorRegisterValueMSB << 8) | (readSensorRegisterValueLSB));
	
Rawcurrent = readSensorRegisterValueCombined;
			
SEGGER_RTT_WriteString(0, "Read in print function successful\n");
SEGGER_RTT_printf(0, "MSB 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
SEGGER_RTT_printf(0, "Raw current reading: %d",readSensorRegisterValueCombined);
	
	if (i2cReadStatus == kWarpStatusOK)
	
	{

			SEGGER_RTT_printf(0, "MSB 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
			SEGGER_RTT_printf(0, "Raw current reading: %d",readSensorRegisterValueCombined);
			
	
	
	ina219_currentDivider_uA = 100;      // Current LSB = 10uA per bit (1000/10 = 100)	
	current= Rawcurrent / ina219_currentDivider_uA;
	SEGGER_RTT_printf(0, "Current(uA): ,%d", current);
	
	}
	
	else
	{
		SEGGER_RTT_printf(0, "Error reading current in print function,");
	}

}
