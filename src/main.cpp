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
* - STM32 pin |PA7|PA6|PA5|PA4|PA3|PA2|PA1|PA0| |PB0|PB6|PB7|PB8|PB9| Not in use: |PA15 |PB5  |PB4  |PB3   | **ALT-SPI1**
*
* BSD license, all text here must be included in any redistribution.
*
*/

//uncomment the line below if you would like to have debug messages
//#define SERIAL_DEBUG_ENABLED 1

//uncomment the line below if you would like to use SSD1306 OLED display
#define SSD1306_ENABLED 1

//uncomment the line below if you would like to use ILI9340 TFT display
#define ILI9340_ENABLED 1


// Adafruit BusIO library also needs to be installed on PlatformIO!

#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_GFX.h>

#ifdef  SSD1306_ENABLED
  #include <SSD1306Ascii.h>
  #include <SSD1306AsciiWire.h>
  #include <Adafruit_SSD1306.h>
  //#include "SSD1306AsciiAvrI2c.h" /* Use only when no other I2C devices are used! */
  #define OLED_I2C_ADDR 0x3C /* OLED module I2C address */
  #define OLED_WIDTH 128
  #define OLED_HEIGHT 32
#endif // #ifdef SSD1306_ENABLED

#ifdef ILI9340_ENABLED
  //#define USE_MY_BLUEPILL
  #include <MCUFRIEND_kbv.h> // for ILI9340
  #include <mcufriend_special.h>
  #include <Fonts/FreeSans12pt7b.h>
  #include <Fonts/FreeSans18pt7b.h>
  #include <Fonts/FreeSans24pt7b.h>
  // Assign human-readable names to some common 16-bit color values:
  #define	ILI9340_BLACK      0x0000
  #define	ILI9340_NAVY       0x000F
  #define	ILI9340_BLUE       0x001F
  #define	ILI9340_RED        0xF800
  #define	ILI9340_GREEN      0x07E0
  #define ILI9340_CYAN       0x07FF
  #define ILI9340_MAGENTA    0xF81F
  #define ILI9340_ORANGE     0xFDA0
  #define ILI9340_YELLOW     0xFFE0
  #define ILI9340_LIGHT_GRAY 0xBDF7
  #define ILI9340_DARK_GRAY  0x7BEF
  #define ILI9340_WHITE      0xFFFF
  #define ILI9340_DISPLAY_CLEAR_COLOR ILI9340_BLACK
  //Screen coordinates
  #define ILI9340_TIMER_POS_X 45
  #define ILI9340_TIMER_POS_Y 80
  #define ILI9340_TIMER_WIDTH 240
  #define ILI9340_TIMER_HEIGTH 80
  #define ILI9340_TIMER_Y_OFFSET 5
  #define ILI9340_TEMPERATURE_POS_X 25
  #define ILI9340_TEMPERATURE_POS_Y 170
  #define ILI9340_TEMPERATURE_WIDTH 310
  #define ILI9340_TEMPERATURE_HEIGTH 60
  #define ILI9340_TEMPERATURE_Y_OFFSET 3
  #define ILI9340_MILLI_VOLT_POS_X 45
  #define ILI9340_MILLI_VOLT_POS_Y 215
  #define ILI9340_MILLI_VOLT_WIDTH 250
  #define ILI9340_MILLI_VOLT_HEIGTH 20
  #define ILI9340_MILLI_VOLT_Y_OFFSET 2
#endif  // #ifdef ILI9340_ENABLED

#ifdef SSD1306_ENABLED
  //Set up OLED display
  SSD1306AsciiWire oled_ssd1306_display;
  //Adafruit_SSD1306 oled_ssd1306_display(OLED_WIDTH, OLED_HEIGHT, &Wire);
  //SSD1306AsciiAvrI2c oled_ssd1306_display; /* Use only when no other I2C devices are used! */
#endif // #ifdef SSD1306_ENABLED

#ifdef ILI9340_ENABLED
  //	MCUFRIEND_kbv(int CS=A3, int RS=A2, int WR=A1, int RD=A0, int RST=A4); //shield wiring
  MCUFRIEND_kbv tft_ili9340;
#endif // #ifdef ILI9340_ENABLED

#define SW_ON_LEVEL LOW  /* Switch on level */
#define SW_OFF_LEVEL HIGH  /* Switch off level */

