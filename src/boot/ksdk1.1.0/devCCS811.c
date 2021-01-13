#include <stdlib.h>
#include <time.h> 

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
//#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"
//#include "math.h"



extern volatile WarpI2CDeviceState	deviceCCS811State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;

int16_t		threshold;
time_t 		seconds;
bool  		aboveThreshold;
bool  		Ready;
float  		RR;
float  		offset = 0;
float 		timepassed;
float  		counter = 0;


/*
 *	CCS811.
 */
void
initCCS811(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer)
{
	deviceStatePointer->i2cAddress	= i2cAddress;
	deviceStatePointer->signalType	= (	kWarpTypeMaskTotalVOC 		|
						kWarpTypeMaskEquivalentCO2	|
						kWarpTypeMaskHumidity 		|
						kWarpTypeMaskTemperature
					);
	return;
}

WarpStatus
writeSensorRegisterCCS811(uint8_t deviceRegister, uint8_t *payload, uint16_t menuI2cPullupValue)
{
	uint8_t		commandByte[1];
	uint8_t		payloadSize;
	i2c_status_t	status;

	switch (deviceRegister)
	{
		case 0x01:
		{
			payloadSize = 1;
			break;
		}

		case 0x11:
		{
			payloadSize = 2;
			break;
		}

		case 0x05:
		case 0xF1:
		case 0xFF:
		{
			payloadSize = 4;
			break;
		}

		case 0x10:
		{
			payloadSize = 5;
			break;
		}

		case 0xF2:
		{
			payloadSize = 9;
			break;
		}

		case 0xF3:
		case 0xF4:
		{
			payloadSize = 0;
			break;
		}

		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}

	i2c_device_t slave =
	{
		.address = deviceCCS811State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	commandByte[0] = deviceRegister;

	if(payloadSize)
	{
		status = I2C_DRV_MasterSendDataBlocking(
								0 /* I2C instance */,
								&slave,
								commandByte,
								1,
								payload,
								payloadSize,
								gWarpI2cTimeoutMilliseconds);
	}
	else
	{
		status = I2C_DRV_MasterSendDataBlocking(
						0 /* I2C instance */,
						&slave,
						commandByte,
						1,
						NULL,
						0,
						gWarpI2cTimeoutMilliseconds);
	}

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorCCS811(uint8_t *payloadMEAS_MODE, uint16_t menuI2cPullupValue)
{
	WarpStatus	status1, status2;

	/*
	 *	See https://narcisaam.github.io/Init_Device/ for more information
	 *	on how to initialize and configure CCS811
	 */

	/*
	 *	Delay needed before start of i2c.
	 */

	OSA_TimeDelay(20);

	status1 = writeSensorRegisterCCS811(kWarpSensorConfigurationRegisterCCS811APP_START /* register address APP_START */,
							payloadMEAS_MODE /* Dummy value */,
							menuI2cPullupValue);

	/*
	 *	Wait for the sensor to change to application mode
	 */
	OSA_TimeDelay(500);

	status2 = writeSensorRegisterCCS811(kWarpSensorConfigurationRegisterCCS811MEAS_MODE /* register address MEAS_MODE */,
							payloadMEAS_MODE /* payload: 3F initial reset */,
							menuI2cPullupValue);

	/*
	 *	After writing to MEAS_MODE to configure the sensor in mode 1-4,
	 *	run CCS811 for 20 minutes, before accurate readings are generated.
	 */

	return (status1 | status2);
}

WarpStatus
readSensorRegisterCCS811(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	returnValue;


	if ((deviceRegister > 0xFF) || (numberOfBytes > kWarpSizesI2cBufferBytes))
	{
		return kWarpStatusBadDeviceCommand;
	}

	i2c_device_t slave =
	{
		.address = deviceCCS811State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};


	cmdBuf[0] = deviceRegister;


	returnValue = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceCCS811State.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

	if (returnValue != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
printSensorDataCCS811(bool hexModeFlag)
{
	//uint16_t	readSensorRegisterValueLSB;
	//uint16_t	readSensorRegisterValueMSB;
	//int16_t		readSensorRegisterValueCombined;
	//int16_t		Vref;
	//int16_t		Vntc;
	//int16_t		Rntc;
	int16_t		equivalentCO2, TVOC;
	
	WarpStatus	i2cReadStatus;


	i2cReadStatus	= readSensorRegisterCCS811(kWarpSensorOutputRegisterCCS811ALG_DATA, 4 /* numberOfBytes */);
	equivalentCO2	= (deviceCCS811State.i2cBuffer[0] << 8) | deviceCCS811State.i2cBuffer[1];
	TVOC		= (deviceCCS811State.i2cBuffer[2] << 8) | deviceCCS811State.i2cBuffer[3];
	threshold	= 1000;
    
			
	
	if (i2cReadStatus != kWarpStatusOK)
	{
		SEGGER_RTT_WriteString(0, " ----, ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			SEGGER_RTT_printf(0, " 0x%02x 0x%02x, 0x%02x 0x%02x,",
				deviceCCS811State.i2cBuffer[3],
				deviceCCS811State.i2cBuffer[2],
				deviceCCS811State.i2cBuffer[1],
				deviceCCS811State.i2cBuffer[0]);
		}
		else
		{
			//SEGGER_RTT_printf(0, "Into loop1!");
			
			
			seconds = time(NULL);
			timepassed = seconds - offset;
				
				if(timepassed >= 0 && timepassed < 20000)
   					{
						SEGGER_RTT_printf(0, "Into loop2!");
						Ready = 0;
						
						
   					if(equivalentCO2 > threshold && !aboveThreshold) //detects when the signal has passed the threshold and is on the risign edge
     						{
								SEGGER_RTT_printf(0, "Breath detected!");
    							counter++;   // counts up the beats detected
       							aboveThreshold = true;
      						}

    				if(equivalentCO2 < threshold) // if the value crosses the threshold but it is on the falling edge it does not detect a beat
      						{
								SEGGER_RTT_printf(0, "Into loop 2!");
       							aboveThreshold = false;
								SEGGER_RTT_printf(0, "No Breath detected!");
      						}
     
   					}
					
   				if(timepassed >= 20000) //once 10 seconds have passed, it estimates the beats per minute
  					{
						SEGGER_RTT_printf(0, "Into loop 3!");
  						RR = counter*3;   // Compute bpm
   						counter = 0;
   						Ready = 1;
   						offset = seconds;  // Reset Counter
   					}
				 
				
				//if (equivalentCO2 > threshold && !aboveThreshold)
				//		{
				//		aboveThreshold = true;
				//		SEGGER_RTT_printf(0, "Breath detected!");
				//		}
				//if(equivalentCO2 < threshold) 
      			//			{
       			//			aboveThreshold = false;  
     			//			}
			
			SEGGER_RTT_printf(0, " %d, %d, %d, %d, %d", equivalentCO2, TVOC, RR, counter, timepassed);
		}
	}
}
