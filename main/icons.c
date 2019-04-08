#include "icons.h"

#include <stdint.h>
#include <stdlib.h>

extern const uint8_t sun_raw_start[]
  asm("_binary_sun_raw_start");
extern const uint8_t moon_raw_start[]
  asm("_binary_moon_raw_start");
extern const uint8_t cloudy_raw_start[]
  asm("_binary_cloudy_raw_start");
extern const uint8_t mist_raw_start[]
  asm("_binary_mist_raw_start");
extern const uint8_t light_rain_raw_start[]
  asm("_binary_light_rain_raw_start");
extern const uint8_t medium_hail_raw_start[]
  asm("_binary_medium_hail_raw_start");
extern const uint8_t medium_rain_raw_start[]
  asm("_binary_medium_rain_raw_start");
extern const uint8_t medium_sleet_raw_start[]
  asm("_binary_medium_sleet_raw_start");
extern const uint8_t medium_snow_raw_start[]
  asm("_binary_medium_snow_raw_start");
extern const uint8_t heavy_rain_raw_start[]
  asm("_binary_heavy_rain_raw_start");
extern const uint8_t heavy_hail_raw_start[]
  asm("_binary_heavy_hail_raw_start");
extern const uint8_t heavy_snow_raw_start[]
  asm("_binary_heavy_snow_raw_start");
extern const uint8_t storm_raw_start[]
  asm("_binary_storm_raw_start");

int code_to_icon_id(int code, int day) {
  switch (code) {
  case 1000: // Sunny/clear
    return day ? ICON_SUN : ICON_MOON;
  case 1003: // Partly cloudy
  case 1006: // Cloudy
  case 1009: // Overcast
    return ICON_CLOUDY;
  case 1030: // Mist
  case 1135: // Fog
  case 1147: // Freezing fog
    return ICON_MIST;
  case 1063: // Patchy rain nearby
  case 1150: // Patchy light drizzle
  case 1153: // Patchy drizzle
  case 1168: // Freezing drizzle
  case 1180: // Patchy light rain
  case 1183: // Light rain
  case 1240: // Light rain shower
    return ICON_LIGHT_RAIN;
  case 1069: // Patchy sleet nearby
  case 1204: // Light sleet
  case 1207: // Moderate or heavy sleet
  case 1249: // Light sleet showers
  case 1252: // Moderate or heavy sleet showers
    return ICON_MEDIUM_SLEET;
  case 1072: // Patchy fleeting drizzle nearby
  case 1171: // Heavy freezing drizzle
  case 1186: // Moderate rain at times
  case 1189: // Moderate rain
    return ICON_MEDIUM_RAIN;
  case 1192: // Heavy rain at times
  case 1195: // Heavy rain
  case 1201: // Moderate or heavy freezing rain
  case 1243: // Moderate or heavy rain shower
  case 1246: // Torrential rain shower
    return ICON_HEAVY_RAIN;
  case 1087: // Thundery outbreaks in nearby
  case 1273: // Patchy light rain in area with thunder
  case 1276: // Moderate or heavy rain in area with thunder
    return ICON_STORM;
  case 1066: // Patchy snow nearby
  case 1210: // Patchy light snow
  case 1213: // Light snow
  case 1279: // Patchy light snow in area with thunder
  case 1114: // Blowing snow
  case 1216: // Patchy moderate snow
  case 1255: // Light snow showers
    return ICON_MEDIUM_SNOW;
  case 1219: // Moderate snow
  case 1222: // Patchy heavy snow
  case 1225: // Heavy snow
  case 1282: // Moderate or heavy snow in area with thunder
  case 1258: // Moderate or heavy snow showers
  case 1117: // Blizzard
    return ICON_HEAVY_SNOW;
  case 1237: // Ice pellets
    return ICON_MEDIUM_HAIL;
  case 1264: // Moderate or heavy showers of ice pellets
    return ICON_HEAVY_HAIL;
  default:
    return ICON_UNKNOWN;
  }
}

const uint8_t* get_icon(int icon_id) {
  switch (icon_id) {
  case ICON_SUN:
    return sun_raw_start;
  case ICON_MOON:
    return moon_raw_start;
  case ICON_CLOUDY:
    return cloudy_raw_start;
  case ICON_MIST:
    return mist_raw_start;
  case ICON_LIGHT_RAIN:
    return light_rain_raw_start;
  case ICON_MEDIUM_HAIL:
    return medium_hail_raw_start;
  case ICON_MEDIUM_RAIN:
    return medium_rain_raw_start;
  case ICON_MEDIUM_SLEET:
    return medium_sleet_raw_start;
  case ICON_MEDIUM_SNOW:
    return medium_snow_raw_start;
  case ICON_HEAVY_HAIL:
    return heavy_hail_raw_start;
  case ICON_HEAVY_RAIN:
    return heavy_rain_raw_start;
  case ICON_HEAVY_SNOW:
    return heavy_snow_raw_start;
  case ICON_STORM:
    return storm_raw_start;
  default:
    return NULL;
  }
}
