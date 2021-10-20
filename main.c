#include "display_driver.h"

int main(int argc, char *argv[]) {
    uint16_t err;
    if ((err = IT8951_init())) {
        printf("Display Error: %d", err);
        return err;
    }

    IT8951_sys_info info;

    IT8951_wait_display_ready();
    IT8951_clear_display(0xFF); /* White */

    IT8951_draw_jpeg(0, 0, "./image.jpg");


    IT8951_wait_display_ready();
    IT8951_update_display();

    IT8951_destroy();
    return 0;
}