#define LED_ON LOW    /* LED Switch on level */
#define LED_OFF HIGH  /* LED Switch off level */

#define COUNTER_STATE_RESET 0
#define COUNTER_STATE_DISABLED 1
#define COUNTER_STATE_START 2
#define COUNTER_STATE_COUNTING 3
#define COUNTER_STATE_STOP 4

#define DISPLAY_CLEAR_TRUE true
#define DISPLAY_CLEAR_FALSE false
#define DISPLAY_STOPPED_TRUE true
#define DISPLAY_STOPPED_FALSE false

#define VIRT_REED_SWITCH_OFF 0
#define VIRT_REED_SWITCH_ON 1

#define REED_SWITCH_STATE_RESET 0
#define REED_SWITCH_STATE_START_TIMER 1
#define REED_SWITCH_STATE_STOP_TIMER 2
#define REED_SWITCH_STATE_READ_PIN 3
#define REED_SWITCH_STATE_ROTATE_BIN_COUNTER 4
#define REED_SWITCH_STATE_SET_LOGIC_SWITCH 5

#define VOLT_METER_STATE_RESET 0
#define VOLT_METER_STATE_START_TIMER 1
#define VOLT_METER_STATE_STOP_TIMER 2
#define VOLT_METER_STATE_READ_VOLTAGE 3
#define VOLT_METER_STATE_CONVERT_TO_TEMPERATURE 4

#define DISPLAY_STATE_RESET 0
#define DISPLAY_STATE_TIMER_RUNNING 1
#define DISPLAY_STATE_TIMER_STOPPED 2
#define DISPLAY_STATE_HOLD_TIMER_ON 3
#define DISPLAY_STATE_TEMPERATURE 4
#define DISPLAY_STATE_DO_NOTHING 5

// Thermistor calculation values
// Original idea and code from Jimmy Roasts, https://github.com/JimmyRoasts/LaMarzoccoTempSensor

// resistance at 25 degrees C
#define THERMISTORNOMINAL_V1 50000  // version 1
#define THERMISTORNOMINAL_V2 49120  // version 2 updated calculation
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT_V1 4400  // version 1
#define BCOEFFICIENT_V2 3977  // version 2, updated calculation
// the value of the 'other' resistor
#define SERIESRESISTOR_V1 6960
#define SERIESRESISTOR_V2 6190  // version 2, measured on board
//reference voltage
//#define VOLTAGEREF 4.585
#define VOLTAGEREF 4.16

#define LMREF  5.07 //measured from LMBoard --- GND Board

//Top Level Variables:
int DEBUG = 1;  //Set to 1 to enable serial monitor debugging info

//Switch Variables - "sw1"
int state_Manual_Switch = 0;                   //The actual ~state~ of the state machine
int state_prev_Manual_Switch = 0;              //Remembers the previous state (useful to know when the state has changed)
int pin_Manual_Switch = PB4;                    //Input/Output (IO) pin for the switch, 10 = pin 10 a.k.a. D10
int value_Manual_Switch = 0;                     //Value of the switch ("HIGH" or "LOW")
unsigned long t_Manual_Switch = 0;             //Typically represents the current time of the switch
unsigned long t_0_Manual_Switch = 0;           //The time that we last passed through an interesting state
unsigned long bounce_delay_Manual_Switch = 20; //The delay to list for bouncing

//Switch Variables - "Reed_Switch"
int state_Reed_Switch = 0;                   //The actual ~state~ of the state machine
//int pin_Reed_Switch = 3;                   //Input/Output (IO) pin for the switch <- old config!
int pin_Reed_Switch = PB3;                    //Input/Output (IO) pin for the switch, 10 = pin 10 a.k.a. D10
int value_Reed_Switch = 0;                   //Value of the switch ("HIGH" or "LOW")
unsigned long t_Reed_Switch = 0;             //Typically represents the current time of the switch
unsigned long t_0_Reed_Switch = 0;           //The time that we last passed through an interesting state
unsigned long bounce_delay_Reed_Switch = 5; //The delay to filter bouncing
unsigned int bin_counter = 0; //binary counter for reed switch
int virtual_Reed_Switch = VIRT_REED_SWITCH_OFF; // virtual switch

//SM "Volt Meter" variables
int state_Volt_Meter = 0;
unsigned long t_Volt_Meter = 0;
unsigned long t_0_Volt_Meter = 0;
unsigned long delay_Between_2_Measures = 1000;

