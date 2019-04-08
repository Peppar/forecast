#ifndef __ICONS_H__
#define __ICONS_H__

#include <stdint.h>

#define ICON_UNKNOWN 0
#define ICON_SUN 1
#define ICON_MOON 2
#define ICON_CLOUDY 3
#define ICON_MIST 4
#define ICON_LIGHT_RAIN 5
#define ICON_MEDIUM_HAIL 6
#define ICON_MEDIUM_RAIN 7
#define ICON_MEDIUM_SLEET 8
#define ICON_MEDIUM_SNOW 9
#define ICON_HEAVY_RAIN 10
#define ICON_HEAVY_HAIL 11
#define ICON_HEAVY_SNOW 12
#define ICON_STORM 13

int code_to_icon_id(int code, int day);
const uint8_t* get_icon(int icon_id);

#endif
