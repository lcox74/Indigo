#include "display_driver.h"

#include "res/font.h"


uint8_t draw_glyph(uint8_t glyph, uint16_t x, uint16_t y) {
	struct glyph_t g = weather_font.glyphs[glyph];

	uint16_t w = 0, h = 0;
	for (uint16_t iy = g.o_y; iy < g.o_y + g.o_h; iy++) {		
		for (uint16_t ix = g.o_x; ix < g.o_x + g.o_w; ix++) {
			uint64_t pos = iy * weather_font.w + ix;
			IT8951_draw_pixel(x + w, y + h, weather_font.d[pos]);
			w++;
		}
		h++;
		w = 0;
	}


	return g.w; /* Returns the width of the glyph */

}

int main(int argc, char *argv[]) {
    printf("Initialising Display\n");
    uint16_t err;
    if ((err = IT8951_init())) {
        printf("Initialisation Error: %d", err);
        return err;
    }
    
    printf("Clearing Display\n");
    IT8951_wait_display_ready();
    IT8951_clear_display(0xFF); /* White */

    printf("Drawing Glyphs\n");
    int g = 0;
    for (int y = 0; y < 9; y++) {
    	for (int x = 0; x < 12; x++) {
    	    draw_glyph(g++, x * 65 + 10, y * 65 + 10);
	}
    }

    IT8951_wait_display_ready();
    IT8951_update_display();

    IT8951_destroy();
    return 0;
}
