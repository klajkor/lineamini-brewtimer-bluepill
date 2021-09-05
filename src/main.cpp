/**
 * Arduino Brew Timer and Temperature Display for LaMarzocco Linea Mini espresso machine
 *
 *
 * @author Robert Klajko
 * @url https://github.com/klajkor/lineamini-brewtimer-bluepill.git
 *
 * Board: STM32 BluePill (generic STM32F103C8) with Arduino Framework
 *
 * Extension modules and hw used:
 *  - SSD1306 OLED display, I2C - optional, please see source code for feature switch (SSD1306_ENABLED)
 *  - ILI9340 TFT display, SPI and 8 bit data - optional, please see source code for feature switch (ILI9340_ENABLED)
 *  - reed switch, sensing magnet valve sc
 *  - INA219, I2C
 * Libraries used:
 *  - SSD1306Ascii by Bill Greiman - Copyright (c) 2019, Bill Greiman
 *  - Adafruit INA219 by Adafruit - Copyright (c) 2012, Adafruit Industries
 *  - MCUFRIEND_kbv - Copyright (c) 2020, David Prentice
 *
 * Wiring of reed switch:
 *  - PB3
 *  - GND
 *
 * Wiring of I2C devices (INA219, SSD1306):
 *  - SDA - PB11 (I2C2 used due to pins required for ILI9340 TFT)
 *  - SCL - PB10 (I2C2 used due to pins required for ILI9340 TFT)
 *
 * Wiring of ILI9340:
 * - LCD pins  |D7 |D6 |D5 |D4 |D3 |D2 |D1 |D0 | |RD |WR |RS |CS |RST| Not in use: |SD_SS|SD_DI|SD_DO|SD_SCK|
 * - STM32 pin |PA7|PA6|PA5|PA4|PA3|PA2|PA1|PA0| |PB0|PB6|PB7|PB8|PB9| Not in use: |PA15 |PB5  |PB4  |PB3   |
 * **ALT-SPI1**
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "main.h"
#include "display.h"
#include "temperature_meter.h"
#include <Arduino.h>

#define SW_ON_LEVEL LOW   /* Switch on level */
#define SW_OFF_LEVEL HIGH /* Switch off level */

#define LED_ON LOW   /* LED Switch on level */
#define LED_OFF HIGH /* LED Switch off level */

#define COUNTER_STATE_RESET 0
#define COUNTER_STATE_DISABLED 1
#define COUNTER_STATE_START 2
#define COUNTER_STATE_COUNTING 3
#define COUNTER_STATE_STOP 4

#define REED_SWITCH_STATE_RESET 0
#define REED_SWITCH_STATE_START_TIMER 1
#define REED_SWITCH_STATE_STOP_TIMER 2
#define REED_SWITCH_STATE_READ_PIN 3
#define REED_SWITCH_STATE_ROTATE_BIN_COUNTER 4
#define REED_SWITCH_STATE_SET_LOGIC_SWITCH 5

// Top Level Variables:
int DEBUG = 1; // Set to 1 to enable serial monitor debugging info

// Switch Variables - "sw1"
int           state_Manual_Switch = 0;      // The actual ~state~ of the state machine
int           state_prev_Manual_Switch = 0; // Remembers the previous state (useful to know when the state has changed)
int           pin_Manual_Switch = PB4;      // Input/Output (IO) pin for the switch, 10 = pin 10 a.k.a. D10
int           value_Manual_Switch = 0;      // Value of the switch ("HIGH" or "LOW")
unsigned long t_Manual_Switch = 0;          // Typically represents the current time of the switch
unsigned long t_0_Manual_Switch = 0;        // The time that we last passed through an interesting state
unsigned long bounce_delay_Manual_Switch = 20; // The delay to list for bouncing

// Switch Variables - "Reed_Switch"
int state_Reed_Switch = 0; // The actual ~state~ of the state machine
// int pin_Reed_Switch = 3;                   //Input/Output (IO) pin for the switch <- old config!
int           pin_Reed_Switch = PB3;        // Input/Output (IO) pin for the switch, 10 = pin 10 a.k.a. D10
int           value_Reed_Switch = 0;        // Value of the switch ("HIGH" or "LOW")
unsigned long t_Reed_Switch = 0;            // Typically represents the current time of the switch
unsigned long t_0_Reed_Switch = 0;          // The time that we last passed through an interesting state
unsigned long bounce_delay_Reed_Switch = 5; // The delay to filter bouncing
unsigned int  bin_counter = 0;              // binary counter for reed switch
int           virtual_Reed_Switch = VIRT_REED_SWITCH_OFF; // virtual switch

// State Machine Counter variables
int           state_counter1 = 0;      // The actual ~state~ of the state machine
int           prev_state_counter1 = 0; // Remembers the previous state (useful to know when the state has changed)
int           iSecCounter1 = -1;
int           prev_iSecCounter1 = 0;
int           iMinCounter1 = -1;
int           prev_iMinCounter1 = 0;
unsigned long start_counter1 = 0;
unsigned long elapsed_counter1 = 0;

