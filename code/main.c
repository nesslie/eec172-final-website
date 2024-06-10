//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************


//*****************************************************************************
//
// Application Name     -   SSL Demo
// Application Overview -   This is a sample application demonstrating the
//                          use of secure sockets on a CC3200 device.The
//                          application connects to an AP and
//                          tries to establish a secure connection to the
//                          Google server.
// Application Details  -
// docs\examples\CC32xx_SSL_Demo_Application.pdf
// or
// http://processors.wiki.ti.com/index.php/CC32xx_SSL_Demo_Application
//
// Team 9: Emily Hoang and Brian Barcenas
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup ssl
//! @{
//
//*****************************************************************************

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "utils.h"
#include "systick.h"
#include "rom_map.h"
#include "timer.h"
#include "timer_if.h"
#include "gpio.h"
#include "uart.h"

#include "spi.h"
#include "interrupt.h"

//Common interface includes
#include "gpio_if.h"
#include "i2c_if.h"
#include "common.h"
#include "uart_if.h"

#include "Adafruit_GFX.h"
#include "glcdfont.h"
#include "oled_test.h"

// Custom includes
#include "utils/network_utils.h"

// Net App Includes
#include "json.h"

// Standard includes
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Adafruit_SSD1351.h"
#include "pin_mux_config.h"
#include "spi.h"
#include "gpio.h"


// Pin configurations
#include "pin_mux_config.h"
//#include "pinmux.h"


//*****************************************************************************
#define SPI_IF_BIT_RATE 100000
#define TR_BUFF_SIZE     100

//*****************************************************************************



//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                6    /* Current Date */
#define MONTH               6     /* Month 1-12 */
#define YEAR                2024  /* Current year */
#define HOUR                2    /* Time - hours */
#define MINUTE              32    /* Time - minutes */
#define SECOND              0     /* Time - seconds */


#define APPLICATION_NAME      "SSL"
#define APPLICATION_VERSION   "SQ24"
#define SERVER_NAME           "a1g5zjt2s28trs-ats.iot.us-east-2.amazonaws.com" // CHANGE ME
#define GOOGLE_DST_PORT       8443


#define POSTHEADER "POST /things/lab4/shadow HTTP/1.1\r\n"             // CHANGE ME
#define GETHEADER "GET /things/lab4/shadow HTTP/1.1\r\n"             // CHANGE ME
#define HOSTHEADER "Host: a1g5zjt2s28trs-ats.iot.us-east-2.amazonaws.com\r\n"  // CHANGE ME
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"

#define DATA1 "{" \
            "\"state\": {\r\n"                                              \
                "\"desired\" : {\r\n"                                       \
                    "\"var\" :\""                                           \

#define MESSAGE_HEADER "{" \
            "\"state\": {\r\n"                                              \
                "\"desired\" : {\r\n"                                       \
                    "\"message\" :\""                                           \


#define STOCK "{" \
            "\"state\": {\r\n"                                              \
                "\"desired\" : {\r\n"                                       \
                    "\"stock\" :\""                                           \
                    //  "Hello phone, "                                     \


#define DATA3           "\"\r\n"                                            \
                "}"                                                         \
            "}"                                                             \
        "}\r\n\r\n"



#define STOCKS_MODE 100
#define MESSAGES_MODE 101
#define NO_SEND_MODE 99
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif



// some helpful macros for systick

// the cc3200's fixed clock frequency of 80 MHz
// note the use of ULL to indicate an unsigned long long constant
#define SYSCLKFREQ 80000000ULL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
    ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\

// macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended
volatile int systick_cnt = 0;

extern void (* const g_pfnVectors[])(void);


#define RETERR_IF_TRUE(condition)                                              \
  {                                                                            \
    if (condition)                                                             \
      return FAILURE;                                                          \
  }
#define RET_IF_ERR(Func)                                                       \
  {                                                                            \
    int iRetVal = (Func);                                                      \
    if (SUCCESS != iRetVal)                                                    \
      return iRetVal;                                                          \
  }


#define FOREVER 1

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static int set_time();
static void BoardInit(void);
static int http_post(int);
static int http_get(int);
void extractStockObject(const char *);
int appMode;


//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************

static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

bool sendData;
bool renderBuffer;
bool deleteFlag;
bool receivedGet;

volatile unsigned long SW2_intcount;
volatile unsigned char SW2_intflag;
volatile unsigned long SW3_intcount;
volatile unsigned char SW3_intflag;

//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************

/**
 * Reset SysTick Counter
 */
static inline void SysTickReset(void) {
    // any write to the ST_CURRENT register clears it
    // after clearing it automatically gets reset without
    // triggering exception logic
    // see reference manual section 3.2.1
    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;
}

/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt = 1;
}

//*****************************************************************************
/** INTERRUPT CONFIGURATION: **/

typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

static const PinSetting switch2 = { .port = GPIOA2_BASE, .pin = 0x40};
static const PinSetting irPinConfig = { .port = GPIOA0_BASE, .pin = 0x1};


static void GPIOA1IntHandler(void) { // SW3 handler
    unsigned long ulStatus;

    ulStatus = MAP_GPIOIntStatus(GPIOA1_BASE, true);
    MAP_GPIOIntClear(GPIOA1_BASE, ulStatus);        // clear interrupts on GPIOA1
    SW3_intcount++;
    SW3_intflag=1;
}


