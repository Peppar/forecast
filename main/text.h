#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdint.h>

/* Drawn a string of hand-written digits zero to nine, plus symbol,
 * minus symbol and degree sign ('*')
 *
 * The (x,y) coordinates specify the bottom-left corner of the string,
 * or bottom-right corner if right-adjusted.
 */
void draw_text(uint8_t* buf, int width, int height,
               int x, int y, const char* s, int radj);

/* Calculate the width of the string in pixels. */
int text_width(const char* buf);

#endif