// SM Status LED Variables
int           state_Status_Led = 0;
int           state_prev_Status_Led = 0;
int           pin_Status_Led = LED_BUILTIN;
int           val_Status_Led = 0;
unsigned long t_Status_Led = 0;
unsigned long t_0_Status_Led = 0;
unsigned long on_delay_Status_Led = 50;
unsigned long off_delay_Status_Led = 50;
int           beep_count_Status_Led = 0;
int           beep_number_Status_Led = 2;

char TimeCounterStr[] = "00:00"; /** String to store time counter value, format: MM:SS */
char SecondsCounterStr[] = "00"; /** String to store time counter value, format: SS */

/* Functions */

void setup()
{
    // if DEBUG is turned on, intialize serial connection
    Serial.begin(115200);
#ifdef SERIAL_DEBUG_ENABLED
    Serial.println(F("Debugging is ON"));
#endif
    Gpio_Init();
    ina219_init();
#ifdef ILI9340_ENABLED
    ILI9340_Init();
#endif // #ifdef ILI9340_ENABLED
#ifdef SSD1306_ENABLED
    Ssd1306_Oled_Init();
#endif
    // SM inits
    Display_Clear();
    StateMachine_counter1();
    StateMachine_Reed_Switch();
    state_machine_volt_meter();
    state_Display = DISPLAY_STATE_RESET;
    StateMachine_Display();
}

void loop()
{
    // Instruct all the state machines to proceed one step
    StateMachine_Reed_Switch();

    // Provide events that can force the state machines to change state
    switch (virtual_Reed_Switch)
    {
    case VIRT_REED_SWITCH_OFF:
        Status_Led_Off();
        if (state_counter1 == COUNTER_STATE_COUNTING)
        {
            state_counter1 = COUNTER_STATE_STOP;
        }
        break;
    case VIRT_REED_SWITCH_ON:
        Status_Led_On();
        if (state_counter1 == COUNTER_STATE_DISABLED)
        {
            state_counter1 = COUNTER_STATE_START;
        }
        break;
    }
    StateMachine_counter1();
    state_machine_volt_meter();
    StateMachine_Display();
}

/**
 * @brief Counter 1 state machine - counts the seconds
 */
void StateMachine_counter1(void)
{

    prev_state_counter1 = state_counter1;

    // State Machine Section
    switch (state_counter1)
    {
    case COUNTER_STATE_RESET:
        iSecCounter1 = 0;
        iMinCounter1 = 0;
        elapsed_counter1 = 0;
        state_counter1 = COUNTER_STATE_DISABLED;
        break;
    case COUNTER_STATE_DISABLED:
        // waiting for START event
        break;
    case COUNTER_STATE_START:
        iSecCounter1 = 0;
        iMinCounter1 = 0;
        elapsed_counter1 = 0;
        start_counter1 = millis();
        state_counter1 = COUNTER_STATE_COUNTING;
        state_Display = DISPLAY_STATE_TIMER_RUNNING;
        break;
    case COUNTER_STATE_COUNTING:
        prev_iSecCounter1 = iSecCounter1;
        elapsed_counter1 = millis() - start_counter1;
        iSecCounter1 = int((elapsed_counter1 / 1000) % 60);
        iMinCounter1 = int((elapsed_counter1 / 1000) / 60);
        if (iSecCounter1 != prev_iSecCounter1)
        {
// debug display
#ifdef SERIAL_DEBUG_ENABLED
            Serial.print(F("iMinCounter1: "));
            Serial.print(iMinCounter1, DEC);
            Serial.print(F(" iSecCounter1: "));
            Serial.println(iSecCounter1, DEC);
#endif
            state_Display = DISPLAY_STATE_TIMER_RUNNING;
        }
        break;
    case COUNTER_STATE_STOP:
        state_counter1 = COUNTER_STATE_DISABLED;
        state_Display = DISPLAY_STATE_TIMER_STOPPED;
        break;
    }
    if (prev_state_counter1 == state_counter1)
    {
        // do nothing
    }
    else
    {
// debug display
#ifdef SERIAL_DEBUG_ENABLED
        Serial.print(F("state_counter1: "));
        Serial.println(state_counter1, DEC);
#endif
    }
}

/**
 * @brief Reed switch state machine, extended with a logic switch which handles the 50Hz switching of reed switch on
 * the solenoid
 */