static void GPIOA2IntHandler(void) {    // SW2 handler
    unsigned long ulStatus;

    ulStatus = MAP_GPIOIntStatus(switch2.port, true);
    MAP_GPIOIntClear(switch2.port, ulStatus);       // clear interrupts on GPIOA2
    SW2_intcount++;
    SW2_intflag=1;
 //   appMode = NO_SEND_MODE;
}

bool sendData;




volatile unsigned long irIntCount;
volatile unsigned char irIntFlag;

//************************************************************************

volatile unsigned char outgoingMsgBuffer[100]; //buffer to store msg to write
volatile int outlength = 0;

volatile char letter;
volatile int pressedCount1 = -1;
volatile int pressedCount2 = -1;
volatile int pressedCount3 = -1;
volatile int pressedCount4 = -1;
volatile int pressedCount5 = -1;
volatile int pressedCount6 = -1;
volatile int pressedCount7 = -1;
volatile int pressedCount8 = -1;
volatile int pressedCount9 = -1;

long lRetVal = -1;

void displayChar(unsigned long target){ //gets button and writes letter thru char cycling
    switch(target){
        case 0xC004: {  //button 0
            char letter = ' ';
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);

            Report("Space!\n\r");
            //Report("You pressed 0!\n\r");
            break;
        }
        case 0xC084: {  //button 1
            pressedCount1++;
            char dict[] = ".!?";
            letter = dict[pressedCount1 % 3]; //cycle thru char array
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //draw/write char on oled:
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            Report("%c!\n\r", letter);
            if (pressedCount1 == 3) { //loop back to start of cycle
                pressedCount1 = 0;
            }
            //Report("You pressed 1!\n\r");
            break;
        }
        case 0xC044:{   //button 2
            pressedCount2++;
            //Report("%d!\n\r", pressedCount2);
            char dict[] = "ABC";
            letter = dict[pressedCount2 % 3];
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], WHITE, WHITE, 1);
            Report("%c!\n\r", letter);
            if (pressedCount2 == 3) {
               pressedCount2 = 0;
            }
            // Report("You pressed 2!\n\r");
            break;
        }
        case 0xC0C4:{   //button 3
            pressedCount3++;
            char dict[] = "DEF";
            letter = dict[pressedCount3 % 3];
            // Report("You pressed 3!\n\r");
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            fillRect((outlength)*10, 118, 10, 15, WHITE);
            drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            Report("%c!\n\r", letter);
            if (pressedCount3 == 3) {
               pressedCount3 = 0;
            }
            break;
        }
        case 0xC024:{   //button 4
            pressedCount4++;
            char dict[] = "GHI";
            letter = dict[pressedCount4 % 3];
            Report("%c!\n\r", letter);
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            //  Report("You pressed 4!\n\r");
            if (pressedCount4 == 3) {
                pressedCount4 = 0;
            }
            break;
        }
        case 0xC0A4:{   //button 5
            pressedCount5++;
            char dict[] = "JKL";
            letter = dict[pressedCount5 % 3];
            Report("%c!\n\r", letter);
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            // Report("You pressed 5!\n\r");
            if (pressedCount5 == 3) {
                pressedCount5 = 0;
            }
            break;
        }
        case 0xC064:{   //button 6
            pressedCount6++;
            char dict[] = "MNO";
            letter = dict[pressedCount6 % 3];
            Report("%c!\n\r", letter);
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            // Report("You pressed 6!\n\r");
            if (pressedCount6 == 3) {
                pressedCount6 = 0;
            }
            break;
        }
        case 0xC0E4:{   //button 7
            pressedCount7++;
            char dict[] = "PQRS";
            letter = dict[pressedCount7 % 4];
            Report("%c!\n\r", letter);
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            // Report("You pressed 7!\n\r");
            if (pressedCount7 == 4) {
                pressedCount7 = 0;
            }
            break;
        }
        case 0xC014:{   //button 8
            pressedCount8++;
            char dict[] = "TUV";
            letter = dict[pressedCount8 % 3];
            Report("%c!\n\r", letter);
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            //  Report("You pressed 8!\n\r");
            if (pressedCount8 == 3) {
                pressedCount8 = 0;
            }
            break;
        }
        case 0xC094:{   //button 9
            pressedCount9++;
            char dict[] = "WXYZ";
            letter = dict[pressedCount9 % 4];
            Report("%c!\n\r", letter);
            outgoingMsgBuffer[outlength] = letter;
            renderBuffer = 1;
            //fillRect((outlength)*10, 90, 10, 15, WHITE);
            //drawChar(outlength*10, 90, outgoingMsgBuffer[outlength], BLACK, BLACK, 2);
            //fillRect((outlength)*10, 118, 10, 15, WHITE);
            //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
            // Report("You pressed 9!\n\r");
            if (pressedCount9 == 4) {
                pressedCount9 = 0;
            }
            break;
        }
        case 0xC038:{   //button mute; enter/send message
            // send
            // Set flag to true to call http_post() in main
            sendData = true;
            Report("You pressed Mute!\n\r");
            break;
        }
        case 0xC0A0:    //button last; delete last char
            // delete
            outgoingMsgBuffer[outlength-1] = '\0';
            //fillRect((outlength-1)*10, 90, 10, 15, WHITE);
            fillRect((outlength)*10, 118, 10, 15, WHITE);

            if (outlength >= 0) {    //edge case: if buffer is empty, then make sure index stays at 0
            --outlength;
            } else {
                outlength = 0;
            }
            deleteFlag = 1;
            Report("You pressed Last!\n\r");
            break;
        default:
            Report("Unrecognized Button \n\r");
            break;
    }
}

