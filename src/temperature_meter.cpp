#include "temperature_meter.h"

//#define SERIAL_DEBUG_ENABLED

// Thermistor calculation values
// Original idea and code from Jimmy Roasts, https://github.com/JimmyRoasts/LaMarzoccoTempSensor

// resistance at 25 degrees C
#define THERMISTORNOMINAL_V1 (50000U) // version 1
#define THERMISTORNOMINAL_V2 (49120U) // version 2 updated calculation

// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL (25U)

// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT_V1 (4400U) // version 1
#define BCOEFFICIENT_V2 (3977U) // version 2, updated calculation
// the value of the 'other' resistor
#define SERIESRESISTOR_V1 (6960U)
#define SERIESRESISTOR_V2 (6190U) // version 2, measured on board

// reference Vcc voltage
#define LMREF ((float)(5.07)) // measured from LMBoard --- GND Board

// Current and voltage sensor class
static Adafruit_INA219 ina219_monitor;
static uint8_t         in219_init_success;

// State Machine "Volt Meter" variables
static int           state_volt_meter = VOLT_METER_STATE_RESET;
static unsigned long t_volt_meter = 0;
static unsigned long t_0_volt_meter = 0;
static unsigned long delay_between_2_measures = 495; // In milliseconds

// Global temperature strings
char TEMPERATURE_STR_V2[6] = "999.9";
char TEMPERATURE_STR_LED_V2[5] = "99.9";
char MILLI_VOLT_STR[6] = "99999";

void ina219_init(void)
{
    delay(10);
    ina219_monitor.begin();
    delay(100);
    if (ina219_monitor.success())
    {
        in219_init_success = 1;
    }
    else
    {
        in219_init_success = 0;
    }
}

float get_thermistor_voltage(void)
{
    float bus_voltage_V;

    if (in219_init_success)
    {
        // measure voltage
        bus_voltage_V = ina219_monitor.getBusVoltage_V();
    }
    else
    {
        // Only for testing, if ina219 init failed:
        bus_voltage_V = (float)((45 - ((millis() / 1000) % 30)) / 10.0);
    }
    return bus_voltage_V;
}

void state_machine_volt_meter(void)
{
    float measured_V;
    float head_temperature;

    switch (state_volt_meter)
    {
    case VOLT_METER_STATE_RESET:
        state_volt_meter = VOLT_METER_STATE_START_TIMER;
        head_temperature = 0.0;
        break;

    case VOLT_METER_STATE_START_TIMER:
        t_0_volt_meter = millis();
        state_volt_meter = VOLT_METER_STATE_STOP_TIMER;
        break;

    case VOLT_METER_STATE_STOP_TIMER:
        t_volt_meter = millis();
        if (t_volt_meter - t_0_volt_meter > delay_between_2_measures)
        {
            state_volt_meter = VOLT_METER_STATE_READ_VOLTAGE;
        }
        break;

    case VOLT_METER_STATE_READ_VOLTAGE:
        measured_V = get_thermistor_voltage();
        head_temperature = calculate_temperature_v2(measured_V);
        update_temperature_str(head_temperature, TEMPERATURE_STR_V2, TEMPERATURE_STR_LED_V2);
        update_millivolt_str(measured_V, MILLI_VOLT_STR);
        state_volt_meter = VOLT_METER_STATE_START_TIMER;
        break;
    }
}

float calculate_temperature_v2(float thermistor_voltage)
{
    float steinhart = 0.0;
    float calc_Temperature_V2 = 0.0;
    float thermistor_Res = 0.00; // Thermistor calculated resistance

    thermistor_Res = SERIESRESISTOR_V2 * (1 / ((LMREF / thermistor_voltage) - 1.0));
    steinhart = thermistor_Res / THERMISTORNOMINAL_V2;
    steinhart = log(steinhart);                         // ln(R/Ro)
    steinhart = steinhart / BCOEFFICIENT_V2;            // 1/B * ln(R/Ro)
    steinhart += (1.0 / (TEMPERATURENOMINAL + 273.15)); // + (1/To)
    steinhart = 1.0 / steinhart;                        // Invert
    calc_Temperature_V2 = (float)steinhart - 273.15;    // convert to C
    if (calc_Temperature_V2 <= -10)
    {
        calc_Temperature_V2 = -9.9;
    }
    if (calc_Temperature_V2 > 199.9)
    {
        calc_Temperature_V2 = 199.9;
    }
    return calc_Temperature_V2;
}

void update_temperature_str(float temperature_i, char *p_temperature_str_o, char *p_temperature_str_led_o)
{
    dtostrf(temperature_i, 5, 1, p_temperature_str_o);
    if (temperature_i >= 100.0)
    {
        dtostrf(temperature_i, 4, 0, p_temperature_str_led_o);
    }
    else
    {
        dtostrf(temperature_i, 4, 1, p_temperature_str_led_o);
    }
// debug display
#ifdef SERIAL_DEBUG_ENABLED
    Serial.print(TEMPERATURE_STR_V2);
    Serial.println(F(" *C"));
#endif
}

void update_millivolt_str(float thermistor_voltage_i, char *p_millivolt_str_o)
{
    int millivolt;
    millivolt = (int)(thermistor_voltage_i * 1000);
    // convert to text
    dtostrf(millivolt, 5, 0, p_millivolt_str_o);
// debug display
#ifdef SERIAL_DEBUG_ENABLED
    Serial.print(volt_String);
    Serial.println(F(" mV"));
#endif
}