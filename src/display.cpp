#include "display.h"

// State Machine Display Global variables
int state_Display = 0;

// State Machine Display Local variables
static unsigned long t_Display = DISPLAY_STATE_RESET;
static unsigned long t_0_Display = 0;
static unsigned long delay_For_Stopped_Timer = 5000; // millisec

void display_Timer_On_All(bool need_Display_Clear, bool need_Display_Stopped)
{
    update_TimeCounterStr(iMinCounter1, iSecCounter1);
#ifdef SSD1306_ENABLED
    display_Timer_On_Ssd1306(SecondsCounterStr, need_Display_Clear, need_Display_Stopped);
#endif
#ifdef ILI9340_ENABLED
    display_Timer_On_ILI9340(SecondsCounterStr, need_Display_Clear, need_Display_Stopped);
#endif
}

void StateMachine_Display(void)
{
    unsigned long display_hold_timer;

    switch (state_Display)
    {
    case DISPLAY_STATE_RESET:
        Display_Stopped_Timer();
        t_0_Display = millis();
        state_Display = DISPLAY_STATE_TEMPERATURE;
        break;

    case DISPLAY_STATE_TIMER_RUNNING:
        Display_Running_Timer();
        // Display_Temperature();
        state_Display = DISPLAY_STATE_TEMPERATURE;
        break;

    case DISPLAY_STATE_TIMER_STOPPED:
        Display_Stopped_Timer();
        t_0_Display = millis();
        state_Display = DISPLAY_STATE_TEMPERATURE;
        break;

    case DISPLAY_STATE_HOLD_TIMER_ON:
        t_Display = millis();
        display_hold_timer = t_Display - t_0_Display;
        if (display_hold_timer > delay_For_Stopped_Timer &&
            ((display_hold_timer % DISPLAY_TEMPERATURE_FREQ_MILLISEC) == 0))
        {
            state_Display = DISPLAY_STATE_TEMPERATURE;
        }
        else
        {
#ifdef ILI9340_ENABLED
            if ((display_hold_timer % DISPLAY_TEMPERATURE_FREQ_MILLISEC) == 0)
            {
                state_Display = DISPLAY_STATE_TEMPERATURE;
            }
#endif
        }
        break;

    case DISPLAY_STATE_TEMPERATURE:
        Display_Temperature();
        if (virtual_Reed_Switch == VIRT_REED_SWITCH_OFF)
        {
            state_Display = DISPLAY_STATE_HOLD_TIMER_ON;
        }
        else
        {
            state_Display = DISPLAY_STATE_DO_NOTHING;
        }
        break;

    case DISPLAY_STATE_DO_NOTHING:
        break;
    }
}

void Display_Running_Timer(void)
{
    display_Timer_On_All(DISPLAY_CLEAR_FALSE, DISPLAY_STOPPED_FALSE);
}

void Display_Stopped_Timer(void)
{
    display_Timer_On_All(DISPLAY_CLEAR_FALSE, DISPLAY_STOPPED_TRUE);
}

void Display_Temperature(void)
{
#ifdef SSD1306_ENABLED
    display_Temperature_On_Ssd1306(temperature_str_V2);
#endif
#ifdef ILI9340_ENABLED
    display_Temperature_On_ILI9340(temperature_str_V2);
    display_Milli_Volt_On_ILI9340(milli_volt_str);
#endif
}

void Display_Clear(void)
{
#ifdef SSD1306_ENABLED
    oled_ssd1306_display.clear();
#endif
#ifdef ILI9340_ENABLED
    Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
#endif
}