//SM Display variables
int state_Display = 0;
unsigned long t_Display = 0;
unsigned long t_0_Display = 0;
unsigned long delay_For_Stopped_Timer = 5000;  //millisec


//SM Counter variables
int state_counter1 = 0;                   //The actual ~state~ of the state machine
int prev_state_counter1 = 0;              //Remembers the previous state (useful to know when the state has changed)
int iSecCounter1 = -1;
int prev_iSecCounter1 = 0;
int iMinCounter1 = -1;
int prev_iMinCounter1 = 0;
unsigned long start_counter1 = 0;
unsigned long elapsed_counter1 = 0;

//SM Status LED Variables
int state_Status_Led = 0;
int state_prev_Status_Led = 0;
int pin_Status_Led = LED_BUILTIN;
int val_Status_Led = 0;
unsigned long t_Status_Led = 0;
unsigned long t_0_Status_Led = 0;
unsigned long on_delay_Status_Led = 50;
unsigned long off_delay_Status_Led = 50;
int beep_count_Status_Led = 0;
int beep_number_Status_Led = 2;

char TimeCounterStr[] = "00:00"; /** String to store time counter value, format: MM:SS */

// Current and voltage sensor class
Adafruit_INA219 ina219_monitor;

// INA219 sensor variables
float bus_Voltage_V;    /** Measured bus voltage in V*/
float bus_Voltage_mV;    /** Measured bus voltage in mV*/
char volt_String[] = "99999.9";         /** String to store measured voltage value in mV */

// Calculated temperature
float calc_Temperature_V1 = 0.0;
char temperature_String_V1[] = "  999.9";
float steinhart = 0.0;
float calc_Temperature_V2 = 0.0;
char temperature_String_V2[] = "999.9";
float thermistor_Res = 0.00; // Thermistor calculated resistance

/* Function declarations */

void StateMachine_counter1(void);
void StateMachine_Manual_Switch(void);
void StateMachine_Reed_Switch(void);
void StateMachine_Status_Led(void);
void StateMachine_Volt_Meter(void);
void StateMachine_Display(void);

void update_TimeCounterStr(int tMinutes, int tSeconds);

void Gpio_Init(void);
void Status_Led_Off(void);
void Status_Led_On(void);
void Ssd1306_Oled_Init(void);
void ILI9340_Init(void);
void display_Timer_On_All(bool need_Display_Clear,bool need_Display_Stopped);
void display_Timer_On_Ssd1306(bool need_Display_Clear,bool need_Display_Stopped);
void display_Timer_On_ILI9340(bool need_Display_Clear,bool need_Display_Stopped);
void display_Temperature_On_Ssd1306(void);
void display_Temperature_On_ILI9340(void);
void display_Milli_Volt_On_ILI9340(void);

void ina219_Init(void);
void get_Voltage(void);
void calculate_Temperature_V2(void);

void Adafruit_Text_Display_Test(void);

void Display_Running_Timer(void);
void Display_Stopped_Timer(void);
void Display_Temperature(void);
void Display_Clear(void);
void Display_Clear_ILI9340(uint16_t color);

/* Functions */

void setup() {
  //if DEBUG is turned on, intialize serial connection
  Serial.begin(115200);
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.println(F("Debugging is ON"));
  #endif
  Gpio_Init();
  ina219_Init();
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
  StateMachine_Volt_Meter();
  state_Display = DISPLAY_STATE_RESET;
  StateMachine_Display();
  //Adafruit_Text_Display_Test();

}

void loop() {
    //Instruct all the state machines to proceed one step
  StateMachine_Reed_Switch();

  //Provide events that can force the state machines to change state
  switch (virtual_Reed_Switch) {
    case VIRT_REED_SWITCH_OFF:
      Status_Led_Off();
      if (state_counter1 == COUNTER_STATE_COUNTING) {
        state_counter1 = COUNTER_STATE_STOP;
      }
      break;
    case VIRT_REED_SWITCH_ON:
      Status_Led_On();
      if (state_counter1 == COUNTER_STATE_DISABLED) {
        state_counter1 = COUNTER_STATE_START;
      }
      break;

  }
  StateMachine_counter1();
  StateMachine_Volt_Meter();
  StateMachine_Display();
}

