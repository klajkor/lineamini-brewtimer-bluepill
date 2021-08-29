#pragma once

//#include "display.h"

#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define OLED_I2C_ADDR 0x3C /* OLED module I2C address */
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

// Set up OLED display
extern SSD1306AsciiWire oled_ssd1306_display;

void Ssd1306_Oled_Init(void);
void display_Timer_On_Ssd1306(char *pCounterStr, bool need_Display_Clear, bool need_Display_Stopped);
void display_Temperature_On_Ssd1306(char *pTemperatureStr);
