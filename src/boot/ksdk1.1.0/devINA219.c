#include <stdint.h>


#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devINA219.h"


#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"


extern volatile WarpI2CDeviceState deviceINA219State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;

uint16_t ina219Calibration;
uint32_t ina219CurrentScale;
float ina219PowerScale;


void 
initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer)
{
	deviceStatePointer->i2cAddress	= i2cAddress;
	return;
}



WarpStatus resetINA219(uint16_t pullupValue)
{
	return setConfig((uint16_t)INA219_DEFAULT_CONFIG, pullupValue);
}

WarpStatus setConfig(uint16_t config, uint16_t pullupValue)
{
	uint8_t bytes [sizeof(uint16_t)] =
	{
		config 		& 0xFF,
		config >> 8 & 0xFF
	};

	return writeSensorRegisterINA219(INA219_REG_CONFIG, bytes[0], bytes[1], pullupValue);
}

WarpStatus calibrateSensor_32v_1A(int16_t pullupValue)
{
	//Cal value
	ina219Calibration = 10240;
	ina219CurrentScale = 25; //mA
	ina219PowerScale = 0.8f;

	uint8_t calibBytes [sizeof(uint16_t)] =
	{
		ina219Calibration      & 0xFF,
		ina219Calibration >> 8 & 0xFF
	};

	uint16_t config = 	INA219_CONFIG_BVOLTAGE_RNG_32V |
						INA219_CONFIG_PGA_GAIN_8_320|
						INA219_CONFIG_BADC_12_BIT |
						INA219_CONFIG_SADC_12_BIT_128_AVG |
						INA219_CONFIG_MODE_BUS_AND_SHUNT_VOLTAGE_CONTINUOUS;

	uint8_t configBytes [sizeof(uint16_t)] =
	{
		config      & 0xFF,
		config >> 8 & 0xFF
	};



	WarpStatus calibStatus =  writeSensorRegisterINA219(INA219_REG_CALIBRATION, calibBytes[0], calibBytes[1], pullupValue); //Wait min 4us for sensor to write
	OSA_TimeDelay(100); //Wait min 4us for sensor to write
	WarpStatus configStatus = writeSensorRegisterINA219(INA219_REG_CONFIG, configBytes[0], configBytes[1], pullupValue);
	SEGGER_RTT_printf(0, "Calibration code 0x%02x\n", calibStatus | configStatus);
	
	return calibStatus | configStatus;		
}

WarpStatus calibrateSensor_16v_400mA(int16_t pullupValue)
{
	//Cal value
	ina219Calibration = 8192;
	ina219CurrentScale = 20; //mA
	ina219PowerScale = 1.0f;

	uint8_t calibBytes [sizeof(uint16_t)] =
	{
		ina219Calibration      & 0xFF,
		ina219Calibration >> 8 & 0xFF
	};

	uint16_t config = 	INA219_CONFIG_BVOLTAGE_RNG_16V |
						INA219_CONFIG_PGA_GAIN_1_40|
						INA219_CONFIG_BADC_12_BIT |
						INA219_CONFIG_SADC_12_BIT |
						INA219_CONFIG_MODE_BUS_AND_SHUNT_VOLTAGE_CONTINUOUS;

	uint8_t configBytes [sizeof(uint16_t)] =
	{
		config      & 0xFF,
		config >> 8 & 0xFF
	};



	WarpStatus calibStatus =  writeSensorRegisterINA219(INA219_REG_CALIBRATION, calibBytes[0], calibBytes[1], pullupValue); //Wait min 4us for sensor to write
	OSA_TimeDelay(100); //Wait min 4us for sensor to write
	WarpStatus configStatus = writeSensorRegisterINA219(INA219_REG_CONFIG, configBytes[0], configBytes[1], pullupValue);
	SEGGER_RTT_printf(0, "Calibration code 0x%02x\n", calibStatus | configStatus);
	
	return calibStatus | configStatus;		
}

float
getCurrent_mA( uint16_t pullupValue)
{
    readSensorRegisterINA219(INA219_REG_CURRENT, 2);
	uint8_t msb = deviceINA219State.i2cBuffer[0];
	uint8_t lsb = deviceINA219State.i2cBuffer[1];

	uint16_t current = (msb << 8) | lsb;

	float currentF = (float) current;

	
	SEGGER_RTT_printf(0, "%d    0x%02x    0x%02x \n", current, msb, lsb);
	currentF /= ina219CurrentScale;

	return currentF;
}

WarpStatus
writeSensorRegisterINA219(uint8_t deviceRegister, uint8_t payload_msb, uint8_t payload_lsb, uint16_t menuI2cPullupValue)
{

    uint8_t payloadBytes[2], commandByte[1];
    i2c_status_t status;

    switch (deviceRegister)
    {
        case 0x00:
        case 0x05:
        {
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
    payloadBytes[0] = payload_msb;
    payloadBytes[1] = payload_lsb;
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

    //SEGGER_RTT_WriteString(0, "write success\n");
	return kWarpStatusOK;
}


WarpStatus
readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	status;


    //SEGGER_RTT_WriteString(0, " entered read function,");

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


    //SEGGER_RTT_printf(0, " 0x%02x,", slave.address);
    //SEGGER_RTT_printf(0, " %d\n", slave.baudRate_kbps);
	cmdBuf[0] = deviceRegister;

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceINA219State.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

    //SEGGER_RTT_WriteString(0, "Status");
	if (status != kStatus_I2C_Success)
	{

        SEGGER_RTT_printf(0, "read failed 0x%02x \n", status);
		return kWarpStatusDeviceCommunicationFailed;
	}

    //SEGGER_RTT_WriteString(0, "read success\n");
	return kWarpStatusOK;
}

/* Calibration vals
32V 1A
 calvalue 10240

  ina219_currentDivider_mA = 25;    // Current LSB = 40uA per bit (1000/40 = 25)
  ina219_powerMultiplier_mW = 0.8f; // Power LSB = 800uW per bit

*/