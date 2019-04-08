/* Forecast by Oskar Holstensson
 *
 * Based on SPI master example in ESP IDF, which is in the public domain,
 * and on the HelTec Arduino library (https://github.com/HelTecAutomation/e-ink),
 * which specifies no license that I can see.
 *
 * All code, except e-ink.c and e-ink.h, is in the public domain.
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *
 * 8 April 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "soc/gpio_struct.h"
#include "tcpip_adapter.h"

#include "e-ink.h"
#include "forecast.h"
#include "forecast_graphics.h"

/* ESP32 GPIO pins for the SPI bus */
#define PIN_NUM_MOSI 5
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   18

/* Pins for the busy and data/control signals.
 * The busy pin is currently ignored, and can be skipped. */
#define PIN_NUM_BUSY 21
#define PIN_NUM_DC   4

/* Sleep interval (in microseconds) between forecast updates */
/* 900 seconds is a quarter of an hour */
#define SLEEP_INTERVAL (900*1000*1000)

/* FreeRTOS event group to signal when we are connected & ready to
 * make a request */
static EventGroupHandle_t g_wifi_event_group;

/* Global EPD device handle */
static spi_device_handle_t g_epd;

/* The event group allows multiple bits for each event,
 * but we only care about one event - are we connected
 * to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const char *TAG = "fc";

static void forecast_task(void *parm);

static esp_err_t event_handler(void *ctx, system_event_t *event) {
  switch(event->event_id) {
  case SYSTEM_EVENT_STA_START:
    xTaskCreate(forecast_task, "forecast_task", 4096,
                NULL, 3, NULL);
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    xEventGroupSetBits(g_wifi_event_group, CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    esp_wifi_connect();
    xEventGroupClearBits(g_wifi_event_group, CONNECTED_BIT);
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void wifi_init(void) {
  tcpip_adapter_init();
  g_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  wifi_config_t sta_config =
    {
     .sta =
     {
      .ssid = CONFIG_WIFI_SSID,
      .password = CONFIG_WIFI_PASSWORD,
      .bssid_set = false,
     }
    };
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());
}

void app_main() {
  esp_err_t ret;
  spi_bus_config_t buscfg =
    {
     .miso_io_num = -1,
     .mosi_io_num = PIN_NUM_MOSI,
     .sclk_io_num = PIN_NUM_CLK,
     .quadwp_io_num = -1,
     .quadhd_io_num = -1,
     .max_transfer_sz = 5000, /* 5000 is a full EPD frame */
    };

  /* Initialize the SPI bus */
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
  ESP_ERROR_CHECK(ret);

  /* Attach the EPD to the SPI bus */
  ret = epd_spi_bus_add(HSPI_HOST, &g_epd, PIN_NUM_CS);
  ESP_ERROR_CHECK(ret);

  /* Initialize NVS flash */
  ret = nvs_flash_init();
  ESP_ERROR_CHECK(ret);

  /* Initialize Wifi */
  wifi_init();

  /* Wait forever */
  while(1) {
    vTaskSuspend(NULL);
  }
}

static void forecast_task(void *parm) {
  forecast_t forecast;
  forecast_t prev_forecast;
  int first_forecast = 1;

  while (1) {
    /* Initialize the display */
    epd_init(g_epd, lut_full_update, PIN_NUM_DC, PIN_NUM_BUSY);

    /* Wait for the callback to set the CONNECTED_BIT in the
     * event group.
     */
    xEventGroupWaitBits(g_wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP, attempting to fetch forecast");

    for (int retry = 0; retry < 5; ++retry) {
      if (get_forecast(&forecast) == ESP_OK) {
        ESP_LOGI(TAG, "Got forecast: code=%d min=%d max=%d",
                 forecast.code, forecast.temp_min, forecast.temp_max);

        if (first_forecast
            || (forecast.code != prev_forecast.code)
            || (forecast.temp_min != prev_forecast.temp_min)
            || (forecast.temp_max != prev_forecast.temp_max)) {
          first_forecast = 0;
          prev_forecast = forecast;
          if (draw_forecast(&forecast) == ESP_OK) {
            ESP_LOGI(TAG, "Successfully drew forecast");
            break;
          }
          else {
            ESP_LOGE(TAG, "Unable to draw forecast");
          }
        }
      }
      else {
        ESP_LOGE(TAG, "Unable to fetch forecast");
      }
      /* 10 second delay between attempts. */
      vTaskDelay(10000 / portTICK_RATE_MS);
    }

    /* Wait for the display to finish updating, then put it to sleep */
    epd_wait_busy();
    epd_sleep();

    /* Put the module in deep sleep */
    ESP_LOGI(TAG, "Going to deep sleep");
    esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL);
    esp_deep_sleep_start();
    ESP_LOGI(TAG, "Woke up from sleep");
  }
}
