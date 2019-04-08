## Forecast for ESP32-WROOM-32 and HelTec e-ink1.54

Connects to a weather service (APIXU, free for up to 10000 requests per month),
and displays the forecast on the e-ink display.

* Follow [the getting started guide for ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/get-started-devkitc.html).
* Hook up the e-ink display to the GPIO pins of the ESP32.
* Set the corresponding GPIO pin numbers in main/main.c
* Run "make menuconfig" and fill in the required configuration variables.
  Fill in everything in the "Forecast app" submenu.
* Run "make flash monitor" with the device connected to make and
  program the device and monitor for log messages. Don't forget to push
  the BOOT button on the board when it says "Connecting".

# Legal remarks

Based on the SPI master example in the ESP IDF, which is in the public domain,
and on the HelTec Arduino library (https://github.com/HelTecAutomation/e-ink),
which specifies no license that I can see.

All code, except e-ink.c and e-ink.h, is in the public domain.

Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.

