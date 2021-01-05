#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Requires bcm2835 Library http://www.airspayce.com/mikem/bcm2835 */
#include <bcm2835.h>

/* Utility Functions */
#define U32_H(x)        (uint16_t)((x >> 16) & 0x0000FFFF)
#define U32_L(x)        (uint16_t)((x >> 0 ) & 0x0000FFFF)
#define U16_EBR(e,b,r)  (uint16_t)((e << 8) | (b << 4) | (r))
#define U16_U32(l,h)    (uint32_t) (l | (h << 16))

/* System Info */
typedef struct 
{
    uint16_t pw;        /* Panel Width */
    uint16_t ph;        /* Panel Height */
    uint16_t ib_addr_l; /* Image Buffer Address Low */
    uint16_t ib_addr_h; /* Image Buffer Address High */
    uint16_t fw[8]; 	/* Firmware Version */
    uint16_t lut[8]; 	/* LUT Version */
} IT8951_sys_info;

/* System API */
uint8_t  IT8951_init(void);
void     IT8951_destroy(void);
void     IT8951_get_system_info(void *);
void     IT8951_wait_display_ready(void);
void     IT8951_clear_display(uint8_t);
void     IT8951_draw_pixel(uint16_t, uint16_t, uint8_t);
void     IT8951_update_display(void);

#endif /* _DRIVER_H_ */