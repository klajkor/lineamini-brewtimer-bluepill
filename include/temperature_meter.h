#pragma once

#include "Adafruit_INA219.h"
#include "main.h"
#include <Arduino.h>

#define VOLT_METER_STATE_RESET 0
#define VOLT_METER_STATE_START_TIMER 1
#define VOLT_METER_STATE_STOP_TIMER 2
#define VOLT_METER_STATE_READ_VOLTAGE 3

// Global temperature strings
extern char TEMPERATURE_STR_V2[6];
extern char TEMPERATURE_STR_LED_V2[5];
extern char MILLI_VOLT_STR[6];

void  ina219_init(void);
float get_thermistor_voltage(void);
void  state_machine_volt_meter(void);
float calculate_temperature_v2(float thermistor_voltage);
void  update_temperature_str(float temperature_i, char *p_temperature_str_o, char *p_temperature_str_led_o);
void  update_millivolt_str(float thermistor_voltage_i, char *p_millivolt_str_o);