volatile unsigned long long ulsystickdelta;
volatile bool repeated = 0;
volatile unsigned long current = 0;
unsigned long binaryBuffer[36];

static void irTestHandler(void){
    unsigned long ulStatus;

     ulStatus = MAP_GPIOIntStatus(irPinConfig.port, true);
     MAP_GPIOIntClear(irPinConfig.port, ulStatus);    // clear interrupts on GPIOA0

     irIntCount++;

     ulsystickdelta = TICKS_TO_US(SYSTICK_RELOAD_VAL - MAP_SysTickValueGet());
     ulsystickdelta = ulsystickdelta >> 11; //convert to 1 or 0

     if(ulsystickdelta > 2){    //if not 1 or 0
         memset(binaryBuffer, 0, sizeof(binaryBuffer));
         irIntCount = 0;

         if(ulsystickdelta == 11 || ulsystickdelta == 12) {     //leader code; reset timer
             //Report("Key repeating!\n\r")
             Timer_IF_ReLoad(TIMERA0_BASE, TIMER_A, 1000);
         } else if (ulsystickdelta >= 13) {     //not repeated
             repeated = 0;
             current = 0;
         }
     } else {   // 1 or 0
         binaryBuffer[irIntCount] = ulsystickdelta;
         //Report("%lu\n\r", ulsystickdelta);
     }

     if(repeated==0){   //not repeated
         if(irIntCount == 16) {     //end of first waveform; 16 bits
             binaryBuffer[0] = 0;
             unsigned long power = 1, output = 0;
             unsigned long i = 0;
             for(i = 16; i > 0; --i){   //convert binary to decimal
                 output += binaryBuffer[i] * power;
                 power *= 2;
             }
             if(output != current){
                 repeated = 1;          //set high to ensure no repeats
                 displayChar(output);   //get button for first press
             }
             //reset:
              current = output;
              //Report("%lu\n\r", output);
              irIntCount = 0;
              memset(binaryBuffer, 0, sizeof(binaryBuffer));
              Timer_IF_ReLoad(TIMERA0_BASE, TIMER_A, 1000);
          }
      }
     SysTickReset();
}

static void timeoutHandler(void){    //when button press times out; resets
    Timer_IF_InterruptClear(TIMERA0_BASE);  //clear interrupt
    repeated = 0;
    Report("Current Buffer: %s\n\r", outgoingMsgBuffer);
    Timer_IF_ReLoad(TIMERA0_BASE, TIMER_A, 1000);
    current = 0;

    //reset each button clicks when timed out:
    pressedCount1 = -1;
    pressedCount2 = -1;
    pressedCount3 = -1;
    pressedCount4 = -1;
    pressedCount5 = -1;
    pressedCount6 = -1;
    pressedCount7 = -1;
    pressedCount8 = -1;
    pressedCount9 = -1;

    //write char to oled on timeout:
    if(outgoingMsgBuffer[outlength] != '\0'){
        ++outlength;
    }
}

void DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen) {
  unsigned char ucBufIndx = 0;
  UART_PRINT("Read contents");
  UART_PRINT("\n\r");
  while (ucBufIndx < ucLen) {
    // UART_PRINT(" 0x%x, ", pucDataBuf[ucBufIndx]);
    UART_PRINT(" %d, ", pucDataBuf[ucBufIndx]);
    ucBufIndx++;
    if ((ucBufIndx % 8) == 0) {
      UART_PRINT("\n\r");
    }
  }
  UART_PRINT("\n\r");
}

int ProcessReadCommand(char *pcInpString) {
  unsigned char ucDevAddr, ucLen;
  unsigned char aucDataBuf[256];
  char *pcErrPtr;
  int iRetVal;

  //
  // Get the device address
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucDevAddr = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
  //
  // Get the length of data to be read
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucLen = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);
  // RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

  //
  // Read the specified length of data
  //
  iRetVal = I2C_IF_Read(ucDevAddr, aucDataBuf, ucLen);

  if (iRetVal == SUCCESS) {
    UART_PRINT("I2C Read complete\n\r");

    //
    // Display the buffer over UART on successful write
    //
    DisplayBuffer(aucDataBuf, ucLen);
  } else {
    UART_PRINT("I2C Read failed\n\r");
    return FAILURE;
  }

  return SUCCESS;
}

int getIntFromBuffer(unsigned char *pucDataBuf, unsigned char ucLen) {
  unsigned char ucBufIndx = 0;
  int buffer;

  memcpy(&buffer, &pucDataBuf[ucBufIndx], sizeof(pucDataBuf[ucBufIndx]));

  if (buffer & 0x80) {
    buffer = buffer | 0xffffff00;
  }
  return buffer;
}

//****************************************************************************
//
//! Parses the readreg command parameters and invokes the I2C APIs
//! i2c readreg 0x<dev_addr> 0x<reg_offset> <rdlen>
//!
//! \param pcInpString pointer to the readreg command parameters
//!
//! This function
//!    1. Parses the readreg command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessReadRegCommand(char *pcInpString) {
  unsigned char ucDevAddr, ucRegOffset, ucRdLen;
  unsigned char aucRdDataBuf[256];
  char *pcErrPtr;

  //
  // Get the device address
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucDevAddr = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
  //
  // Get the register offset address
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucRegOffset = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);

  //
  // Get the length of data to be read
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucRdLen = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);
  // RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

  //
  // Write the register address to be read from.
  // Stop bit implicitly assumed to be 0.
  //
  RET_IF_ERR(I2C_IF_Write(ucDevAddr, &ucRegOffset, 1, 0));

  //
  // Read the specified length of data
  //
  RET_IF_ERR(I2C_IF_Read(ucDevAddr, &aucRdDataBuf[0], ucRdLen));

  // UART_PRINT("I2C Read From address complete\n\r");

  //
  // Display the buffer over UART on successful readreg
  //
  // DisplayBuffer(aucRdDataBuf, ucRdLen);

  return getIntFromBuffer(aucRdDataBuf, ucRdLen);
}

