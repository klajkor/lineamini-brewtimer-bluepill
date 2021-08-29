#pragma once

//#define USE_MY_BLUEPILL
#include <MCUFRIEND_kbv.h> // for ILI9340
//#include <mcufriend_special.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

// Assign human-readable names to some common 16-bit color values:
#define ILI9340_BLACK 0x0000
#define ILI9340_NAVY 0x000F
#define ILI9340_BLUE 0x001F
#define ILI9340_RED 0xF800
#define ILI9340_GREEN 0x07E0
#define ILI9340_CYAN 0x07FF
#define ILI9340_MAGENTA 0xF81F
#define ILI9340_ORANGE 0xFDA0
#define ILI9340_YELLOW 0xFFE0
#define ILI9340_LIGHT_GRAY 0xBDF7
#define ILI9340_DARK_GRAY 0x7BEF
#define ILI9340_WHITE 0xFFFF
#define ILI9340_DISPLAY_CLEAR_COLOR ILI9340_BLACK

// Screen coordinates
#define ILI9340_TIMER_POS_X 45
#define ILI9340_TIMER_POS_Y 80
#define ILI9340_TIMER_WIDTH 240
#define ILI9340_TIMER_HEIGTH 80
#define ILI9340_TIMER_Y_OFFSET 5
#define ILI9340_TEMPERATURE_POS_X 25
#define ILI9340_TEMPERATURE_POS_Y 170
#define ILI9340_TEMPERATURE_WIDTH 310
#define ILI9340_TEMPERATURE_HEIGTH 60
#define ILI9340_TEMPERATURE_Y_OFFSET 3
#define ILI9340_MILLI_VOLT_POS_X 45
#define ILI9340_MILLI_VOLT_POS_Y 215
#define ILI9340_MILLI_VOLT_WIDTH 250
#define ILI9340_MILLI_VOLT_HEIGTH 20
#define ILI9340_MILLI_VOLT_Y_OFFSET 2

//	MCUFRIEND_kbv(int CS=A3, int RS=A2, int WR=A1, int RD=A0, int RST=A4); //shield wiring
extern MCUFRIEND_kbv tft_ili9340;

void ILI9340_Init(void);
void display_Timer_On_ILI9340(char *pCounterStr, bool need_Display_Clear, bool need_Display_Stopped);
void display_Temperature_On_ILI9340(char *pTemperatureStr);
void display_Milli_Volt_On_ILI9340(char *pMilliVoltStr);
void Display_Clear_ILI9340(uint16_t color);