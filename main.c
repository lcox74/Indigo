#include "display_driver.h"

int main(int argc, char *argv[]) {
    printf("Initialising Display...");
    uint16_t err;
    if ((err = IT8951_init())) {
        printf("\t[FAILED]\n");
        printf("Error: %d", err);
        return err;
    }
    printf("\t[DONE]\n");
    
    printf("Clearing Display\n");
    IT8951_wait_display_ready();
    IT8951_clear_display(0x00); /* White */

    printf("Draw To Display\n");
    IT8951_draw_pixel(0,0,0);
    IT8951_wait_display_ready();
    IT8951_update_display();

    // IT8951_clear_display(0xFF); /* Black */

    printf("Clean Display\n");
    IT8951_destroy();
    return 0;
}