void StateMachine_Reed_Switch(void)
{
    int prev_virtual_Reed_Switch;
    // State Machine Section
    switch (state_Reed_Switch)
    {
    case REED_SWITCH_STATE_RESET: // RESET!
        // variables initialization
        bin_counter = 0;
        virtual_Reed_Switch = VIRT_REED_SWITCH_OFF;
        value_Reed_Switch = SW_OFF_LEVEL;
        state_Reed_Switch = REED_SWITCH_STATE_START_TIMER;
        break;

    case REED_SWITCH_STATE_START_TIMER: // Start SW timer
        // Start debounce timer and proceed to state3, "OFF, armed to ON"
        t_0_Reed_Switch = millis();
        state_Reed_Switch = REED_SWITCH_STATE_STOP_TIMER;
        break;

    case REED_SWITCH_STATE_STOP_TIMER: // Timer stop
        // Check to see if debounce delay has passed
        t_Reed_Switch = millis();
        if (t_Reed_Switch - t_0_Reed_Switch > bounce_delay_Reed_Switch)
        {
            state_Reed_Switch = REED_SWITCH_STATE_READ_PIN;
        }
        break;

    case REED_SWITCH_STATE_READ_PIN: // Read Switch pin
        value_Reed_Switch = digitalRead(pin_Reed_Switch);
        state_Reed_Switch = REED_SWITCH_STATE_ROTATE_BIN_COUNTER;
        break;
    case REED_SWITCH_STATE_ROTATE_BIN_COUNTER: // Rotate binary counter
        bin_counter = bin_counter << 1;
        if (value_Reed_Switch == SW_ON_LEVEL)
        {
            bin_counter++;
        }
        state_Reed_Switch = REED_SWITCH_STATE_SET_LOGIC_SWITCH;
        break;

    case REED_SWITCH_STATE_SET_LOGIC_SWITCH:
        prev_virtual_Reed_Switch = virtual_Reed_Switch;
        if (bin_counter > 0)
        {
            if (prev_virtual_Reed_Switch == VIRT_REED_SWITCH_OFF)
            {
                virtual_Reed_Switch = VIRT_REED_SWITCH_ON;
// debug display
#ifdef SERIAL_DEBUG_ENABLED
                Serial.println(F("Virtual Reed switch ON"));
#endif
            }
        }
        else
        {
            if (prev_virtual_Reed_Switch == VIRT_REED_SWITCH_ON)
            {
                virtual_Reed_Switch = VIRT_REED_SWITCH_OFF;
// debug display
#ifdef SERIAL_DEBUG_ENABLED
                Serial.println(F("Virtual Reed switch OFF"));
#endif
            }
        }
        state_Reed_Switch = REED_SWITCH_STATE_START_TIMER;

        break;
    }
}

/**
 * @brief LED-1 state machine
 */
void StateMachine_Status_Led(void)
{
    // Common code for every state
    state_prev_Status_Led = state_Status_Led;
    // State Machine Section
    switch (state_Status_Led)
    {
    case 0: // RESET
        // Set Beep Count to 0 then proceed to WAIT
        beep_count_Status_Led = 0;
        state_Status_Led = 1;
        break;
    case 1: // WAIT
        // Do nothing.  Only the top level loop can force us out of this state into state 2 "TURN ON"
        break;

    case 2: // TURNING ON
        Status_Led_On();
// debug display
#ifdef SERIAL_DEBUG_ENABLED
        Serial.println(F(":: LED ON"));
#endif
        t_0_Status_Led = millis();
        state_Status_Led = 3;
        break;
    case 3: // ON
        // Wait for time to elapse, then proceed to TURN OFF
        t_Status_Led = millis();
        if (t_Status_Led - t_0_Status_Led > on_delay_Status_Led)
        {
            state_Status_Led = 4;
        }
        break;
    case 4: // TURNING OFF
        // Increment the beep counter, proceed to OFF
        beep_count_Status_Led++;
        t_0_Status_Led = millis();
        Status_Led_Off();
// debug display
#ifdef SERIAL_DEBUG_ENABLED
        Serial.println(F(":: LED off"));
#endif
        state_Status_Led = 5;
        break;
    case 5: // OFF
        t_Status_Led = millis();
        if (t_Status_Led - t_0_Status_Led > off_delay_Status_Led)
        {
            state_Status_Led = 2;
        }
        if (beep_count_Status_Led >= beep_number_Status_Led)
        {
            state_Status_Led = 0;
        }
        break;
    }
}

/**
 * @brief Converts the minutes and seconds to char and updates the TimeCounterStr string
 * @param tMinutes : minutes value
 * @param tSeconds : seconds value
 */
void update_TimeCounterStr(int tMinutes, int tSeconds)
{
    TimeCounterStr[0] = (char)((tMinutes / 10) + 0x30);
    TimeCounterStr[1] = (char)((tMinutes % 10) + 0x30);
    TimeCounterStr[3] = (char)((tSeconds / 10) + 0x30);
    TimeCounterStr[4] = (char)((tSeconds % 10) + 0x30);
    SecondsCounterStr[0] = (char)((tSeconds / 10) + 0x30);
    SecondsCounterStr[1] = (char)((tSeconds % 10) + 0x30);
}

void Gpio_Init(void)
{
    Wire.setSDA(PB11); // SDA2 pin
    Wire.setSCL(PB10); // SCL2 pin
    Wire.begin();
    pinMode(pin_Manual_Switch, INPUT_PULLUP); // INPUT => reverse logic!
    pinMode(pin_Reed_Switch, INPUT_PULLUP);   // INPUT => reverse logic!
    pinMode(pin_Status_Led, OUTPUT);
    Status_Led_Off();
}


void Status_Led_Off(void)
{
    digitalWrite(pin_Status_Led, LED_OFF);
}

void Status_Led_On(void)
{
    digitalWrite(pin_Status_Led, LED_ON);
}