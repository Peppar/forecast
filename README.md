# Forecast for ESP32-WROOM-32 and HelTec e-ink1.54

Connects to a weather service (APIXU, free for up to 10000 requests per month),
and displays the forecast on the e-ink display.

![The ESP32-WROOM-32 module connected to the HelTec e-ink1.54 module on a breadboard](https://user-images.githubusercontent.com/51078/55734599-1f2ce500-5a20-11e9-8456-ad8c125d6106.jpg "Forecast test setup")

## Setup

1. Follow [the getting started guide for ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/get-started-devkitc.html).
2. Hook up the e-ink display to the GPIO pins of the ESP32.
3. Set the corresponding GPIO pin numbers in main/main.c
4. Run "make menuconfig" and fill in the required configuration variables.
   Fill in everything in the "Forecast app" submenu.
5. Run "make flash monitor" with the device connected to make and
   program the device and monitor for log messages. Don't forget to push
   the BOOT button on the board when it says "Connecting".

You can also use cmake to build out-of-tree. Initialize a cmake build
directory between steps 3 and 4 and continue from this
directory. Note: "make monitor" doesn't seem to work with cmake.

## Legal remarks

Based on the SPI master example in the ESP IDF, which is in the public domain,
and on the HelTec Arduino library (https://github.com/HelTecAutomation/e-ink),
which specifies no license that I can see.

All code, except e-ink.c and e-ink.h, is in the public domain.

Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.

   
