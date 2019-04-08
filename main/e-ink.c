/* E-ink control library based on the HelTec Arduino library
 * (https://github.com/HelTecAutomation/e-ink)
 *
 * Adjustments by Oskar Holstensson. 8 April 2019
 */
#include "e-ink.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF

/* We need to remember the D/C pin and BUSY pin GPIO number.
 * Store them as a global variables.
 */
static int g_epd_dc_pin;
static int g_epd_busy_pin;

/* We keep a handle to the SPI device here.
 */
static spi_device_handle_t g_spi;

/* Lookup tables sent to the display.
 */
const uint8_t lut_full_update[] =
{
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const uint8_t lut_partial_update[] =
{
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* The E-ink module needs a bunch of command/argument values to be
 * initialized. They are stored in this struct.
 */
typedef struct {
  uint8_t cmd;
  uint8_t data[16];
  uint8_t databytes; // No of data in data; bit 7 = delay after set;
                     // 0xFF = end of cmds.
} epd_init_cmd_t;

/* Place data into DRAM. Constant data gets placed into DROM by
 * default, which is not accessible by DMA.
 */
DRAM_ATTR static const epd_init_cmd_t epd_init_cmds[] =
  {
   {DRIVER_OUTPUT_CONTROL, {(EPD_HEIGHT-1)&0xFF, ((EPD_HEIGHT-1)>>8)&0xFF, 0}, 3},
   {BOOSTER_SOFT_START_CONTROL, {0xD7, 0xD6, 0x9D}, 3},
   {WRITE_VCOM_REGISTER, {0xA8}, 1},
   {SET_DUMMY_LINE_PERIOD, {0x1A}, 1},
   {SET_GATE_TIME, {0x08}, 1},
   {DATA_ENTRY_MODE_SETTING, {0x03}, 1},
   {0, {0}, 0xff},
  };

/* Wait for the device to deassert BUSY.
 */
void epd_wait_busy()
{
  while(gpio_get_level(g_epd_busy_pin)) {/* No-op */}
}

/* Send a command to the display. Uses spi_device_polling_transmit,
 * which waits until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in
 * polling mode for higher speed. The overhead of interrupt
 * transactions is more than just waiting for the transaction to
 * complete.
 */
static void epd_send_command(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));         // Zero out the transaction
    t.length = 8;                     // Command is 8 bits
    t.tx_buffer = &cmd;               // The data is the cmd itself
    t.user = (void*)0;                // D/C needs to be set to 0
    epd_wait_busy();
    ret = spi_device_polling_transmit(g_spi, &t); // Transmit!
    assert(ret == ESP_OK);            // Should have had no issues.
}

/* Send data to the display. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since data transactions are usually small, they are handled in
 * polling mode for higher speed. The overhead of interrupt
 * transactions is more than just waiting for the transaction to
 * complete.
 */
static void epd_send_data(const uint8_t* data, int len) {
  esp_err_t ret;
  spi_transaction_t t;
  if (len == 0) return;             // No need to send anything
  memset(&t, 0, sizeof(t));         // Zero out the transaction
  t.length = len*8;                 // Len is in bytes, transaction
                                    // length is in bits.
  t.tx_buffer = data;               // Data
  t.user = (void*)1;                // D/C needs to be set to 1
  ret = spi_device_polling_transmit(g_spi, &t); // Transmit!
  assert(ret == ESP_OK);            // Should have had no issues.
}

/* Send one byte of data to the display.
 */
static void epd_send_byte(uint8_t data) {
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));         // Zero out the transaction
  t.length = 8;                     // One byte is eight bits.
  t.tx_buffer = &data;              // Data
  t.user = (void*)1;                // D/C needs to be set to 1
  ret = spi_device_polling_transmit(g_spi, &t); // Transmit!
  assert(ret == ESP_OK);            // Should have had no issues.
}

/**
 *  @brief: specify the memory area for data R/W
 */
static void epd_set_memory_area(int x_start, int y_start, int x_end, int y_end) {
  epd_send_command(SET_RAM_X_ADDRESS_START_END_POSITION);
  epd_send_byte((x_start >> 3) & 0xFF);
  epd_send_byte((x_end >> 3) & 0xFF);
  epd_send_command(SET_RAM_Y_ADDRESS_START_END_POSITION);
  epd_send_byte(y_start & 0xFF);
  epd_send_byte((y_start >> 8) & 0xFF);
  epd_send_byte(y_end & 0xFF);
  epd_send_byte((y_end >> 8) & 0xFF);
}

/**
 *  @brief: specify the start point for data R/W
 */
static void epd_set_memory_pointer(int x, int y) {
  epd_send_command(SET_RAM_X_ADDRESS_COUNTER);
  epd_send_byte((x >> 3) & 0xFF);
  epd_send_command(SET_RAM_Y_ADDRESS_COUNTER);
  epd_send_byte(y & 0xFF);
  epd_send_byte((y >> 8) & 0xFF);
}

