#include "display_driver.h"

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
//    int g = 0;
//    for (int y = 0; y < 12; y++) {
//       for (int x = 0; x < 9; x++) {
//            IT8951_draw_glyph(g++, x * 65 + 10, y * 65 + 10, 0);
//        }
//    }

    for (int x = 0; x < 60; x++) {
        for (int y = 0; y < 60; y++) IT8951_draw_pixel(100 + x, 100 + y, 100);
    }

    IT8951_wait_display_ready();
    IT8951_update_display();

    IT8951_destroy();
    return 0;
}
