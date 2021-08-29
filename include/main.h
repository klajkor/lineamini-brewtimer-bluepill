#pragma once

#include <SPI.h>
#include <Wire.h>
#include <math.h>

// uncomment the line below if you would like to have debug messages
//#define SERIAL_DEBUG_ENABLED 1

// uncomment the line below if you would like to use SSD1306 OLED display
#define SSD1306_ENABLED 1

// uncomment the line below if you would like to use ILI9340 TFT display
#define ILI9340_ENABLED 1

// Adafruit BusIO library also needs to be installed on PlatformIO!

extern int  iSecCounter1;
extern int  iMinCounter1;
extern char TimeCounterStr[];
extern char SecondsCounterStr[];

/* Function declarations */

void StateMachine_counter1(void);
void StateMachine_Manual_Switch(void);
void StateMachine_Reed_Switch(void);
void StateMachine_Status_Led(void);

void update_TimeCounterStr(int tMinutes, int tSeconds);

void Gpio_Init(void);
void Status_Led_Off(void);
void Status_Led_On(void);