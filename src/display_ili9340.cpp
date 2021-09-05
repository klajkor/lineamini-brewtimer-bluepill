#include "display_ili9340.h"

// Screen coordinates
#define ILI9340_TIMER_POS_X (25)
#define ILI9340_TIMER_POS_Y (110)
#define ILI9340_TIMER_WIDTH (110)
#define ILI9340_TIMER_HEIGTH (75)
#define ILI9340_TIMER_SEC_POS_X (ILI9340_TIMER_POS_X + ILI9340_TIMER_WIDTH)
#define ILI9340_TIMER_SEC_POS_Y (ILI9340_TIMER_POS_Y)
#define ILI9340_TIMER_SEC_WIDTH (72)
#define ILI9340_TIMER_SEC_HEIGTH (ILI9340_TIMER_HEIGTH)
#define ILI9340_TEMPERATURE_POS_X (30)
#define ILI9340_TEMPERATURE_POS_Y (250)
#define ILI9340_TEMPERATURE_WIDTH (220)
#define ILI9340_TEMPERATURE_HEIGTH (60)
#define ILI9340_MILLI_VOLT_POS_X (30)
#define ILI9340_MILLI_VOLT_POS_Y (300)
#define ILI9340_MILLI_VOLT_WIDTH (220)
#define ILI9340_MILLI_VOLT_HEIGTH (20)

// Assign human-readable names to some common 16-bit color values:
#define ILI9340_BLACK (0x0000)
#define ILI9340_NAVY (0x000F)
#define ILI9340_BLUE (0x001F)
#define ILI9340_RED (0xF800)
#define ILI9340_GREEN (0x07E0)
#define ILI9340_CYAN (0x07FF)
#define ILI9340_MAGENTA (0xF81F)
#define ILI9340_ORANGE (0xFDA0)
#define ILI9340_YELLOW (0xFFE0)
#define ILI9340_LIGHT_GRAY (0xBDF7)
#define ILI9340_DARK_GRAY (0x7BEF)
#define ILI9340_WHITE (0xFFFF)
#define ILI9340_BACKGROUND_COLOR (ILI9340_BLACK)

MCUFRIEND_kbv tft_ili9340;

static sprite_ili9340_s sprite_timer_active = {.start_x = ILI9340_TIMER_POS_X,
                                               .start_y = ILI9340_TIMER_POS_Y,
                                               .width = ILI9340_TIMER_WIDTH,
                                               .height = ILI9340_TIMER_HEIGTH,
                                               .background_color = ILI9340_BACKGROUND_COLOR,
                                               .font_color = ILI9340_WHITE,
                                               .font = &FreeSans24pt7b,
                                               .text_size = 2};

static sprite_ili9340_s sprite_timer_inactive = {.start_x = ILI9340_TIMER_POS_X,
                                                 .start_y = ILI9340_TIMER_POS_Y,
                                                 .width = ILI9340_TIMER_WIDTH,
                                                 .height = ILI9340_TIMER_HEIGTH,
                                                 .background_color = ILI9340_BACKGROUND_COLOR,
                                                 .font_color = ILI9340_DARK_GRAY,
                                                 .font = &FreeSans24pt7b,
                                                 .text_size = 2};

static sprite_ili9340_s sprite_timer_sec = {.start_x = ILI9340_TIMER_SEC_POS_X,
                                            .start_y = ILI9340_TIMER_SEC_POS_Y,
                                            .width = ILI9340_TIMER_SEC_WIDTH,
                                            .height = ILI9340_TIMER_SEC_HEIGTH,
                                            .background_color = ILI9340_BACKGROUND_COLOR,
                                            .font_color = ILI9340_WHITE,
                                            .font = &FreeSans24pt7b,
                                            .text_size = 1};

static sprite_ili9340_s sprite_temperature = {.start_x = ILI9340_TEMPERATURE_POS_X,
                                              .start_y = ILI9340_TEMPERATURE_POS_Y,
                                              .width = ILI9340_TEMPERATURE_WIDTH,
                                              .height = ILI9340_TEMPERATURE_HEIGTH,
                                              .background_color = ILI9340_BACKGROUND_COLOR,
                                              .font_color = ILI9340_ORANGE,
                                              .font = &FreeSans24pt7b,
                                              .text_size = 1};