/**
* @brief Counter 1 state machine - counts the seconds
*/
void StateMachine_counter1(void) {

  prev_state_counter1 = state_counter1;

  //State Machine Section
  switch (state_counter1) {
    case COUNTER_STATE_RESET:
      iSecCounter1 = 0;
      iMinCounter1 = 0;
      elapsed_counter1 = 0;
      state_counter1 = COUNTER_STATE_DISABLED;
      break;
    case COUNTER_STATE_DISABLED:
      //waiting for START event
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
      iSecCounter1 = int ((elapsed_counter1 / 1000) % 60);
      iMinCounter1 = int ((elapsed_counter1 / 1000) / 60);
      if (iSecCounter1 != prev_iSecCounter1) {
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
    //do nothing
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
* @brief Reed switch state machine, extended with a logic switch which handles the 50Hz switching of reed switch on the solenoid
*/
void StateMachine_Reed_Switch(void) {
  int prev_virtual_Reed_Switch;
  //State Machine Section
  switch (state_Reed_Switch) {
    case REED_SWITCH_STATE_RESET: //RESET!
      // variables initialization
      bin_counter = 0;
      virtual_Reed_Switch = VIRT_REED_SWITCH_OFF;
      value_Reed_Switch = SW_OFF_LEVEL;
      state_Reed_Switch = REED_SWITCH_STATE_START_TIMER;
      break;

    case REED_SWITCH_STATE_START_TIMER: //Start SW timer
      //Start debounce timer and proceed to state3, "OFF, armed to ON"
      t_0_Reed_Switch = millis();
      state_Reed_Switch = REED_SWITCH_STATE_STOP_TIMER;
      break;

    case REED_SWITCH_STATE_STOP_TIMER: //Timer stop
      //Check to see if debounce delay has passed
      t_Reed_Switch = millis();
      if (t_Reed_Switch - t_0_Reed_Switch > bounce_delay_Reed_Switch) {
        state_Reed_Switch = REED_SWITCH_STATE_READ_PIN;
      }
      break;

    case REED_SWITCH_STATE_READ_PIN: //Read Switch pin
      value_Reed_Switch = digitalRead(pin_Reed_Switch);
      state_Reed_Switch = REED_SWITCH_STATE_ROTATE_BIN_COUNTER;
      break;
    case REED_SWITCH_STATE_ROTATE_BIN_COUNTER: //Rotate binary counter
      bin_counter = bin_counter << 1;
      if (value_Reed_Switch == SW_ON_LEVEL) {
        bin_counter++ ;
      }
      state_Reed_Switch = REED_SWITCH_STATE_SET_LOGIC_SWITCH;
      break;

    case REED_SWITCH_STATE_SET_LOGIC_SWITCH:
      prev_virtual_Reed_Switch=virtual_Reed_Switch;
      if (bin_counter > 0) {
        if (prev_virtual_Reed_Switch == VIRT_REED_SWITCH_OFF) {
          virtual_Reed_Switch = VIRT_REED_SWITCH_ON;
          // debug display
          #ifdef SERIAL_DEBUG_ENABLED
          Serial.println(F("Virtual Reed switch ON"));
          #endif
        }

      }
      else
      {
        if (prev_virtual_Reed_Switch == VIRT_REED_SWITCH_ON) {
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
void StateMachine_Status_Led(void) {
  //Common code for every state
  state_prev_Status_Led = state_Status_Led;
  //State Machine Section
  switch (state_Status_Led) {
    case 0: //RESET
      //Set Beep Count to 0 then proceed to WAIT
      beep_count_Status_Led = 0;
      state_Status_Led = 1;
      break;
    case 1: //WAIT
      //Do nothing.  Only the top level loop can force us out of this state into state 2 "TURN ON"
      break;

    case 2: //TURNING ON
      Status_Led_On();
      // debug display
      #ifdef SERIAL_DEBUG_ENABLED
      Serial.println(F(":: LED ON"));
      #endif
      t_0_Status_Led = millis();
      state_Status_Led = 3;
      break;
    case 3: //ON
      //Wait for time to elapse, then proceed to TURN OFF
      t_Status_Led = millis();
      if (t_Status_Led - t_0_Status_Led > on_delay_Status_Led) {
        state_Status_Led = 4;
      }
      break;
    case 4: //TURNING OFF
      //Increment the beep counter, proceed to OFF
      beep_count_Status_Led++;
      t_0_Status_Led = millis();
      Status_Led_Off();
      // debug display
      #ifdef SERIAL_DEBUG_ENABLED
      Serial.println(F(":: LED off"));
      #endif
      state_Status_Led = 5;
      break;
    case 5: //OFF
      t_Status_Led = millis();
      if (t_Status_Led - t_0_Status_Led > off_delay_Status_Led) {
        state_Status_Led = 2;
      }
      if (beep_count_Status_Led >= beep_number_Status_Led) {
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
void update_TimeCounterStr(int tMinutes, int tSeconds) {
  TimeCounterStr[0] = (char) ((tMinutes / 10)+0x30);
  TimeCounterStr[1] = (char) ((tMinutes % 10)+0x30);
  TimeCounterStr[3] = (char) ((tSeconds / 10)+0x30);
  TimeCounterStr[4] = (char) ((tSeconds % 10)+0x30);
}

void Gpio_Init(void) {
  Wire.setSDA(PB11); // SDA2 pin
  Wire.setSCL(PB10); // SCL2 pin
  Wire.begin();
  pinMode(pin_Manual_Switch, INPUT_PULLUP); //INPUT => reverse logic!
  pinMode(pin_Reed_Switch, INPUT_PULLUP); //INPUT => reverse logic!
  pinMode(pin_Status_Led, OUTPUT);
  Status_Led_Off();  
}

void display_Timer_On_All(bool need_Display_Clear,bool need_Display_Stopped) {
  update_TimeCounterStr(iMinCounter1,iSecCounter1);
  #ifdef SSD1306_ENABLED
  display_Timer_On_Ssd1306(need_Display_Clear,need_Display_Stopped);
  #endif
  #ifdef ILI9340_ENABLED
  display_Timer_On_ILI9340(need_Display_Clear,need_Display_Stopped);
  #endif
}


void ina219_Init(void)
{
  Wire.setSDA(PB11); // SDA2 pin
  Wire.setSCL(PB10); // SCL2 pin  
  delay(100);
  ina219_monitor.begin();
}

void get_Voltage(void)
{
  //measure voltage and current
  bus_Voltage_V = (ina219_monitor.getBusVoltage_V());
  bus_Voltage_mV = bus_Voltage_V*1000;
  //convert to text
  dtostrf(bus_Voltage_mV, 7, 1, volt_String);
  // debug display
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print(volt_String);
  Serial.println(F(" mV"));
  #endif
}

void calculate_Temperature_V2(void) {
  thermistor_Res = SERIESRESISTOR_V2 * (1/((LMREF / bus_Voltage_V) -1));
  steinhart = thermistor_Res / THERMISTORNOMINAL_V2;
  steinhart = log(steinhart);                          // ln(R/Ro)
  steinhart /= BCOEFFICIENT_V2;                        // 1/B * ln(R/Ro)
  steinhart += (1.0 / (TEMPERATURENOMINAL + 273.15));  // + (1/To)
  steinhart = (float) (1.0 / steinhart);               // Invert
  calc_Temperature_V2 = (float) (steinhart - 273.15);  // convert to C
  if (bus_Voltage_V >= 1.0 and bus_Voltage_V <= 4.0) 
  {
    dtostrf(calc_Temperature_V2, 5, 1, temperature_String_V2);  
  }
  else 
  {
    strcpy(temperature_String_V2, "Error");
  }
  
  // debug display
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print(temperature_String_V2);
  Serial.println(F(" *C"));
  #endif
}

void StateMachine_Volt_Meter(void) {

  switch (state_Volt_Meter) {
    case VOLT_METER_STATE_RESET:
      state_Volt_Meter = VOLT_METER_STATE_START_TIMER;
      break;

    case VOLT_METER_STATE_START_TIMER:
      t_0_Volt_Meter = millis();
      state_Volt_Meter = VOLT_METER_STATE_STOP_TIMER;
      break;

    case VOLT_METER_STATE_STOP_TIMER:
      t_Volt_Meter = millis();
      if (t_Volt_Meter - t_0_Volt_Meter >= delay_Between_2_Measures) {
        state_Volt_Meter = VOLT_METER_STATE_READ_VOLTAGE;
      }
      break;

    case VOLT_METER_STATE_READ_VOLTAGE:
      get_Voltage();
      state_Volt_Meter = VOLT_METER_STATE_CONVERT_TO_TEMPERATURE;
      break;

    case VOLT_METER_STATE_CONVERT_TO_TEMPERATURE:
      calculate_Temperature_V2();
      state_Volt_Meter = VOLT_METER_STATE_START_TIMER;
      state_Display = DISPLAY_STATE_TEMPERATURE;      
      break;
  }
}

void StateMachine_Display(void) {

  switch (state_Display) {
    case DISPLAY_STATE_RESET:
      Display_Stopped_Timer();
      state_Display = DISPLAY_STATE_TEMPERATURE;
      break;

    case DISPLAY_STATE_TIMER_RUNNING:
      Display_Running_Timer();
      //Display_Temperature();
      state_Display = DISPLAY_STATE_TEMPERATURE;
      break;

    case DISPLAY_STATE_TIMER_STOPPED:
      Display_Stopped_Timer();
      t_0_Display = millis();
      state_Display = DISPLAY_STATE_TEMPERATURE;      
      break;

    case DISPLAY_STATE_HOLD_TIMER_ON:
      t_Display = millis();
      if (t_Display - t_0_Display > delay_For_Stopped_Timer) {
        state_Display = DISPLAY_STATE_TEMPERATURE;
      }
      break;

    case DISPLAY_STATE_TEMPERATURE:
      Display_Temperature();
      state_Display = DISPLAY_STATE_DO_NOTHING;
      break;

    case DISPLAY_STATE_DO_NOTHING:
      break;
  }
}

void Display_Running_Timer(void) {
  display_Timer_On_All(DISPLAY_CLEAR_FALSE,DISPLAY_STOPPED_FALSE);
}

void Display_Stopped_Timer(void) {
  display_Timer_On_All(DISPLAY_CLEAR_FALSE,DISPLAY_STOPPED_TRUE);
}

void Display_Temperature(void) {
  #ifdef SSD1306_ENABLED
  display_Temperature_On_Ssd1306();
  #endif
  #ifdef ILI9340_ENABLED
  display_Temperature_On_ILI9340();
  display_Milli_Volt_On_ILI9340();
  #endif
}

void Display_Clear(void) {
  #ifdef SSD1306_ENABLED
  oled_ssd1306_display.clear();
  #endif
  #ifdef ILI9340_ENABLED
  Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
  #endif
}


#ifdef SSD1306_ENABLED
void Ssd1306_Oled_Init(void) {
  Wire.setSDA(PB11); // SDA2 pin
  Wire.setSCL(PB10); // SCL2 pin  
  oled_ssd1306_display.begin(&Adafruit128x32, OLED_I2C_ADDR);
  oled_ssd1306_display.clear();
  oled_ssd1306_display.setFont(fixed_bold10x15);
  oled_ssd1306_display.setRow(0);
  oled_ssd1306_display.println("Linea Mini ");
  oled_ssd1306_display.println("Brew Timer ");
  delay(1000);
  oled_ssd1306_display.clear();
  oled_ssd1306_display.setRow(0);
  oled_ssd1306_display.println("Version 1.1");
  delay(500);  
}


/**
* @brief Updates and then displays the TimeCounterStr string on the OLED screen, format: MM:SS
* @param need_Display_Clear : is display clear needed?
* @param need_Display_Stopped : is visualisation of sopped timer needed?
*/
void display_Timer_On_Ssd1306(bool need_Display_Clear,bool need_Display_Stopped) {
  if(need_Display_Clear) {
    oled_ssd1306_display.clear();
  }
  oled_ssd1306_display.setCol(0);
  oled_ssd1306_display.setRow(0);
  oled_ssd1306_display.print(TimeCounterStr);
  if(need_Display_Stopped) {
    oled_ssd1306_display.print(" stop ");
  }
  else {
    oled_ssd1306_display.print("      ");
  }
}

void display_Temperature_On_Ssd1306() {
  oled_ssd1306_display.setCol(0);
  oled_ssd1306_display.setRow(2);
  oled_ssd1306_display.print(temperature_String_V2);
  oled_ssd1306_display.print(F(" *C"));
}
#endif

#ifdef ILI9340_ENABLED
void ILI9340_Init(void) {
  tft_ili9340.reset();
  uint16_t identifier;
  uint16_t ID = tft_ili9340.readID(); //
  identifier = ID;
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  #endif
  if (ID == 0xE300) ID = 0x9340; // "Weird" Banggood shield, forcing ILI9340 setup
  tft_ili9340.begin(ID);
  tft_ili9340.setRotation(1);
  // Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
  // tft_ili9340.setCursor(0, 0);
  // tft_ili9340.print("ID = 0x");
  // tft_ili9340.println(identifier, HEX);
  // delay(5000);
  Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
  tft_ili9340.setCursor(0, 40);
  //uint16_t wid = tft_ili9340.width();
  tft_ili9340.setTextColor(ILI9340_WHITE);  
  tft_ili9340.setFont(&FreeSans24pt7b);
  tft_ili9340.setTextSize(1);
  tft_ili9340.println("  Linea Mini ");
  tft_ili9340.println("  Brew Timer ");
  tft_ili9340.println("  Version 1.0");
  delay(1500);  
}

/**
* @brief Updates and then displays the TimeCounterStr string on the ILI9340 TFT screen, format: MM:SS
* @param need_Display_Clear : is display clear needed?
* @param need_Display_Stopped : is visualisation of sopped timer needed?
*/
void display_Timer_On_ILI9340(bool need_Display_Clear,bool need_Display_Stopped) {
  if(need_Display_Clear) {
    Display_Clear_ILI9340(ILI9340_DISPLAY_CLEAR_COLOR);
  }
  tft_ili9340.setFont(&FreeSans24pt7b);
  tft_ili9340.setTextSize(2);
  if(need_Display_Stopped) {
    tft_ili9340.setTextColor(ILI9340_DARK_GRAY);
  }
  else {
    tft_ili9340.setTextColor(ILI9340_WHITE);
  }  
  tft_ili9340.fillRect(ILI9340_TIMER_POS_X, ILI9340_TIMER_POS_Y-ILI9340_TIMER_HEIGTH, ILI9340_TIMER_WIDTH, ILI9340_TIMER_HEIGTH + ILI9340_TIMER_Y_OFFSET, ILI9340_DISPLAY_CLEAR_COLOR);
  tft_ili9340.setCursor(ILI9340_TIMER_POS_X, ILI9340_TIMER_POS_Y);
  tft_ili9340.println(TimeCounterStr);
}

void display_Temperature_On_ILI9340(void) {
  tft_ili9340.fillRect(ILI9340_TEMPERATURE_POS_X, ILI9340_TEMPERATURE_POS_Y-ILI9340_TEMPERATURE_HEIGTH, ILI9340_TEMPERATURE_WIDTH, ILI9340_TEMPERATURE_HEIGTH+ILI9340_TEMPERATURE_Y_OFFSET, ILI9340_DISPLAY_CLEAR_COLOR);
  tft_ili9340.setFont(&FreeSans18pt7b);
  tft_ili9340.setTextSize(2);
  tft_ili9340.setTextColor(ILI9340_ORANGE);
  tft_ili9340.setCursor(ILI9340_TEMPERATURE_POS_X, ILI9340_TEMPERATURE_POS_Y);
  tft_ili9340.print(temperature_String_V2);
  tft_ili9340.print(F(" *C"));  
}

void display_Milli_Volt_On_ILI9340(void) {
  tft_ili9340.setFont(&FreeSans12pt7b);
  tft_ili9340.setTextSize(1);
  tft_ili9340.fillRect(ILI9340_MILLI_VOLT_POS_X, ILI9340_MILLI_VOLT_POS_Y-ILI9340_MILLI_VOLT_HEIGTH, ILI9340_MILLI_VOLT_WIDTH, ILI9340_MILLI_VOLT_HEIGTH+ILI9340_MILLI_VOLT_Y_OFFSET, ILI9340_DISPLAY_CLEAR_COLOR);
  tft_ili9340.setTextColor(ILI9340_MAGENTA);
  tft_ili9340.setCursor(ILI9340_MILLI_VOLT_POS_X, ILI9340_MILLI_VOLT_POS_Y);
  tft_ili9340.print(F("Th Voltage: "));
  tft_ili9340.print(volt_String);
  tft_ili9340.print(F(" mV"));
}

void Display_Clear_ILI9340(uint16_t color) {
  //Serial.println("Display_Clear_ILI9340");
  tft_ili9340.fillScreen(color);
  tft_ili9340.setCursor(0, 0);
  delay(50);
}
#endif // #ifdef ILI9340_ENABLED

void Status_Led_Off(void) {
  digitalWrite(pin_Status_Led, LED_OFF);
}

void Status_Led_On(void) {
  digitalWrite(pin_Status_Led, LED_ON);
}