 
#ifndef __ssd1331_H__
#define __ssd1331_H__
 
#include "mbed.h"
 
// Screen Settings
#define width   96-1        // Max X axial direction in screen
#define height  64-1        // Max Y axial direction in screen
#define Set_Column_Address  0x15
#define Set_Row_Address     0x75
#define contrastA           0x81
#define contrastB           0x82
#define contrastC           0x83
#define display_on          0xAF
#define display_off         0xAE
 
// Internal Font size settings
#define NORMAL  0
#define WIDE    1
#define HIGH    2
#define WH      3
#define WHx36   4
#define X_width 6 
#define Y_height 8 
 
 
// GAC hardware acceleration commands
#define GAC_DRAW_LINE           0x21    // Draw Line
#define GAC_DRAW_RECTANGLE      0x22    // Rectangle
#define GAC_COPY_AREA           0x23    // Copy Area
#define GAC_DIM_WINDOW          0x24    // Dim Window
#define GAC_CLEAR_WINDOW        0x25    // Clear Window
#define GAC_FILL_ENABLE_DISABLE 0x26    // Enable Fill
#define SCROLL_SETUP            0x27    // Setup scroll
#define SCROLL_STOP             0x2E    // Scroll Stop
#define SCROLL_START            0x2F    // Scroll Start
 
// Basic RGB color definitions         RED GREEN BLUE values                         
 
#define Black           0x0000      //   0,   0,   0 
#define LightGrey       0xC618      // 192, 192, 192 
#define DarkGrey        0x7BEF      // 128, 128, 128 
#define Red             0xF800      // 255,   0,   0 
#define Green           0x07E0      //   0, 255,   0 
#define Cyan            0x07FF      //   0, 255, 255 
#define Blue            0x001F      //   0,   0, 255 
#define Magenta         0xF81F      // 255,   0, 255 
#define Yellow          0xFFE0      // 255, 255,   0 
#define White           0xFFFF      // 255, 255, 255 
 
// example code
/*
#include "mbed.h"
#include "ssd1331.h"
 
ssd1331 oled(D8, D9, D10, D11, NC, D13); // cs, res, dc, miso(nc), sck (KL25z)
 
char Time[50],Date[50];
void gettime();
 
uint8_t main() { 
 
    while(1){
        
        oled.Fill_Screen(oled.toRGB(255,0,0)); //red
        wait_ms(500);
        oled.Fill_Screen(oled.toRGB(0,255,0)); //green
        wait_ms(500);
        oled.Fill_Screen(oled.toRGB(0,0,255)); //blue
        wait_ms(500);
        oled.Fill_Screen(oled.toRGB(255,255,255)); //white
        wait_ms(500);
        
        oled.cls(); // clear screen to black
 
        oled.circle (20, 40, 30 ,oled.toRGB(0,0,255) , 1);      //fill circle
        oled.circle (20, 40, 30 ,oled.toRGB(255,255,255) , 0);  //circle 
        oled.circle (20, 60, 40 ,oled.toRGB(255,0,0) , 0);      //circle
        oled.line( 0, 0, width, height, oled.toRGB(0,255,255)); //line
        oled.line( width, 0, 0, height, oled.toRGB(255,0,255)); //line
        oled.rectangle(10,10,90,60,oled.toRGB(255,255,0));      //rectangle
        oled.fillrectangle(20,20,40,40,oled.toRGB(255,255,255),oled.toRGB(0,255,0)); //fillrectangle
        
        for(uint8_t y = 9; y >= 0; y--) {
             oled.contrast(y);  // set contrast level
             oled.foreground(oled.toRGB(255,255,255)); // set text colour
             oled.locate(1, 10); // set text start location
             oled.printf("%d",y); // std printf
             wait_ms(300);
        }  
        
        wait_ms(1000);
        oled.contrast(9); // set contrast to maximum
        wait_ms(2000);
        oled.cls();
 
        oled.SetFontSize(HIGH); // set tall font
        oled.foreground(oled.toRGB(0,255,0)); // set text colour
        oled.locate(0, 10);
        oled.printf( "HIGH 12345");  
        
        oled.SetFontSize(WIDE); // set text to wide
        oled.foreground(oled.toRGB(0,0,255));
        oled.locate(0, 28);
        oled.printf( "WIDE 123");  
        
        oled.SetFontSize(WH); // set text to wide and tall
        oled.foreground(oled.toRGB(255,0,0));
        oled.locate(0, 40);
        oled.printf( "WH 123");
        
        oled.SetFontSize(NORMAL); // set text to normal
        oled.foreground(oled.toRGB(255,255,255));      
                
        oled.ScrollSet(0,8,18,1,0); // set scroll function
        oled.Scrollstart(); // start scroll
       
        gettime();wait(1);gettime();wait(1);gettime();wait(1);      
        oled.ScrollSet(0,8,18,-2,0);
        oled.Scrollstart();       
        gettime();wait(1);gettime();wait(1);gettime();wait(1);
        
        oled.ScrollSet(0,8,18,3,0);
        oled.Scrollstart();
        
        gettime();wait(1);gettime();wait(1);gettime();wait(1);
        
        oled.ScrollSet(0,8,18,-4,0);
        oled.Scrollstart();
       
        gettime();wait(1);gettime();wait(1);gettime();wait(1);
        
        oled.Scrollstop(); // stop scroll
        wait(1);
     }               
}
void gettime()
{    
    time_t seconds = time(NULL);
    strftime(Time,40,"%H:%M:%S %a", localtime(&seconds));
    strftime(Date,40,"%d-%b-%Y", localtime(&seconds));
    oled.locate(0, 0);
    oled.printf(Time); 
}
*/
 
