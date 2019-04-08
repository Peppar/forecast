#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdint.h>

void draw_text(uint8_t* buf, int width, int height,
               int x, int y, const char* s, int radj);

#endif