//****************************************************************************
//
//! Parses the writereg command parameters and invokes the I2C APIs
//! i2c writereg 0x<dev_addr> 0x<reg_offset> <wrlen> <0x<byte0> [0x<byte1> ...]>
//!
//! \param pcInpString pointer to the readreg command parameters
//!
//! This function
//!    1. Parses the writereg command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessWriteRegCommand(char *pcInpString) {
  unsigned char ucDevAddr, ucRegOffset, ucWrLen;
  unsigned char aucDataBuf[256];
  char *pcErrPtr;
  int iLoopCnt = 0;

  //
  // Get the device address
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucDevAddr = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);

  //
  // Get the register offset to be written
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucRegOffset = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
  aucDataBuf[iLoopCnt] = ucRegOffset;
  iLoopCnt++;

  //
  // Get the length of data to be written
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucWrLen = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);
  // RETERR_IF_TRUE(ucWrLen > sizeof(aucDataBuf));

  //
  // Get the bytes to be written
  //
  for (; iLoopCnt < ucWrLen + 1; iLoopCnt++) {
    //
    // Store the data to be written
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    aucDataBuf[iLoopCnt] =
        (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
  }
  //
  // Write the data values.
  //
  RET_IF_ERR(I2C_IF_Write(ucDevAddr, &aucDataBuf[0], ucWrLen + 1, 1));

  UART_PRINT("I2C Write To address complete\n\r");

  return SUCCESS;
}

//****************************************************************************
//
//! Parses the write command parameters and invokes the I2C APIs
//!
//! \param pcInpString pointer to the write command parameters
//!
//! This function
//!    1. Parses the write command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessWriteCommand(char *pcInpString) {
  unsigned char ucDevAddr, ucStopBit, ucLen;
  unsigned char aucDataBuf[256];
  char *pcErrPtr;
  int iRetVal, iLoopCnt;

  //
  // Get the device address
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucDevAddr = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);

  //
  // Get the length of data to be written
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucLen = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);
  // RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

  for (iLoopCnt = 0; iLoopCnt < ucLen; iLoopCnt++) {
    //
    // Store the data to be written
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    aucDataBuf[iLoopCnt] =
        (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
  }

  //
  // Get the stop bit
  //
  pcInpString = strtok(NULL, " ");
  RETERR_IF_TRUE(pcInpString == NULL);
  ucStopBit = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);

  //
  // Write the data to the specified address
  //
  iRetVal = I2C_IF_Write(ucDevAddr, aucDataBuf, ucLen, ucStopBit);
  if (iRetVal == SUCCESS) {
    UART_PRINT("I2C Write complete\n\r");
  } else {
    UART_PRINT("I2C Write failed\n\r");
    return FAILURE;
  }

  return SUCCESS;
}

//****************************************************************************
//
//! Parses the user input command and invokes the I2C APIs
//!
//! \param pcCmdBuffer pointer to the user command
//!
//! This function
//!    1. Parses the user command.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ParseNProcessCmd(char *pcCmdBuffer) {
  char *pcInpString;
  int iRetVal = FAILURE;

  pcInpString = strtok(pcCmdBuffer, " \n\r");
  if (pcInpString != NULL)

  {

    if (!strcmp(pcInpString, "read")) {
      //
      // Invoke the read command handler
      //
      iRetVal = ProcessReadCommand(pcInpString);
    } else if (!strcmp(pcInpString, "readreg")) {
      //
      // Invoke the readreg command handler
      //
      iRetVal = ProcessReadRegCommand(pcInpString);
    } else if (!strcmp(pcInpString, "writereg")) {
      //
      // Invoke the writereg command handler
      //
      iRetVal = ProcessWriteRegCommand(pcInpString);
    } else if (!strcmp(pcInpString, "write")) {
      //
      // Invoke the write command handler
      //
      iRetVal = ProcessWriteCommand(pcInpString);
    } else {
      UART_PRINT("Unsupported command\n\r");
      return FAILURE;
    }
  }

  return iRetVal;
}


static void SPIInit(void) {
    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Enable the SPI module clock
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);
    MAP_SPIReset(GSPI_BASE);

    // Configure SPI interface
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                        SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                        (SPI_SW_CTRL_CS |
                        SPI_4PIN_MODE |
                        SPI_TURBO_OFF |
                        SPI_CS_ACTIVEHIGH |
                        SPI_WL_8));

    // Enable SPI for communication
    MAP_SPIEnable(GSPI_BASE);

    //set CS GPIO pin to high initially
    GPIOPinWrite(GPIOA0_BASE, 0x80, 0x80);  //turn on P62 / CS|OC output signal

    Adafruit_Init(); //oled init
}

