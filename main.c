#include "display_driver.h"

int main(int argc, char *argv[]) {
    uint16_t err;
    if ((err = IT8951_init())) {
        printf("Error: %d", err);
        return err;
    }

    IT8951_wait_display_ready();
    IT8951_clear_display(0x1);

    IT8951_wait_display_ready();
    IT8951_clear_display(0x0);

    IT8951_destroy();
    return 0;
}