class ssd1331 : public Stream {
public:
    // constructor
    ssd1331(PinName cs_pin, PinName rst_pin, PinName a0_pin, PinName mosi_pin, PinName miso_pin, PinName sclk_pin);
    
    void Init(void);
    void pixel(uint8_t x,uint8_t y, uint16_t color); // place a pixel x,y coordinates, color
    void rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t colorline); // draw rectangle, start x,y end x,y, color
    void fillrect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t colorline,uint16_t colorfill); // fill rectangle start x,y, end x,y, outline color, fill color.
    void line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t color); // draw line start x,y, end x,y, color
    void circle (uint8_t radius, uint8_t x, uint8_t y,uint16_t color,uint16_t fill); // draw circle radius, position x,y, color
    void Fill_Screen(uint16_t color); // fill screen with any colour
    void foreground(uint16_t color); // set text color
    void background(uint16_t color); // set background color
    void SetFontSize(uint8_t); // set internal font size NORMAL, HIGH, WIDE, WH (high and wide), WHx36 (large 36x36 pixel size)
    void on();  // display on
    void off(); // display off
    void cls(); // clear screen to black screen
    void dim();  // flip dim/normal on each call
    void contrast(char value); //0~9 low~high
    void locate(uint8_t column, uint8_t row); // text start position x,y
    uint16_t toRGB(uint16_t R,uint16_t G,uint16_t B);   // get color from RGB values 00~FF(0~255)
    uint8_t row(); // set row position (in pixels)
    uint8_t column(); // set column position (in pixels)
    void ScrollSet(int8_t horizontal, int8_t startline, int8_t linecount, int8_t vertical , int8_t frame_interval); // set up scroll function
    void Scrollstart(); // start scrolling
    void Scrollstop(); // stop scrolling
    void Copy(uint8_t src_x1,uint8_t src_y1,uint8_t src_x2,uint8_t src_y2, uint8_t dst_x,uint8_t dst_y); // GAC function to copy/paste area on the screen
    void character(uint8_t x, uint8_t y, uint16_t c); // Print single character, x & y pixel co-ords.
    void set_font(unsigned char* f); // set external font. Use oled.set_font(NULL) to restore default font
    void Bitmap(const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t w, uint8_t h, unsigned char color); // load mono bitmap from flash
    void Bitmap16(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap); // load bitmap from flash
    
    int Bitmap16RAM(uint8_t x, uint8_t y, unsigned char *Name_BMP); // copy image to RAM, uses up to 12.288k RAM, Fast but need plenty of RAM
    int Bitmap16FS(uint8_t x, uint8_t y, unsigned char *Name_BMP); // load from fielsystem, uses 96b RAM, slower, will work on any MCU
    // use GIMP to generate images, open image, select image-scale image, set width/height(max 96x64), select File-Export As
    // select Windows BMP image, Name it with .BMP extension, select export, select Compatability Options- check 'Do Not write colour space'
    // select Advanced Options- check '16bit R5 G6 B5', select export. Copy the file to a SD card or to the local file system. 
    // image maximum size 96x64, smaller images can be placed anywhere on screen setting x,y co-ords but must fit within boundaries
    // example:
    // oled.Bitmap16FS(0,0,(unsigned char *)"/sd/image.bmp"); full size 96x64 image
    // oled.Bitmap16RAM(20,20,(unsigned char *)"/sd/image.bmp"); place smaller image starting x-20, y-20
   
protected:
    // Stream implementation functions
    virtual int _putc(int c);
    virtual int _getc();
private:
    void RegWrite(unsigned char Command);
    void RegWriteM(unsigned char *Command, uint8_t count);
    void DataWrite(unsigned char c);
    void DataWrite_to(uint16_t Dat);
    void FontSizeConvert(int *lpx, int *lpy);
    void PutChar(uint8_t x,uint8_t y,int a);
    void PutCharInt(uint8_t x,uint8_t y,uint16_t a);
    void putp(int colour);
    unsigned char* font;
    uint16_t Char_Color;    // text color
    uint16_t BGround_Color; // background color
    void window(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    void Maxwindow(); // reset display window to full size
    // pixel location
    uint8_t _x;
    uint8_t _y;
    
    // window location
    uint8_t _x1;
    uint8_t _x2;
    uint8_t _y1;
    uint8_t _y2;
    uint8_t char_x;
    uint8_t char_y;
    uint8_t chr_size;
    uint8_t cwidth;       // character's width
    uint8_t cvert;        // character's height
    uint8_t externalfont;
    DigitalOut  CS,  RES,  DC;
    SPI spi; // mosi, miso, sclk    
};
 
#endif