/**
 *  @brief: set the look-up table register
 */
void epd_set_lut(const uint8_t* lut) {
  epd_send_command(WRITE_LUT_REGISTER);
  epd_send_data(&lut[0], 30);
}

/**
 *  @brief: After this command is transmitted, the chip would enter the
 *          deep-sleep mode to save power.
 *          The deep sleep mode would return to standby by hardware reset.
 *          You can use epd_init to awaken.
 */
void epd_sleep() {
  epd_send_command(DEEP_SLEEP_MODE);
}

/**
 *  @brief: Put a partial image buffer to the frame memory.
 *          this won't update the display.
 */
void epd_set_partial_frame_memory(const uint8_t* image_buffer,
                                  int x, int y,
                                  int image_width, int image_height) {
  int x_end;
  int y_end;

  if (image_buffer == NULL || x < 0 || image_width < 0
      || y < 0 || image_height < 0) {
    return;
  }

  x &= 0xF8;
  image_width &= 0xF8;
  if (x + image_width >= EPD_WIDTH) {
    x_end = EPD_WIDTH - 1;
  } else {
    x_end = x + image_width - 1;
  }
  if (y + image_height >= EPD_HEIGHT) {
    y_end = EPD_HEIGHT - 1;
  } else {
    y_end = y + image_height - 1;
  }
  epd_set_memory_area(x, y, x_end, y_end);
  epd_set_memory_pointer(x, y);
  epd_send_command(WRITE_RAM);

  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      epd_send_byte(image_buffer[i + j * (image_width / 8)]);
    }
  }
}

/**
 *  @brief: Put an image buffer to the frame memory.
 *          this won't update the display.
 */
void epd_set_frame_memory(const uint8_t* image_buffer) {
  epd_set_memory_area(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
  epd_set_memory_pointer(0, 0);
  epd_send_command(WRITE_RAM);
  epd_send_data(image_buffer, EPD_WIDTH / 8 * EPD_HEIGHT);
}

/**
 *  @brief: Clear the frame memory with the specified color.
 *          This won't update the display.
 */
void epd_clear_frame_memory(uint8_t color) {
  epd_set_memory_area(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
  epd_set_memory_pointer(0, 0);
  epd_send_command(WRITE_RAM);
  for (int i = 0; i < EPD_WIDTH / 8 * EPD_HEIGHT; i++) {
    epd_send_byte(color);
  }
}

/**
 *  @brief: Update the display.
 *          There are 2 memory areas embedded in the e-paper display
 *          but once this function is called, the the next action of
 *          SetFrameMemory or ClearFrame will set the other memory
 *          area.
 */
void epd_display_frame() {
  epd_send_command(DISPLAY_UPDATE_CONTROL_2);
  epd_send_byte(0xC4);
  epd_send_command(MASTER_ACTIVATION);
  epd_send_command(TERMINATE_FRAME_READ_WRITE);
}

/**
 *  @brief: This function is called (in irq context!) just before a
 *          transmission starts. It will set the D/C line to the value
 *          indicated in the user field.
 */
static void epd_spi_pre_transfer_callback(spi_transaction_t *t)
{
  int dc = (int)t->user;
  gpio_set_level(g_epd_dc_pin, dc);
}

/**
 *  @brief: Add the display as a device to the SPI bus.
 */
esp_err_t epd_spi_bus_add(spi_host_device_t host,
                          spi_device_handle_t* handle,
                          int cs_pin) {
  spi_device_interface_config_t devcfg =
    {
     .clock_speed_hz = 2*1000*1000,           // Clock out at 2 MHz
     .mode = 0,                               // SPI mode 0
     .spics_io_num = cs_pin,                  // CS pin
     .queue_size = 7,                         // We want to be able to
                                              // queue 7 transactions
     .pre_cb = epd_spi_pre_transfer_callback, // Specify pre-transfer
                                              // callback to handle D/C
                                              // line
    };

  // Attach the display to the SPI bus
  return spi_bus_add_device(HSPI_HOST, &devcfg, handle);
}

/**
 *  @brief: Initialize the display.
 */
void epd_init(spi_device_handle_t spi, const uint8_t* lut,
              int dc_pin, int busy_pin)
{
  int cmd = 0;

  g_spi = spi;
  g_epd_dc_pin = dc_pin;
  g_epd_busy_pin = busy_pin;

  // Initialize non-SPI GPIOs
  gpio_set_direction(dc_pin, GPIO_MODE_OUTPUT);
  gpio_set_direction(busy_pin, GPIO_MODE_INPUT);

  printf("E-ink initialization.\n");

  // Send all the commands
  while (epd_init_cmds[cmd].databytes != 0xff) {
    epd_send_command(epd_init_cmds[cmd].cmd);
    epd_send_data(epd_init_cmds[cmd].data,
                  epd_init_cmds[cmd].databytes & 0x1F);
    cmd++;
  }

  epd_set_lut(lut);
}
