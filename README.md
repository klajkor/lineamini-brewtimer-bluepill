# Arduino Brew Timer for LaMarzocco Linea Mini espresso machine

### Board: STM32 BluePill (generic STM32F103C8), Arduino Framework

### Extension modules and peripherials used:
- SSD1306 OLED display, I2C - optional, please see source code for feature switch (SSD1306_ENABLED)
- ILI9340 TFT display, SPI and 8 bit data - optional, please see source code for feature switch (ILI9340_ENABLED)
- reed switch, sensing magnet valve sc
- INA219 voltage and current sensor, I2C

### Libraries used:
- SSD1306Ascii by Bill Greiman - Copyright (c) 2019, Bill Greiman
- Adafruit INA219 by Adafruit - Copyright (c) 2012, Adafruit Industries
- MCUFRIEND_kbv - Copyright (c) 2020, David Prentice

### BluePill pinout connections

I2C bus:
- SCK - pin PB6
- SDA - pin PB7

Toolchain: VSCode + Platform.IO

Copyright (c) 2021, Robert Klajko