void drawHomeScreen() {
    //drawRoundRect(x, y, w, h, 5, color);
    //fillRoundRect(int x, int y, int w, int h, int r, unsigned int color) {
    int x = 5;
    int y = 5;
    int width = 45;
    int height = 45;
    int radius = 5;

    //draw stocks app"
    fillRoundRect(x, y, width, height, radius, BLUE);
    //vertical axis:
    drawLine(13, 14, 13, 40, BLACK);
    drawLine(14, 14, 14, 40, BLACK);
    //horizontal axis:
    drawLine(14, 39, 40, 39, BLACK);
    drawLine(14, 40, 40, 40, BLACK);

    //graph:
    drawLine(14, 33, 22, 23, BLACK);
    drawLine(14, 34, 22, 24, BLACK);
    drawLine(22, 25, 30, 35, BLACK);
    drawLine(22, 24, 30, 34, BLACK);
    drawLine(30, 33, 38, 23, BLACK);
    drawLine(30, 34, 38, 24, BLACK);
    drawTriangle(37, 21, 40, 27, 42, 19, BLACK);

    //draw msg app:
    fillRoundRect(x+75, y, width, height, radius, GREEN);
    fillRoundRect(x+82, y+10, 30, 25, 12, WHITE);
    fillTriangle(x+85, y+30, x+95, y+30, x+82, y+34, WHITE);


    //draw music app:
    fillRoundRect(x, y+75, width, height, radius, MAGENTA);
    fillCircle(x+10, y+108, 5, MAGENTA-10);
    drawLine(x+14, y+93, x+14, y+108, MAGENTA-10);
    drawLine(x+15, y+93, x+15, y+108, MAGENTA-10);

    fillCircle(x+30, y+105, 5, MAGENTA-10);
    drawLine(x+34, y+90, x+34, y+105, MAGENTA-10);
    drawLine(x+35, y+90, x+35, y+105, MAGENTA-10);

    drawLine(x+14, y+94, x+35, y+91, MAGENTA-10);
    drawLine(x+14, y+93, x+35, y+90, MAGENTA-10);
    drawLine(x+14, y+92, x+35, y+89, MAGENTA-10);
    drawLine(x+14, y+91, x+35, y+88, MAGENTA-10);
    drawLine(x+14, y+90, x+35, y+87, MAGENTA-10);
    drawLine(x+14, y+89, x+35, y+86, MAGENTA-10);
    drawLine(x+14, y+88, x+35, y+85, MAGENTA-10);
    drawLine(x+14, y+87, x+35, y+84, MAGENTA-10);
    drawLine(x+14, y+86, x+35, y+83, MAGENTA-10);

    //draw twitter:
    fillRoundRect(x+75, y+75, width, height, radius, CYAN);
    drawLine(x+75, y+75, x+75, y+95, CYAN-10);

}


 char corpName[50];
 char price[50];
 char exchange[50];
void app1() {       //stocks app
    int x = 0;
    int y = 118;
    int width = 128;
    int height = 10;

    fillScreen(BLUE);
    fillRect(x, y, width, height, WHITE);

    int i;
    char prompt[50];
    strcpy(prompt, "Type Ticker");
    for (i=0; i<strlen(prompt); i++) {
        fillRect((i)*10, 5, 10, 15, WHITE);
        drawChar(i*10, 5, prompt[i], BLACK, BLACK, 2);
    }

    receivedGet = 0;
}

void app2() {       //messaging app
    int x = 0;
    int y = 118;
    int width = 128;
    int height = 10;

    fillScreen(BLACK);

    //draw type bar:
    fillRect(x, y, width, height, WHITE);


    //fillRoundRect(65, 20, 65, 10, 3, BLUE);
    //fillRoundRect(0, 35, 65, 10, 3, WHITE);

}

void app3() {       //music app
    int x = 0;
    int y = 118;
    int width = 128;
    int height = 10;

    fillScreen(MAGENTA);

    //draw type bar:
    fillRect(x, y, width, height, WHITE);

}

void app4() {       //twitter(?) app
    int x = 0;
    int y = 118;
    int width = 128;
    int height = 10;

    fillScreen(CYAN);

    //draw type bar:
    fillRect(x, y, width, height, WHITE);

}
//***************************       MAIN        ********************************

char banner[100];
char receivedText[100];



//******************** INITIALIZATION FUNCTIONS *******************************

