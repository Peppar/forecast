#include "forecast_graphics.h"

#include <stdlib.h>
#include <string.h>

#include "e-ink.h"
#include "icons.h"
#include "text.h"

const char *temp_to_text(int temp) {
  static char buf[16];
  char *c = &buf[15];
  *c = '\0';
  //*--c = '*';

  if (temp == 0)
    *--c = '0';
  else {
    int neg = temp < 0;
    if (neg)
      temp = -temp;
    while (temp) {
      *--c = '0' + (temp % 10);
      temp /= 10;
    }
    if (neg)
      *--c = '-';
    //else
    //  *--c = '+';
  }

  return c;
}

void draw_temperature(uint8_t* buf, int x, int y, int temp, int radj) {
  const char *s = temp_to_text(temp);
  draw_text(buf, EPD_WIDTH, EPD_HEIGHT, x, y, s, radj);
}

esp_err_t draw_forecast(forecast_t* forecast) {
  uint8_t* buf = NULL;

  buf = malloc(EPD_WIDTH*EPD_HEIGHT/8);
  if (buf == NULL)
    goto err;

  /* Copy the appropriate weather icon to the buffer */
  const uint8_t* icon = get_icon(code_to_icon_id(forecast->code,
                                                 forecast->day));
  if (icon == NULL)
    goto err;
  memcpy(buf, icon, EPD_WIDTH*EPD_HEIGHT/8);

  /* Draw the minimum and maximum temperatures */
  draw_temperature(buf, 0, 198, forecast->temp_min, 0);
  draw_temperature(buf, 200, 198, forecast->temp_max, 1);

  epd_set_frame_memory(buf);
  epd_display_frame();

  free(buf);
  return ESP_OK;

 err:
  if (buf != NULL)
    free(buf);
  return ESP_FAIL;
}
