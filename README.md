# Arduino Brew Timer and Temperature Display for LaMarzocco Linea Mini espresso machine

## Board
- STM32 BluePill (generic STM32F103C8) with Arduino Framework

## Extension modules and peripherials used:
- SSD1306 OLED display, I2C - optional, please see source code for feature switch (SSD1306_ENABLED)
- ILI9340 TFT display, SPI and 8 bit data - optional, please see source code for feature switch (ILI9340_ENABLED)
- reed switch, sensing magnet valve sc
- INA219 voltage and current sensor, I2C

## Libraries used:
- SSD1306Ascii by Bill Greiman - Copyright (c) 2019, Bill Greiman
- Adafruit INA219 by Adafruit - Copyright (c) 2012, Adafruit Industries
- MCUFRIEND_kbv - Copyright (c) 2020, David Prentice

## BluePill pinout connections

### Wiring of reed switch:
 - PB3
 - GND

### Wiring of I2C devices (INA219, SSD1306):
 - SDA - PB11 (I2C2 used due to pins required for ILI9340 TFT)
 - SCL - PB10 (I2C2 used due to pins required for ILI9340 TFT)

### Wiring of ILI9340:
- LCD pins==>|D7 |D6 |D5 |D4 |D3 |D2 |D1 |D0 | |RD |WR |RS |CS |RST| Not in use: |SD_SS|SD_DI|SD_DO|SD_SCK|
- STM32 pins>|PA7|PA6|PA5|PA4|PA3|PA2|PA1|PA0| |PB0|PB6|PB7|PB8|PB9| Not in use: |PA15 |PB5  |PB4  |PB3   | (ALT-SPI1)

## Toolchain
- VSCode
- Platform.IO
- STM32duino (Arduino firmware for STM32)

Copyright (c) 2021, Robert Klajko

BSD license, all text here must be included in any redistribution.
