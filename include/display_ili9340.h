#pragma once

//#define USE_MY_BLUEPILL
#include <MCUFRIEND_kbv.h> // for ILI9340
//#include <mcufriend_special.h>
#include <Fonts/FreeSans12pt7b.h>
//#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t background_color;
} screen_ili9340_s;

typedef struct
{
    uint16_t       start_x;
    uint16_t       start_y;
    uint16_t       width;
    uint16_t       height;
    uint16_t       background_color;
    uint16_t       font_color;
    const GFXfont *font;
    uint8_t        text_size;
} sprite_ili9340_s;

//	MCUFRIEND_kbv(int CS=A3, int RS=A2, int WR=A1, int RD=A0, int RST=A4); //shield wiring
extern MCUFRIEND_kbv tft_ili9340;

void ILI9340_Init(void);
void display_Timer_On_ILI9340(char *pCounterStr, bool need_Display_Clear, bool need_Display_Stopped);
void display_Temperature_On_ILI9340(char *pTemperatureStr);
void display_Milli_Volt_On_ILI9340(char *pMilliVoltStr);
void display_clear_ili9340(void);
void sprite_clear_ili9340(sprite_ili9340_s *pSprite_i);
void sprite_print_ili9340(sprite_ili9340_s *pSprite_i, char *pString_i);
