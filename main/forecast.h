#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "esp_system.h"

typedef struct {
  int day;
  int code;
  int temp_min;
  int temp_max;
} forecast_t;

esp_err_t get_forecast(forecast_t* forecast);

#endif
