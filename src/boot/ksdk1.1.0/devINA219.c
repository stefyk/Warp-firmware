WarpStatus
writeSensorRegisterINA219(uint8_t deviceRegister, uint16_t payload,uint16_t menuI2cPullupValue )
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
