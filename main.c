#include "display_driver.h"

int main(int argc, char *argv[]) {
    printf("Initialising Display");
    uint16_t err;
    if ((err = IT8951_init())) {
        printf("Initialisation Error: %d", err);
        return err;
    }
    
    printf("Clearing Display\n");
    IT8951_wait_display_ready();
    IT8951_clear_display(0xFF); /* White */

    printf("Draw To Display\n");
    IT8951_draw_pixel(5,5,0x0);
    IT8951_draw_pixel(5,15,0xF);
    IT8951_draw_pixel(5,3,0x2);
    IT8951_draw_pixel(4,5,14);
    IT8951_draw_pixel(10,5,7);
    IT8951_draw_pixel(100,5,6);
    IT8951_wait_display_ready();
    IT8951_update_display();
    bcm2835_delay(1000);

    // IT8951_clear_display(0xFF); /* Black */

    printf("Clean Display\n");
    IT8951_destroy();
    return 0;
}