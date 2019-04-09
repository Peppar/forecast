#include "text.h"

#include <stddef.h>
#include <stdint.h>

extern const uint8_t glyphs_raw_start[] asm("_binary_glyphs_raw_start");

#define GLYPH_COUNT 13
#define INDEX_COUNT 8
#define GLYPH_WIDTH 64
#define GLYPH_HEIGHT 64
#define GLYPHS_WIDTH (GLYPH_WIDTH*GLYPH_COUNT)
#define GLYPHS_HEIGHT (GLYPH_HEIGHT*INDEX_COUNT)
#define GLYPHS_ROW_SIZE (GLYPHS_WIDTH*2/8)

static int char_to_glyph(char c) {
  switch (c) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case '+': return 10;
  case '-': return 11;
  case '*': return 12;
  default: return -1;
  }
}

static int glyph_width(int glyph) {
  switch (glyph) {
  case 0: return 38;
  case 1: return 16;
  case 2: return 30;
  case 3: return 28;
  case 4: return 24;
  case 5: return 30;
  case 6: return 25;
  case 7: return 25;
  case 8: return 28;
  case 9: return 28;
  case 10: return 26;
  case 11: return 32;
  case 12: return 22;
  default: return 0;
  }
}

static const uint8_t* glyph_start(int glyph, int index) {
  if (glyph < 0 || glyph >= GLYPH_COUNT || index < 0 || index >= INDEX_COUNT)
    return NULL;
  return glyphs_raw_start+(glyph*GLYPH_WIDTH*2/8
                           +index*GLYPH_HEIGHT*GLYPHS_ROW_SIZE);
}

static void draw_glyph(uint8_t* buf, int width, int height,
                       int x, int y, int glyph, int index,
                       int set) {
  const uint8_t* g;
  int wbytes;
  int xbytes, xshift, gx, gy;

  g = glyph_start(glyph, index);
  if (g == NULL)
    return;

  if (!set)
    ++g;

  wbytes = width / 8;
  xbytes = x >> 3; /* Let's hope the compiler infers an arithmetic shift */
  xshift = x & 7;

  for (gy = 0; gy < 64; ++gy) {
    uint8_t rem = (set ? 0xFF : 0x00) << (8 - xshift);
    if (y+gy < 0 || y+gy >= height)
      continue;
    for (gx = 0; gx < 9; ++gx) {
      uint8_t gdat;
      uint8_t data;
      gdat = gx != 8 ? g[2*gx+GLYPHS_ROW_SIZE*gy] : set ? 0xFF : 0x00;
      data = rem | (gdat >> xshift);
      rem = gdat << (8 - xshift);
      if (xbytes+gx < 0 || xbytes+gx >= wbytes)
        continue;
      if (set)
        buf[xbytes+gx+wbytes*(y+gy)] &= data;
      else
        buf[xbytes+gx+wbytes*(y+gy)] |= data;
    }
  }
}

/* The next index for each glyph to be drawn. We cycle through the indexes
 * so that glyphs start repeating after eight renderings.
 */
static int g_glyph_indexes[GLYPH_COUNT] = {0};

void draw_text(uint8_t* buf, int width, int height,
               int x, int y, const char* s, int radj) {
  int indexes[13];

  /* The glyphs' origins are 16 from the left and 16 from the bottom */
  x -= 16;
  y -= 48;

  if (radj)
    x -= text_width(s);

  /* First clear all of the glyphs then set all of the glyphs */
  for (int set = 0; set != 2; ++set) {
    for (int i = 0; i < GLYPH_COUNT; ++i) {
      indexes[i] = g_glyph_indexes[i];
    }
    int xx = x;
    for (const char *ss = s; *ss; ++ss) {
      int glyph = char_to_glyph(*ss);
      if (glyph >= 0) {
        draw_glyph(buf, width, height, xx, y, glyph, indexes[glyph], set);
        xx += glyph_width(glyph);
        indexes[glyph] = (indexes[glyph] + 1) % INDEX_COUNT;
      }
    }
  }

  /* Update the glyph indexes */
  for (int i = 0; i < GLYPH_COUNT; ++i) {
    g_glyph_indexes[i] = indexes[i];
  }
}

int text_width(const char* s) {
  int width = 0;
  while (*s)
    width += glyph_width(char_to_glyph(*s++));
  return width;
}
