#ifndef WARP_BUILD_ENABLE_DEVINA219
#define WARP_BUILD_ENABLE_DEVINA219
#endif


kWarpPinI2C0_SCL			= GPIO_MAKE_PIN(HW_GPIOB, 3),		//	Warp KL03_I2C0_SCL	--> PTB3									*/
kWarpPinI2C0_SDA			= GPIO_MAKE_PIN(HW_GPIOB, 4),		//	Warp KL03_I2C0_SDA	--> PTB4




void		initINA219(const uint8_t i2cAddress, WarpI2CDeviceState volatile *  deviceStatePointer);

WarpStatus	readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes);
WarpStatus	writeSensorRegisterINA219(uint8_t deviceRegister,
					uint8_t payloadBtye,
					uint16_t menuI2cPullupValue);


/*WarpStatus	configureSensorINA219(uint8_t payloadF_SETUP, uint8_t payloadCTRL_REG1, uint16_t menuI2cPullupValue);
WarpStatus	readSensorSignalINA219(WarpTypeMask signal,
					WarpSignalPrecision precision,
					WarpSignalAccuracy accuracy,
					WarpSignalReliability reliability,
					WarpSignalNoise noise);
void		printSensorDataINA219(bool hexModeFlag);
