#pragma once

#include <Arduino.h>

#include "main.h"
#include "temperature_meter.h"

#ifdef SSD1306_ENABLED
#include "display_ssd1306.h"
#endif // #ifdef SSD1306_ENABLED

#ifdef ILI9340_ENABLED
#include "display_ili9340.h"
#endif

#define DISPLAY_CLEAR_TRUE true
#define DISPLAY_CLEAR_FALSE false
#define DISPLAY_STOPPED_TRUE true
#define DISPLAY_STOPPED_FALSE false

#define DISPLAY_STATE_RESET 0
#define DISPLAY_STATE_TIMER_RUNNING 1
#define DISPLAY_STATE_TIMER_STOPPED 2
#define DISPLAY_STATE_HOLD_TIMER_ON 3
#define DISPLAY_STATE_TEMPERATURE 4
#define DISPLAY_STATE_DO_NOTHING 5

#define DISPLAY_TEMPERATURE_FREQ_MILLISEC (500U)

// State Machine Display variables
extern int state_Display;

void StateMachine_Display(void);
void display_Timer_On_All(bool need_Display_Clear, bool need_Display_Stopped);
void Display_Running_Timer(void);
void Display_Stopped_Timer(void);
void Display_Temperature(void);
void Display_Clear(void);
