#include "display_ili9340.h"

MCUFRIEND_kbv tft_ili9340;

void ILI9340_Init(void)
{
    tft_ili9340.reset();
    uint16_t ID;

    ID = tft_ili9340.readID();
#ifdef SERIAL_DEBUG_ENABLED
    Serial.print(F("TFT original ID = 0x"));
    Serial.println(ID, HEX);
#endif
    if (ID == 0xE300)
    {
        ID = 0x9340; // "Weird" Banggood shield, forcing ILI9340 setup
    }
    tft_ili9340.begin(ID);
    tft_ili9340.setRotation(1);
    Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
    tft_ili9340.setCursor(0, 40);
    tft_ili9340.setTextColor(ILI9340_WHITE);
    tft_ili9340.setFont(&FreeSans24pt7b);
    tft_ili9340.setTextSize(1);
    tft_ili9340.println(F("  Linea Mini "));
    tft_ili9340.println(F("  Brew Timer "));
    tft_ili9340.println(F("  Version 1.1"));
    delay(1000);
}

/**
 * @brief Updates and then displays the TimeCounterStr string on the ILI9340 TFT screen, format: MM:SS
 * @param need_Display_Clear : is display clear needed?
 * @param need_Display_Stopped : is visualisation of sopped timer needed?
 */
void display_Timer_On_ILI9340(char *pCounterStr, bool need_Display_Clear, bool need_Display_Stopped)
{
    if (need_Display_Clear)
    {
        Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
    }
    tft_ili9340.setFont(&FreeSans24pt7b);
    tft_ili9340.setTextSize(2);
    if (need_Display_Stopped)
    {
        tft_ili9340.setTextColor(ILI9340_DARK_GRAY);
    }
    else
    {
        tft_ili9340.setTextColor(ILI9340_WHITE);
    }
    tft_ili9340.fillRect(ILI9340_TIMER_POS_X, ILI9340_TIMER_POS_Y - ILI9340_TIMER_HEIGTH, ILI9340_TIMER_WIDTH,
                         ILI9340_TIMER_HEIGTH + ILI9340_TIMER_Y_OFFSET, ILI9340_DISPLAY_CLEAR_COLOR);
    tft_ili9340.setCursor(ILI9340_TIMER_POS_X, ILI9340_TIMER_POS_Y);
    tft_ili9340.println(pCounterStr);
}

void display_Temperature_On_ILI9340(char *pTemperatureStr)
{
    tft_ili9340.fillRect(ILI9340_TEMPERATURE_POS_X, ILI9340_TEMPERATURE_POS_Y - ILI9340_TEMPERATURE_HEIGTH,
                         ILI9340_TEMPERATURE_WIDTH, ILI9340_TEMPERATURE_HEIGTH + ILI9340_TEMPERATURE_Y_OFFSET,
                         ILI9340_DISPLAY_CLEAR_COLOR);
    tft_ili9340.setFont(&FreeSans18pt7b);
    tft_ili9340.setTextSize(2);
    tft_ili9340.setTextColor(ILI9340_ORANGE);
    tft_ili9340.setCursor(ILI9340_TEMPERATURE_POS_X, ILI9340_TEMPERATURE_POS_Y);
    tft_ili9340.print(pTemperatureStr);
    tft_ili9340.print(F(" *C"));
}

void display_Milli_Volt_On_ILI9340(char *pMilliVoltStr)
{
    tft_ili9340.setFont(&FreeSans12pt7b);
    tft_ili9340.setTextSize(1);
    tft_ili9340.fillRect(ILI9340_MILLI_VOLT_POS_X, ILI9340_MILLI_VOLT_POS_Y - ILI9340_MILLI_VOLT_HEIGTH,
                         ILI9340_MILLI_VOLT_WIDTH, ILI9340_MILLI_VOLT_HEIGTH + ILI9340_MILLI_VOLT_Y_OFFSET,
                         ILI9340_DISPLAY_CLEAR_COLOR);
    tft_ili9340.setTextColor(ILI9340_MAGENTA);
    tft_ili9340.setCursor(ILI9340_MILLI_VOLT_POS_X, ILI9340_MILLI_VOLT_POS_Y);
    tft_ili9340.print(F("Th Voltage: "));
    tft_ili9340.print(pMilliVoltStr);
    tft_ili9340.print(F(" mV"));
}

void Display_Clear_ILI9340(uint16_t color)
{
    // Serial.println("Display_Clear_ILI9340");
    tft_ili9340.fillScreen(color);
    tft_ili9340.setCursor(0, 0);
    delay(50);
}
