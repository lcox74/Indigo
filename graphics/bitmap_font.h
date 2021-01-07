#ifndef _BITMAP_FONTS_H_
#define _BITMAP_FONTS_H_

#include <stdint.h>

struct glyph_t {
    const char              g;   /* Glyph ASCII Code */
    const uint8_t           w;   /* Glyph Width */
    const uint16_t          o_x; /* Glyph X Position in Bitmap */
    const uint16_t          o_y; /* Glyph Y Position in Bitmap */
    const uint16_t          o_w; /* Glyph Width in Bitmap */
    const uint16_t          o_h; /* Glyph Height in Bitmap */
};

typedef struct {
    const uint16_t          w;      /* Bitmap Width */
    const uint16_t          h;      /* Bitmap Height */
    const uint8_t * const   d;      /* Bitmap Data */
    const uint8_t           size;   /* Number Of Glyphs */
    const struct glyph_t   *glyphs; /* List of Glyphs */
    const uint8_t           start;  /* The offset for char codes */
} font_t;

#endif /* _BITMAP_FONTS_H_ */`