static sprite_ili9340_s sprite_millivolt = {.start_x = ILI9340_MILLI_VOLT_POS_X,
                                            .start_y = ILI9340_MILLI_VOLT_POS_Y,
                                            .width = ILI9340_MILLI_VOLT_WIDTH,
                                            .height = ILI9340_MILLI_VOLT_HEIGTH,
                                            .background_color = ILI9340_BACKGROUND_COLOR,
                                            .font_color = ILI9340_MAGENTA,
                                            .font = &FreeSans12pt7b,
                                            .text_size = 1};

void ILI9340_Init(void)
{
    uint16_t ID;

    tft_ili9340.reset();
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
    tft_ili9340.setRotation(0);
    display_clear_ili9340();
    tft_ili9340.setCursor(0, 0);
    tft_ili9340.setTextColor(ILI9340_WHITE);
    tft_ili9340.setCursor(0, 50);
    tft_ili9340.setFont(&FreeSans24pt7b);
    tft_ili9340.setTextSize(1);
    tft_ili9340.println(F("Linea Mini "));
    tft_ili9340.println(F("Brew Timer "));
    tft_ili9340.println(F("Version 1.1"));
    delay(1000);
}

/**
 * @brief Updates and then displays the TimeCounterStr string on the ILI9340 TFT screen, format: MM:SS
 * @param need_Display_Clear : is display clear needed?
 * @param need_Display_Stopped : is visualisation of sopped timer needed?
 */
void display_Timer_On_ILI9340(char *pCounterStr, bool need_Display_Clear, bool need_Display_Stopped)
{
    char sec_str[] = "sec";
    if (need_Display_Stopped)
    {
        sprite_clear_ili9340(&sprite_timer_inactive);
        sprite_print_ili9340(&sprite_timer_inactive, pCounterStr);
    }
    else
    {
        sprite_clear_ili9340(&sprite_timer_active);
        sprite_print_ili9340(&sprite_timer_active, pCounterStr);
    }
    sprite_clear_ili9340(&sprite_timer_sec);
    sprite_print_ili9340(&sprite_timer_sec, sec_str);
}

void display_Temperature_On_ILI9340(char *pTemperatureStr)
{
    char temp_str[10];
    snprintf(temp_str, 9, "%s *C", pTemperatureStr);
    sprite_clear_ili9340(&sprite_temperature);
    sprite_print_ili9340(&sprite_temperature, temp_str);
}

void display_Milli_Volt_On_ILI9340(char *pMilliVoltStr)
{
    char temp_str[22];
    snprintf(temp_str, 21, "Th Volt: %s mV", pMilliVoltStr);
    sprite_clear_ili9340(&sprite_millivolt);
    sprite_print_ili9340(&sprite_millivolt, temp_str);
}

void display_clear_ili9340(void)
{
    // Serial.println("display_clear_ili9340");
    tft_ili9340.fillScreen(ILI9340_BACKGROUND_COLOR);
    tft_ili9340.setCursor(0, 0);
    delay(20);
}

void sprite_clear_ili9340(sprite_ili9340_s *pSprite_i)
{
    tft_ili9340.fillRect(pSprite_i->start_x, pSprite_i->start_y - pSprite_i->height + 1, pSprite_i->width,
                         pSprite_i->height + 2, pSprite_i->background_color);
}

void sprite_print_ili9340(sprite_ili9340_s *pSprite_i, char *pString_i)
{
    tft_ili9340.setFont(pSprite_i->font);
    tft_ili9340.setTextSize(pSprite_i->text_size);
    tft_ili9340.setTextColor(pSprite_i->font_color);
    tft_ili9340.setCursor(pSprite_i->start_x, pSprite_i->start_y - 1);
    tft_ili9340.print(pString_i);
}
