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
    IT8951_draw_jpeg(0, 0, "test/image.jpg");


    IT8951_wait_display_ready();
    IT8951_update_display();

    IT8951_destroy();
    return 0;
}
