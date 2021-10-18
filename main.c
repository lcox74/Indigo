#include "display_driver.h"

int main(int argc, char *argv[]) {
    printf("Initialising Display\n");
    uint16_t err;
    if ((err = IT8951_init())) {
        printf("Initialisation Error: %d", err);
        return err;
    }

    IT8951_sys_info info;
    IT8951_get_system_info(&info);
    printf("Display (w, h) -> (%u, %u)\n", info.pw, info.ph);

    printf("Clearing Display\n");
    IT8951_wait_display_ready();
    IT8951_clear_display(0xFF); /* White */

    printf("Drawing Image\n");
    IT8951_draw_jpeg(100, 100, "test/image.jpg");

    // for (int x = 0; x < 60; x++) {
    //     for (int y = 0; y < 60; y++) IT8951_draw_pixel(100 + x, 100 + y, 100);
    // }

    IT8951_draw_text("Test", 200, 500, 0);

    IT8951_wait_display_ready();
    IT8951_update_display();

    IT8951_destroy();
    return 0;
}
