
// Standard includes
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "gpio.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"

// Common interface includes
#include "uart_if.h"
#include "pin_mux_config.h"

#include "Adafruit_SSD1351.h"

//*****************************************************************************
#define TR_BUFF_SIZE     100



void writeCommand(unsigned char c) {

//TODO 1
/* Write a function to send a command byte c to the OLED via
*  SPI.
*/
    //avail GPIO pins: p2.p18 (gpio28), !p2.p45, p1.p61 (gpio6), p1.p62 (gpio7)
    // SPI_DOUT (mosi): p2.p15
    //don't use p2.rst for oled reset -- use gpio pin for independent reset


    //unsigned long ulUserData;
    unsigned long ulDummy;

    // Enable Chip select
    MAP_SPICSEnable(GSPI_BASE);
    GPIOPinWrite(GPIOA0_BASE, 0x80, 0x00);  //turn off P62 / CS|OC output signal


    GPIOPinWrite(GPIOA0_BASE, 0x40, 0x00);  //turn off pin 61 / DC


     // Loop until user "Enter Key" is pressed
     //while(ulUserData != '\r') {


     // Push the character over SPI
     MAP_SPIDataPut(GSPI_BASE,c);

     // Clean up the receive register into a dummy variable
     MAP_SPIDataGet(GSPI_BASE,&ulDummy);
     //}

     //
     // Disable chip select
     //
     GPIOPinWrite(GPIOA0_BASE, 0x80, 0x80);  //turn on P62 / CS|OC output signal
     MAP_SPICSDisable(GSPI_BASE);


}
//*****************************************************************************

void writeData(unsigned char c) {

//TODO 2
/* Write a function to send a data byte c to the OLED via
*  SPI.
*/
    //unsigned long ulUserData;
    unsigned long ulDummy;

    MAP_SPICSEnable(GSPI_BASE);
    GPIOPinWrite(GPIOA0_BASE, 0x80, 0x00);  //turn off P62 / CS|OC output signal

    // Enable DC
    GPIOPinWrite(GPIOA0_BASE, 0x40, 0x40);  //turn on pin 61 / DC


     // Loop until user "Enter Key" is pressed
     //while(ulUserData != '\r') {


     // Push the character over SPI
     MAP_SPIDataPut(GSPI_BASE,c);

     // Clean up the receive register into a dummy variable
     MAP_SPIDataGet(GSPI_BASE,&ulDummy);
     //}

     //
     // Disable chip select
     //
     GPIOPinWrite(GPIOA0_BASE, 0x80, 0x80);  //turn on P62 / CS|OC output signal
     MAP_SPICSDisable(GSPI_BASE);


//gpio pin = high
//2 chips in unison = one gpio (software), one hardware
}

//*****************************************************************************
void Adafruit_Init(void){

//TODO 3
/* NOTE: This function assumes that the RESET pin of the 
*  OLED has been wired to GPIO28, pin 18 (P2.2). If you 
*  use a different pin for the OLED reset, then you should
*  update the GPIOPinWrite commands below that set RESET 
*  high or low.
*/

  volatile unsigned long delay;

  GPIOPinWrite(GPIOA3_BASE, 0x10, 0);	// RESET = RESET_LOW

  for(delay=0; delay<100; delay=delay+1);// delay minimum 100 ns

  GPIOPinWrite(GPIOA3_BASE, 0x10, 0x10);	// RESET = RESET_HIGH

	// Initialization Sequence
  writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
  writeData(0x12);
  writeCommand(SSD1351_CMD_COMMANDLOCK);  // set command lock
  writeData(0xB1);

  writeCommand(SSD1351_CMD_DISPLAYOFF);  		// 0xAE

  writeCommand(SSD1351_CMD_CLOCKDIV);  		// 0xB3
  writeCommand(0xF1);  						// 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)

  writeCommand(SSD1351_CMD_MUXRATIO);
  writeData(127);

  writeCommand(SSD1351_CMD_SETREMAP);
  writeData(0x74);

  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(0x00);
  writeData(0x7F);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(0x00);
  writeData(0x7F);

  writeCommand(SSD1351_CMD_STARTLINE); 		// 0xA1
  if (SSD1351HEIGHT == 96) {
    writeData(96);
  } else {
    writeData(0);
  }


  writeCommand(SSD1351_CMD_DISPLAYOFFSET); 	// 0xA2
  writeData(0x0);

  writeCommand(SSD1351_CMD_SETGPIO);
  writeData(0x00);

  writeCommand(SSD1351_CMD_FUNCTIONSELECT);
  writeData(0x01); // internal (diode drop)
  //writeData(0x01); // external bias

//    writeCommand(SSSD1351_CMD_SETPHASELENGTH);
//    writeData(0x32);

  writeCommand(SSD1351_CMD_PRECHARGE);  		// 0xB1
  writeCommand(0x32);

  writeCommand(SSD1351_CMD_VCOMH);  			// 0xBE
  writeCommand(0x05);

  writeCommand(SSD1351_CMD_NORMALDISPLAY);  	// 0xA6

  writeCommand(SSD1351_CMD_CONTRASTABC);
  writeData(0xC8);
  writeData(0x80);
  writeData(0xC8);

  writeCommand(SSD1351_CMD_CONTRASTMASTER);
  writeData(0x0F);

  writeCommand(SSD1351_CMD_SETVSL );
  writeData(0xA0);
  writeData(0xB5);
  writeData(0x55);

  writeCommand(SSD1351_CMD_PRECHARGE2);
  writeData(0x01);

  writeCommand(SSD1351_CMD_DISPLAYON);		//--turn on oled panel
}

