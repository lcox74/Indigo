#ifndef _BITMAP_FONTS_H_
#define _BITMAP_FONTS_H_

#include <stdint.h>

struct kerning_t {
    const char  g;      /* Previous Glyph */
    const char  x_o;    /* Physical Position X Offset to Add*/
};

struct glyph_t {
    const char              g;   /* Glyph ASCII Code */
    const uint8_t           w;   /* Glyph Width */
    const uint16_t          x_o; /* Glyph Position in Bitmap */
    const uint8_t           k_s; /* Number of kerning Glyphs */
    const struct kerning_t *k;   /* Glyph Kerning to Other Glyphs */
};

struct bitmap_t {
    const uint16_t          w;   /* Width */
    const uint16_t          h;   /* Height */
    const uint8_t * const   d;   /* Bitmap Data */
};

typedef struct {
    const struct bitmap_t * data;   /* Bitmap Data */
    const uint8_t           size;   /* Number Of Glyphs */
    const struct glyph_t    glyphs; /* List of Glyphs */
} font_t;

#endif /* _BITMAP_FONTS_H_ */