static void BoardInit(void) {
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

// Initializes SysTick Module
static void SysTickInit(void) {

    // configure the reset value for the systick countdown register
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    MAP_SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    MAP_SysTickIntEnable();

    // enable the systick module itself
    MAP_SysTickEnable();
}

char* extract_json_body(const char* http_response) {
    // Find the position of the end of the headers
    const char *header_end = strstr(http_response, "\r\n\r\n");
    if (header_end == NULL) {
        return NULL; // Failed to find the end of headers
    }

    // Move the pointer to the start of the JSON body
    header_end += 4; // Move past the "\r\n\r\n"

    // Find the last closing brace in the JSON body
    const char *json_end = strrchr(header_end, '}');
    if (json_end == NULL) {
        return NULL; // Failed to find the closing brace
    }

    // Calculate the length of the JSON body
    size_t json_length = json_end - header_end + 1;

    // Allocate memory for the JSON body (+1 for the null terminator)
    char *json_body = (char*)malloc(json_length + 1);
    if (json_body == NULL) {
        return NULL; // Memory allocation failed
    }

    // Copy the JSON body
    strncpy(json_body, header_end, json_length);
    json_body[json_length] = '\0'; // Null-terminate the string
    UART_PRINT("json body: %s\n", json_body);
    return json_body;
}
double stockPrice;

void extractJSON(const char * jsonString){
    jsonParser parser;
    jsonObj rootObj;
    jsonObj stateObj;
    jsonObj reportObj;
    jsonObj stockObj;
    char buffer[256];

    printf("JSON: \n %s\n", jsonString);

    rootObj = json_parser_init(&parser, jsonString);

    if (rootObj == JSON_INVALID_OBJECT) {
        UART_PRINT("Failed to initialize JSON parser\n");
        return;
    } else {
        UART_PRINT("Success\n");
    }

    stateObj = json_get_object(&parser, rootObj, "state");
    if (stateObj == JSON_INVALID_OBJECT) {
        printf("Failed to find 'reported' object\n");
        json_parser_deinit(&parser);
        return;
    }else {
        UART_PRINT("Found STATE OBJ\n");
    }


    reportObj = json_get_object(&parser, stateObj, "reported");
    if (reportObj == JSON_INVALID_OBJECT) {
        printf("Failed to find 'reported' object\n");
        json_parser_deinit(&parser);
        return;
        }else {
            UART_PRINT("Found REPORT OBJ\n");
     }

     stockObj = json_get_object(&parser, reportObj, "stock");
     if (stockObj == JSON_INVALID_OBJECT) {
         printf("Failed to find 'reported' object\n");
         json_parser_deinit(&parser);
             return;
         } else {
             UART_PRINT("Found stock OBJ\n");
         }
     stockPrice = json_object_get_double(&parser, stockObj, "price");

}





//****************************************************************************





//*****************************************************************************
//
//! Main
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
int count = 0;
int page = 0;
int y_msg = 15;
char banner[100];
char receivedText[100];

void main() {

  // Initialize board configurations
  BoardInit();

  // Configure the pinmux settings for the peripherals exercised
  PinMuxConfig();

  // Configuring UART
  InitTerm();

  ClearTerm();

  UART_PRINT("My terminal works!\n\r");

  //**************************SYSTICK TIMER************************
  // Enable SysTick
  SysTickInit();

  // Register Interrupt Handler
  MAP_GPIOIntRegister(GPIOA0_BASE, irTestHandler);

  //set to falling edge to measure pulse widths
  MAP_GPIOIntTypeSet(irPinConfig.port, irPinConfig.pin, GPIO_FALLING_EDGE);

  irIntCount=0;
  irIntFlag=0;

  unsigned long ulStatus = MAP_GPIOIntStatus(irPinConfig.port, false);

  //clear and enable interrupts on pin50:
  MAP_GPIOIntClear(irPinConfig.port, ulStatus);
  MAP_GPIOIntEnable(irPinConfig.port, irPinConfig.pin);


  //timer timeout initialization
  Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
  // set the interrupt
  Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, timeoutHandler);
  // start the timeout length in ms
  Timer_IF_Start(TIMERA0_BASE, TIMER_A, 1000);

  //******************************************************************

  // I2C Init
  I2C_IF_Open(I2C_MASTER_MODE_FST);

  // SPI Init
  SPIInit();

  unsigned long ulStatus2;

  // Register the interrupt handlers for switches
  GPIOIntRegister(GPIOA1_BASE, GPIOA1IntHandler); //GPIOA1BASE; SW3
  GPIOIntRegister(switch2.port, GPIOA2IntHandler); //GPIOA2 BASE; SW2

  // Configure rising edge interrupts on SW2 and SW3
  GPIOIntTypeSet(GPIOA1_BASE, 0x20, GPIO_RISING_EDGE);    // SW3
  GPIOIntTypeSet(switch2.port, switch2.pin, GPIO_RISING_EDGE);    // SW2

  ulStatus2 = MAP_GPIOIntStatus (GPIOA1_BASE, false);          //SW3
  GPIOIntClear(GPIOA1_BASE, ulStatus2);
  ulStatus2 = MAP_GPIOIntStatus (switch2.port, false);         //SW2
  GPIOIntClear(switch2.port, ulStatus2);           // clear interrupts on GPIOA2

  // clear global variables
  SW3_intcount=0;
  SW3_intflag=0;
  SW2_intcount=0;
  SW2_intflag=0;

  // Enable SW2 and SW3 interrupts
  MAP_GPIOIntEnable(GPIOA1_BASE, 0x20);
  MAP_GPIOIntEnable(switch2.port, switch2.pin);

  sendData = false;

  // Display the Banner

  // initialize global default app configuration
  g_app_config.host = SERVER_NAME;
  g_app_config.port = GOOGLE_DST_PORT;

  //Connect the CC3200 to the local access point
  lRetVal = connectToAccessPoint();
  //Set time so that encryption can be used
  lRetVal = set_time();
  if(lRetVal < 0) {
      UART_PRINT("Unable to set time in the device");
      LOOP_FOREVER();
  }
  //Connect to the website with TLS encryption
  lRetVal = tls_connect();
  if(lRetVal < 0) {
      ERR_PRINT(lRetVal);
  }

  sendData = false;

  Message("\n\n\n\r");
  Message("\t\t   ********************************************\n\r");
  Message("\t\t        CC3200 Final Lab TiltPod No Touch  \n\r");
  Message("\t\t   ********************************************\n\r");
  Message("\n\n\n\r");

  int x, y;
  int x_ball, y_ball;

  // centering in the screen
  y_ball = 63;
  x_ball = 63;
  int ball_size = 4;
  const char *exampleJson = "{\"state\":{\"desired\":{\"welcome\":\"aws-iot\",\"var\":\"Hello phone, meow\",\"stock\":\"AAPL\",\"message\":\"D\"},\"reported\":{\"welcome\":\"aws-iot\",\"stock\":{\"ticker\":\"AAPL\",\"name\":\"Apple Inc.\",\"price\":195.575,\"exchange\":\"NASDAQ\",\"updated\":1717695538}},\"delta\":{\"var\":\"Hello phone, meow\",\"stock\":\"AAPL\",\"message\":\"D\"}},\"metadata\":{\"desired\":{\"welcome\":{\"timestamp\":1715723915},\"var\":{\"timestamp\":1717640548},\"stock\":{\"timestamp\":1717695540},\"message\":{\"timestamp\":1717702833}},\"reported\":{\"welcome\":{\"timestamp\":1715723915},\"stock\":{\"ticker\":{\"timestamp\":1717695541},\"name\":{\"timestamp\":1717695541},\"price\":{\"timestamp\":1717695541},\"exchange\":{\"timestamp\":1717695541},\"updated\":{\"timestamp\":1717695541}}}},\"version\":55,\"timestamp\":1717702995}";
  extractJSON(exampleJson);

  unsigned int bg = WHITE;

  //start up oled with homescreen
  fillScreen(WHITE);
  drawHomeScreen();

  while (FOREVER) {
      if (SW3_intflag) {
          SW3_intflag=0;  // clear flag
          Report("SW3 ints = %d\r\n", SW3_intcount);
          page = 0;
          y_ball = 63;
          x_ball = 63;
          fillCircle(x_ball, y_ball, ball_size, bg); //clear ball before bg change
          bg = WHITE;
          fillScreen(WHITE);
          drawHomeScreen();
      }


      if (SW2_intflag) {
          SW2_intflag=0;  // clear flag
          Report("SW2 ints = %d\r\n", SW2_intcount);

          if (page == 0) {    //only on home screen
              // app region checking
              if (x_ball >= 0 && x_ball <= 50) {  //left side
                  if (y_ball >=0 && y_ball <= 50) {   //top left
                      page = 1;
                  } else if (y_ball >= 75 && y_ball <= 127) {   //bottom left
                      page = 3;
                  }
              } else if (x_ball >= 75 && x_ball <= 127) { //right side
                  if (y_ball >=0 && y_ball <= 50) {   //top right
                      page = 2;
                  } else if (y_ball >= 75 && y_ball <= 127) {   //top right
                      page = 4;
                  }
              }
          }

          if (page == 1) {

              if (receivedGet) {     //after info pops up
                  if (x_ball >= 34 && x_ball <= 84) {
                      if (y_ball >= 70 && y_ball <= 100) {
                        page = 5;
                        receivedGet = 0;
                      }
                  }
              }
          }

          if (page == 2) {
              y_msg = 15; //reset
          }

          switch (page) {
              case 0: {
                  break;
              }
              case 1: {             //stocks app
                  bg = BLUE;
                  y_ball = 0;
                  x_ball = 127;
                  app1();
                  appMode = STOCKS_MODE;
                  break;
              }
              case 2: {              //messaging app
                  //printf("rendering message app\n");
                  bg = BLACK;
                  y_ball = 0;
                  x_ball = 0;
                  app2();
                  appMode = MESSAGES_MODE;
                  break;
              }
              case 3: {             //music app:
                  bg = MAGENTA;
                  y_ball = 0;
                  x_ball = 0;
                  app3();
                  break;
              }
              case 4: {             //twitter(?) app
                  bg = CYAN;
                  y_ball = 0;
                  x_ball = 0;
                  app4();
                  break;
              }
              case 5: {             //congrats page for buying on stock app
                  bg = YELLOW;
                  fillScreen(YELLOW);
                  y_ball = 0;
                  x_ball = 0;
                  strcpy(banner, "Congrats!");
                  int i;
                  for (i=0; i<strlen(banner); i++) {
                      fillRect((i)*13, 10, 13, 20, YELLOW);
                      drawChar(i*13, 10, banner[i], BLACK, BLACK, 3);
                  }
                  strcpy(banner, "You bought:");
                  for (i=0; i<strlen(banner); i++) {
                      fillRect((i)*10, 40, 10, 15, YELLOW);
                      drawChar(i*10, 40, banner[i], BLACK, BLACK, 2);
                  }
                  for (i=0; i<strlen(corpName); i++) {
                      fillRect((i)*10, 80, 10, 15, YELLOW);
                      drawChar(i*10, 80, corpName[i], BLACK, BLACK, 2);
                  }
                  page = -1;
                  break;
              }
          }

      }

      // clearing the previous position
      fillCircle(x_ball, y_ball, ball_size, bg);
      //drawHomeScreen();

      // For some reason this needs to be created in the while loop....
      char x_command[23] = "readreg 0x18 0x3 1 \n\r";
      char y_command[23] = "readreg 0x18 0x5 1 \n\r";

      // Calling our modified functions to get sensor data
      x = ParseNProcessCmd(x_command);
      y = ParseNProcessCmd(y_command);

      // Scale it
      x /= 6;
      y /= 6;

      x_ball += x;
      y_ball += y;


      count++;
      if (page == 0 && count % 5 == 0) {
          drawHomeScreen();
          count = 0;
      }


      // boundary checking for x
      if (x_ball >= (127 - ball_size)) {
        x_ball = 127 - ball_size;
      } else if (x_ball <= ball_size) {
        x_ball = ball_size;
      }

      // boundary checking for y
      if (y_ball >= (127 - ball_size)) {
        y_ball = 127 - ball_size;
      } else if (y_ball <= ball_size) {
        y_ball = ball_size;
      }

      // render ball
     fillCircle(x_ball, y_ball, ball_size, RED);
     if (renderBuffer) {
         //fillRect((outlength)*10, 118, 10, 15, WHITE);
         //drawChar(outlength*10, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
         fillRect((outlength)*5, 118, 5, 15, WHITE);
         drawChar(outlength*5, 118, outgoingMsgBuffer[outlength], BLACK, BLACK, 1);
         renderBuffer = 0;
     }
     if (deleteFlag) {
         fillRect((outlength)*5, 118, 5, 15, WHITE);
         deleteFlag = 0;
     }

     //strcpy(corpName, "Apple Inc.");
     //strcpy(price, "195.87");

     if (receivedGet) {
         if (page == 1) {
             fillRect(0, 5, 128, 15, BLUE); //overwrite prompt
              int i;
              int length = strlen(corpName);
              for (i=0; i<length; i++) {
                  fillRect((i)*10, 5, 10, 15, WHITE);
                  drawChar(i*10, 5, corpName[i], BLACK, BLACK, 2);
              }

              length = strlen(price);
              for (i=0; i<length; i++) {
                  fillRect((i)*10, 25, 10, 15, WHITE);
                  drawChar(i*10, 25, price[i], BLACK, BLACK, 2);
              }

              length = strlen(exchange);
              for (i=0; i<length; i++) {
                fillRect((i)*10, 45, 10, 15, WHITE);
                drawChar(i*10, 45, exchange[i], BLACK, BLACK, 2);
              }

              fillRect(34, 70, 50, 30, GREEN);
              drawChar(44, 77, 'B', BLACK, BLACK, 2);
              drawChar(54, 77, 'U', BLACK, BLACK, 2);
              drawChar(64, 77, 'Y', BLACK, BLACK, 2);
         }

         if (page == 2) {
             strcpy(receivedText, "Hi!"); //testing
               int i;
               //fillRoundRect(0, 35, 65, 10, 3, WHITE);
               for (i=0; i<strlen(receivedText); i++) {
                   fillRoundRect((i)*5, y_msg, 65, 10, 3, WHITE);
                   drawChar(i*5, y_msg, receivedText[i], BLACK, BLACK, 1);
               }
               y_msg += 15;
               receivedGet = 0;
         }

     }


     if(sendData) {
         //http_post(lRetVal);
         sendData = false;

         if (page == 2) {
             http_post(lRetVal);
             strcpy(banner, (const char *)outgoingMsgBuffer);
              int i;
              for (i=0; i<strlen(banner); i++) {
                  fillRoundRect(65+(i)*5, y_msg, 65, 10, 3, BLUE);
                  drawChar(65+i*5, y_msg, banner[i], BLACK, BLACK, 1);
              }
              y_msg += 15;

              fillRect(0, 118, 128, 10, WHITE); //reset type bar
              //reset buffer:
              for (i=0; i<outlength; i++) {
                  outgoingMsgBuffer[i] = '\0';
              }
              outlength = 0;

              receivedGet = 1;

         }

         //below should be inside http_post():
         if (page == 1) {
             http_post(lRetVal);
             MAP_UtilsDelay(8000000 * 7);
             http_get(lRetVal);


             strcpy(corpName, (const char *)outgoingMsgBuffer); //replace outMBffr with actual corpname
             sprintf(price, "%f", stockPrice);
             strcpy(exchange, "NASDAQ");
             receivedGet = 1;

             fillRect(0, 118, 128, 10, WHITE); //reset type bar
             //reset buffer:
             int i;
             for (i=0; i<outlength; i++) {
                 outgoingMsgBuffer[i] = '\0';
             }
             outlength = 0;
         }
     }
  }
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

static int http_post(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    //const volatile unsigned char* buffer;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    int dataLength = 0;
    if(appMode == MESSAGES_MODE){
        dataLength += strlen(MESSAGE_HEADER);
    } else if (appMode == STOCKS_MODE){
        dataLength += strlen(STOCK);
    }
    dataLength += strlen((const char *)outgoingMsgBuffer);
    dataLength += strlen(DATA3);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

    if(appMode ==  MESSAGES_MODE){
        strcpy(pcBufHeaders, MESSAGE_HEADER);
        pcBufHeaders += strlen(MESSAGE_HEADER);
    } else if (appMode == STOCKS_MODE){
        strcpy(pcBufHeaders, STOCK);
        pcBufHeaders += strlen(STOCK);
    }
    //data2
    sprintf(pcBufHeaders, "%s", outgoingMsgBuffer);
    pcBufHeaders += strlen((const char *)outgoingMsgBuffer);
    //data3
    strcpy(pcBufHeaders, DATA3);
    pcBufHeaders += strlen(DATA3);

    int testDataLength = strlen(pcBufHeaders);
    int test = strlen(acSendBuff);

    UART_PRINT(acSendBuff);

    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}

static int http_get(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, GETHEADER);
    pcBufHeaders += strlen(GETHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    UART_PRINT(acSendBuff);

    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("GET failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    char *jsonBody = extract_json_body(acRecvbuff);
    //print the JSON we got
    extractJSON((const char *)jsonBody);

    return 0;
}
