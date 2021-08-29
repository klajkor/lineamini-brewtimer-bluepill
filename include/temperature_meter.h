#pragma once

#include "Adafruit_INA219.h"
#include "main.h"
#include <Arduino.h>

#define VOLT_METER_STATE_RESET 0
#define VOLT_METER_STATE_START_TIMER 1
#define VOLT_METER_STATE_STOP_TIMER 2
#define VOLT_METER_STATE_READ_VOLTAGE 3

// Thermistor calculation values
// Original idea and code from Jimmy Roasts, https://github.com/JimmyRoasts/LaMarzoccoTempSensor

// resistance at 25 degrees C
#define THERMISTORNOMINAL_V1 (50000U) // version 1
#define THERMISTORNOMINAL_V2 (49120U) // version 2 updated calculation
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL (25U)
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES (5U)
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT_V1 (4400U) // version 1
#define BCOEFFICIENT_V2 (3977U) // version 2, updated calculation
// the value of the 'other' resistor
#define SERIESRESISTOR_V1 (6960U)
#define SERIESRESISTOR_V2 (6190U) // version 2, measured on board
// scaling value to convert voltage
#define VOLTAGESCALE ((float)(12.1))
// reference voltage
//#define VOLTAGEREF 4.585
#define VOLTAGEREF ((float)(4.16))

#define LMREF ((float)(5.07)) // measured from LMBoard --- GND Board

// Global temperature strings
extern char temperature_str_V2[6];
extern char Led_temperature_str_V2[5];
extern char milli_volt_str[6];

void  ina219_Init(void);
float get_Voltage(void);
void  StateMachine_Volt_Meter(void);
void  calculate_Temperature_V2(float thermistor_voltage);