/***********************************/

void goTo(int x, int y) {
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT)) return;

  // set x and y coordinate
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(SSD1351WIDTH-1);

  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(SSD1351HEIGHT-1);

  writeCommand(SSD1351_CMD_WRITERAM);
}

unsigned int Color565(unsigned char r, unsigned char g, unsigned char b) {
  unsigned int c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}

void fillScreen(unsigned int fillcolor) {
  fillRect(0, 0, SSD1351WIDTH, SSD1351HEIGHT, fillcolor);
}

/**************************************************************************/
/*!
    @brief  Draws a filled rectangle using HW acceleration
*/
/**************************************************************************/
void fillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int fillcolor)
{
  unsigned int i;

  // Bounds check
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT))
	return;

  // Y bounds check
  if (y+h > SSD1351HEIGHT)
  {
    h = SSD1351HEIGHT - y - 1;
  }

  // X bounds check
  if (x+w > SSD1351WIDTH)
  {
    w = SSD1351WIDTH - x - 1;
  }

  // set location
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(x+w-1);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(y+h-1);
  // fill!
  writeCommand(SSD1351_CMD_WRITERAM);

  for (i=0; i < w*h; i++) {
    writeData(fillcolor >> 8);
    writeData(fillcolor);
  }
}

void drawFastVLine(int x, int y, int h, unsigned int color) {

  unsigned int i;
  // Bounds check
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT))
	return;

  // X bounds check
  if (y+h > SSD1351HEIGHT)
  {
    h = SSD1351HEIGHT - y - 1;
  }

  if (h < 0) return;

  // set location
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(x);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(y+h-1);
  // fill!
  writeCommand(SSD1351_CMD_WRITERAM);

  for (i=0; i < h; i++) {
    writeData(color >> 8);
    writeData(color);
  }
}



void drawFastHLine(int x, int y, int w, unsigned int color) {

  unsigned int i;
  // Bounds check
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT))
	return;

  // X bounds check
  if (x+w > SSD1351WIDTH)
  {
    w = SSD1351WIDTH - x - 1;
  }

  if (w < 0) return;

  // set location
  writeCommand(SSD1351_CMD_SETCOLUMN);
  writeData(x);
  writeData(x+w-1);
  writeCommand(SSD1351_CMD_SETROW);
  writeData(y);
  writeData(y);
  // fill!
  writeCommand(SSD1351_CMD_WRITERAM);

  for (i=0; i < w; i++) {
    writeData(color >> 8);
    writeData(color);
  }
}




void drawPixel(int x, int y, unsigned int color)
{
  if ((x >= SSD1351WIDTH) || (y >= SSD1351HEIGHT)) return;
  if ((x < 0) || (y < 0)) return;

  goTo(x, y);

  writeData(color >> 8);
  writeData(color);
}


void  invert(char v) {
   if (v) {
     writeCommand(SSD1351_CMD_INVERTDISPLAY);
   } else {
     	writeCommand(SSD1351_CMD_NORMALDISPLAY);
   }
 }


