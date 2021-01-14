
  
#include <stdint.h>

#include "fsl_spi_master_driver.h"
#include "fsl_port_hal.h"

#include "SEGGER_RTT.h"
#include "gpio_pins.h"
#include "warp.h"
#include "devSSD1331.h"
#include "devtextSSD1331.h"

volatile uint8_t	inBuffer[1];
volatile uint8_t	payloadBytes[1];


/*
 *	Override Warp firmware's use of these pins and define new aliases.
 */
enum
{
	kSSD1331PinMOSI		= GPIO_MAKE_PIN(HW_GPIOA, 8),
	kSSD1331PinSCK		= GPIO_MAKE_PIN(HW_GPIOA, 9),
	kSSD1331PinCSn		= GPIO_MAKE_PIN(HW_GPIOB, 13),
	kSSD1331PinDC		= GPIO_MAKE_PIN(HW_GPIOA, 12),
	kSSD1331PinRST		= GPIO_MAKE_PIN(HW_GPIOB, 2),
};

static int
writeCommand(uint8_t commandByte)
{
	spi_status_t status;

	/*
	 *	Drive /CS low.
	 *
	 *	Make sure there is a high-to-low transition by first driving high, delay, then drive low.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);
	OSA_TimeDelay(10);
	GPIO_DRV_ClearPinOutput(kSSD1331PinCSn);

	/*
	 *	Drive DC low (command).
	 */
	GPIO_DRV_ClearPinOutput(kSSD1331PinDC);

	payloadBytes[0] = commandByte;
	status = SPI_DRV_MasterTransferBlocking(0	/* master instance */,
					NULL		/* spi_master_user_config_t */,
					(const uint8_t * restrict)&payloadBytes[0],
					(uint8_t * restrict)&inBuffer[0],
					1		/* transfer size */,
					1000		/* timeout in microseconds (unlike I2C which is ms) */);

	/*
	 *	Drive /CS high
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);

	return status;
}

int writeCommand_buf(uint8_t* commandByteBuf, uint8_t len)
{
	spi_status_t status;

	/*
	 *	Drive /CS low.
	 *
	 *	Make sure there is a high-to-low transition by first driving high, delay, then drive low.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);
	GPIO_DRV_ClearPinOutput(kSSD1331PinCSn);

	/*
	 *	Drive DC low (command).
	 */
	GPIO_DRV_ClearPinOutput(kSSD1331PinDC);

	status = SPI_DRV_MasterTransferBlocking(0	                                        /* master instance */,
					                        NULL		                                /* spi_master_user_config_t */,
					                        (const uint8_t * restrict)commandByteBuf,
					                        (uint8_t * restrict)&inBuffer[0],
					                        len		                                    /* transfer size */,
					                        1000		                                /* timeout in microseconds (unlike I2C which is ms) */
					                        );
	/*
	 *	Drive /CS high
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinCSn);

	return status;
}

void draw_result2(char* shot, uint8_t len, uint8_t confidence)
{
    
	//Clear Screen & reset cursor
	reset_cursor(); 
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);

    uint8_t i;
    uint8_t x_cursor = 0;
    uint8_t y_cursor = 10; //these cursors are to determine where to place each char.
    int dig[3];
    dig[0] = confidence/10 + 48;
    dig[1] = confidence%10 + 48;
    dig[2] = '%';
    
    //print each char.    
    for( i=0; i<len; i++) {
        PutChar(x_cursor, y_cursor, (int)*(shot + i));
        x_cursor += X_width;
    } 
    
    x_cursor = 0;
    y_cursor = 45;
    
    i = 0;
    
    //print condidence level, then percentage sign.
    for( i=0; i<3; i++) {
        PutChar(x_cursor, y_cursor, dig[i]);
        x_cursor += X_width;
    } 

}

void draw_result(int16_t  RR, int16_t equivalentCO2)
{
    
	//Clear Screen & reset cursor
	reset_cursor(); 
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);

	SetFontSize(WH); // set tall font
   	foreground(toRGB(0,255,0)); // set text colour
	background(toRGB(255,0,255));
	
	
    uint8_t i;
    uint8_t x_cursor = 0;
    uint8_t y_cursor = 0; //these cursors are to determine where to place each char.
   
	int num[4];
    	num[0] = 'R';
	num[1] = 'R';
    num[2] = RR/10 + 48;
    num[3] = RR%10 + 48;

    //print RR result
	
    for( i=0; i<4; i++) 
	
	{
        PutChar(x_cursor, y_cursor, num[i]);
        x_cursor += X_width;
    } 

	int let[7];
		let[0] = 'C';
		let[1] = 'O';
		let[2] = '2';
		let[3] = ':';
		
		equivalentCO2 = equivalentCO2 / 10;
		if (equivalentCO2 != 0) {
			let [4] = equivalentCO2%10 +48;
		} else 
			{
			let [4] = ' ';
			}
		let[4] = equivalentCO2/1000 +48;
		equivalentCO2 = equivalentCO2/10;
		let[5] = equivalentCO2%10 +48;
		equivalentCO2 = equivalentCO2 / 10;
		let[6] = equivalentCO2%10 +48;
		let[7] = equivalentCO2%10 +48;
		
		for( i=0; i<7; i++) 
	
	{
        PutChar(x_cursor, y_cursor, let[i]);
        x_cursor += X_width;
	} 
			
} 


int
devSSD1331init(void)
{
	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Re-configure SPI to be on PTA8 and PTA9 for MOSI and SCK respectively.
	 */
	PORT_HAL_SetMuxMode(PORTA_BASE, 8u, kPortMuxAlt3);
	PORT_HAL_SetMuxMode(PORTA_BASE, 9u, kPortMuxAlt3);

	enableSPIpins();

	/*
	 *	Override Warp firmware's use of these pins.
	 *
	 *	Reconfigure to use as GPIO.
	 */
	PORT_HAL_SetMuxMode(PORTB_BASE, 13u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTA_BASE, 12u, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB_BASE, 2u, kPortMuxAsGpio);


	/*
	 *	RST high->low->high.
	 */
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_ClearPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);
	GPIO_DRV_SetPinOutput(kSSD1331PinRST);
	OSA_TimeDelay(100);

	/*
	 *	Initialization sequence, borrowed from https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
	 */
	writeCommand(kSSD1331CommandDISPLAYOFF);	// 0xAE
	writeCommand(kSSD1331CommandSETREMAP);		// 0xA0
	writeCommand(0x72);				// RGB Color
	writeCommand(kSSD1331CommandSTARTLINE);		// 0xA1
	writeCommand(0x0);
	writeCommand(kSSD1331CommandDISPLAYOFFSET);	// 0xA2
	writeCommand(0x0);
	writeCommand(kSSD1331CommandNORMALDISPLAY);	// 0xA4
	writeCommand(kSSD1331CommandSETMULTIPLEX);	// 0xA8
	writeCommand(0x3F);				// 0x3F 1/64 duty
	writeCommand(kSSD1331CommandSETMASTER);		// 0xAD
	writeCommand(0x8E);
	writeCommand(kSSD1331CommandPOWERMODE);		// 0xB0
	writeCommand(0x0B);
	writeCommand(kSSD1331CommandPRECHARGE);		// 0xB1
	writeCommand(0x31);
	writeCommand(kSSD1331CommandCLOCKDIV);		// 0xB3
	writeCommand(0xF0);				// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8A
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGEB);	// 0x8B
	writeCommand(0x78);
	writeCommand(kSSD1331CommandPRECHARGEA);	// 0x8C
	writeCommand(0x64);
	writeCommand(kSSD1331CommandPRECHARGELEVEL);	// 0xBB
	writeCommand(0x3A);
	writeCommand(kSSD1331CommandVCOMH);		// 0xBE
	writeCommand(0x3E);
	writeCommand(kSSD1331CommandMASTERCURRENT);	// 0x87
	writeCommand(0x06);
	writeCommand(kSSD1331CommandCONTRASTA);		// 0x81
    writeCommand(0x91);
	writeCommand(kSSD1331CommandCONTRASTB);		// 0x82
	writeCommand(0x50);
	writeCommand(kSSD1331CommandCONTRASTC);		// 0x83
	writeCommand(0x7D);
	writeCommand(kSSD1331CommandDISPLAYON);		// Turn on oled panel

	/*
	 *	To use fill commands, you will have to issue a command to the display to enable them. See the manual.
	 */
	//writeCommand(kSSD1331CommandFILL);
	//writeCommand(0x05);
	
	
	/*
	 *	Clear Screen
	 */
	//writeCommand(kSSD1331CommandCLEAR);
	//writeCommand(0x00);
	//writeCommand(0x00);
	//writeCommand(0x5F);
	//writeCommand(0x3F);
	
	writeCommand(kSSD1331CommandCLEAR);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0x5F);
	writeCommand(0x3F);
	 
	
	writeCommand(kSSD1331CommandFILL);
	writeCommand(0x01);
	
	writeCommand(kSSD1331CommandMASTERCURRENT);	// 0x87
	writeCommand(14);  
   
   //Green screen for debugging purposes
 
   writeCommand(kSSD1331CommandDRAWRECT);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0xFF);
	writeCommand(0x3F);
	writeCommand(0x00);
	writeCommand(0xFF);
	writeCommand(0x00);
	writeCommand(0x00);
	writeCommand(0xFF);
	writeCommand(0x00);


	//SetFontSize(NORMAL); // set text to normal
        //foreground(toRGB(0,255,0));
	//background(toRGB(0,0,0));
	
	//	SetFontSize(WH); // set tall font
   	// 	foreground(toRGB(0,255,0)); // set text colour
	
	SetFontSize(WH); // set tall font
    	foreground(toRGB(0,255,0)); // set text colour

	draw_result2("hello\n\n", 7,00);
	
	return 0;
}
