#ifndef __E_INK_H__
#define __E_INK_H__

/* E-ink control library based on the HelTec Arduino library
 * (https://github.com/HelTecAutomation/e-ink)
 *
 * Adjustments by Oskar Holstensson. 8 April 2019
 */

#include <stdint.h>

#include "driver/spi_master.h"

#define EPD_WIDTH  200
#define EPD_HEIGHT 200

extern const uint8_t lut_full_update[];
extern const uint8_t lut_partial_update[];

/**
 *  @brief: set the look-up table register
 */
void epd_set_lut(const uint8_t* lut);

/**
 *  @brief: After this command is transmitted, the chip would enter the
 *          deep-sleep mode to save power.
 *          The deep sleep mode would return to standby by hardware reset.
 *          You can use epd_init to awaken.
 */
void epd_sleep();

/**
 *  @brief: Put a partial image buffer to the frame memory.
 *          this won't update the display.
 */
void epd_set_partial_frame_memory(const uint8_t* image_buffer,
                                  int x, int y,
                                  int image_width, int image_height);
/**
 *  @brief: Put an image buffer to the frame memory.
 *          this won't update the display.
 */
void epd_set_frame_memory(const uint8_t* image_buffer);

/**
 *  @brief: Clear the frame memory with the specified color.
 *          This won't update the display.
 */
void epd_clear_frame_memory(uint8_t color);

/**
 *  @brief: Update the display.
 *          There are 2 memory areas embedded in the e-paper display
 *          but once this function is called, the the next action of
 *          SetFrameMemory or ClearFrame will set the other memory
 *          area.
 */
void epd_display_frame();

/**
 *  @brief: Add the display as a device to the SPI bus.
 */
esp_err_t epd_spi_bus_add(spi_host_device_t host,
                          spi_device_handle_t* handle,
                          int cs_pin);

/**
 *  @brief: Initialize the display.
 */
void epd_init(spi_device_handle_t spi, const uint8_t* lut, uint8_t dc_pin);

#endif
