#include "display_driver.h"

struct IT8951_sys_info sys_info;
uint8_t *frame_buffer;
uint32_t image_buffer_addr;

uint16_t
IT8951_reg_vcom_rd(uint16_t cmd, uint16_t data) {
    IT8951_write_cmd(cmd);
    IT8951_write_data(data);
    return IT8951_read_data();
}

void
IT8951_reg_vcom_wr(uint16_t cmd, uint16_t data0, uint16_t data1) {
    IT8951_write_cmd(cmd);
    IT8951_write_data(data0);
    IT8951_write_data(data1);
}

/* Driver Registers */
uint16_t
IT8951_get_register(uint16_t addr) {
    return IT8951_reg_vcom_rd(IT8951_HIC_REG_RD, addr);
}
void
IT8951_set_register(uint16_t addr, uint16_t data) {
    IT8951_reg_vcom_wr(IT8951_HIC_REG_WR, addr, data);
}

/* VCOM get/set */
uint16_t
IT8951_get_vcom(void) {
    return IT8951_reg_vcom_rd(DEF_CMD_VCOM, 0x01);
}
void
IT8951_set_vcom(uint16_t vcom) {
    IT8951_reg_vcom_wr(DEF_CMD_VCOM, 0x01, vcom);
}

/*
 * Host Controller Functions
 */
void
IT8951_wait_ready(void) {
    uint8_t data = bcm2835_gpio_lev(HRDY);
    while(data == 0) data = bcm2835_gpio_lev(HRDY);
}

void
IT8951_write_to_data_bus(uint16_t data) {
    /* Write data to data bus */
    IT8951_wait_ready();
    bcm2835_spi_transfer(data >> 8);
    bcm2835_spi_transfer(data);
}

void
IT8951_read_to_data_bus(uint16_t *buf) {
    /* Read data to data bus */
    IT8951_wait_ready();
    (*buf) = bcm2835_spi_transfer(0x00) << 8;
    (*buf) |= bcm2835_spi_transfer(0x00);
}

void
IT8951_write_wr_func(uint16_t type, uint16_t data) {
    bcm2835_gpio_write(CS, LOW);
    IT8951_write_to_data_bus(type);
    IT8951_write_to_data_bus(data);
    bcm2835_gpio_write(CS, HIGH);
}

void
IT8951_write_cmd(uint16_t cmd) {
    IT8951_wait_ready();
    IT8951_write_wr_func(IT8951_SPI_CD, cmd);
}

void
IT8951_write_data(uint16_t data) {
    IT8951_wait_ready();
    IT8951_write_wr_func(IT8951_SPI_WR, data);
}

uint16_t
IT8951_read_data(void) {
    uint16_t buf;

    IT8951_wait_ready();
    bcm2835_gpio_write(CS,LOW);

    IT8951_write_to_data_bus(IT8951_SPI_RD);
    IT8951_read_to_data_bus(&buf); /* Dummy Read */
    IT8951_read_to_data_bus(&buf);

    bcm2835_gpio_write(CS,HIGH);
    return buf;
}

void
IT8951_write_partial_data(uint16_t *buf, uint32_t size) {
    IT8951_wait_ready();
    bcm2835_gpio_write(CS, LOW);

    IT8951_write_to_data_bus(IT8951_SPI_WR);
    for (uint32_t i = 0; i < size; i++) IT8951_write_to_data_bus(buf[i]);

    bcm2835_gpio_write(CS, HIGH);
}

void
IT8951_read_partial_data(uint16_t *buf, uint32_t size) {
    IT8951_wait_ready();
    bcm2835_gpio_write(CS,LOW);
    
    IT8951_write_to_data_bus(IT8951_SPI_RD);
    IT8951_read_to_data_bus(&buf[0]); /* Dummy Read */
    for (uint32_t i = 0; i < size; i++) IT8951_read_to_data_bus(&buf[i]);

    bcm2835_gpio_write(CS,HIGH);
}

void
IT8951_write_arg(uint16_t cmd, uint16_t *args, uint16_t size) {
    IT8951_write_cmd(cmd);
    for (uint16_t i = 0; i < size; i++) IT8951_write_data(args[i]);
}


void IT8951_mem_bst_rd_wr(uint16_t cmd, uint32_t addr, uint32_t size) {
    /* Set Arguments */
    uint16_t args[4] = {
        (uint16_t)  (addr & 0x0000FFFF),            /* Addr[15:0] */
        (uint16_t) ((addr >> 16) & 0x0000FFFF ),    /* Addr[25:16] */
        (uint16_t)  (size & 0x0000FFFF),            /* Cnt[15:0] */
        (uint16_t) ((size >> 16) & 0x0000FFFF )     /* Cnt[25:16] */
    };
    
    /* Write to Host Controller */
    IT8951_write_arg(cmd, args, 4);
}

/*
 * Host Interface Command Functions
 */

void
IT8951_sys_run(void) {
    IT8951_write_cmd(IT8951_HIC_SYS_RUN);
}
void
IT8951_standby(void) {
    IT8951_write_cmd(IT8951_HIC_STANDBY);
}
void
IT8951_sleep(void) {
    IT8951_write_cmd(IT8951_HIC_SLEEP);
}
uint16_t
IT8951_reg_rd(uint16_t addr) {
    IT8951_write_cmd(IT8951_HIC_REG_RD);
    IT8951_write_data(addr);
    return IT8951_read_data();
}
void
IT8951_reg_wr(uint16_t addr, uint16_t data) {
    IT8951_write_cmd(IT8951_HIC_REG_WR);
    IT8951_write_data(addr);
    IT8951_write_data(data);
}
void
IT8951_mem_bst_rd_t(uint32_t addr, uint32_t size) {
    IT8951_mem_bst_rd_wr(IT8951_HIC_MEM_BST_RD_T, addr, size);
}
void
IT8951_mem_bst_rd_s(void) {
    IT8951_write_cmd(IT8951_HIC_MEM_BST_RD_S);
}
void
IT8951_mem_bst_wr(uint32_t addr, uint32_t size) {
    IT8951_mem_bst_rd_wr(IT8951_HIC_MEM_BST_WR, addr, size);
}
void
IT8951_mem_bst_end(void) {
    IT8951_write_cmd(IT8951_HIC_MEM_BST_END);
}

/*
 * Graphics Functions
 */
void
IT8951_ld_img_start(struct IT8951_img_info *info) {
    IT8951_write_cmd(IT8951_HIC_LD_IMG);
    IT8951_write_data((info->et << 8) | (info->bpp << 4) | (info->rot));
}
void
IT8951_ld_img_partial_start(struct IT8951_img_info *info, 
                            struct IT8951_partial_img_info *rect) {
    uint16_t arg[5] = {
        (uint16_t) (info->et << 8) | (info->bpp << 4) | (info->rot),
        (uint16_t) rect->x,
        (uint16_t) rect->y,
        (uint16_t) rect->w,
        (uint16_t) rect->h
    };

    IT8951_write_arg(IT8951_HIC_LD_IMG_AREA, arg, 5);
}
void
IT8951_ld_img_end(void) {
    IT8951_write_cmd(IT8951_HIC_LD_IMG_END);
}

void
IT8951_pixel_buffer_wr(struct IT8951_img_info *info, 
                       struct IT8951_partial_img_info *rect) {
    uint16_t *fbuf = (uint16_t *)info->fb_addr;

    IT8951_set_ib_base_addr(info->ib_addr);
    IT8951_ld_img_partial_start(info, rect);

    for (uint32_t j = 0; j < rect->h; j++) {
        for (uint32_t i = 0; i < rect->w; i++) {
            IT8951_write_data(*fbuf);
            fbuf++;
        }
    }
    
    IT8951_ld_img_end();
}

void
IT8951_display_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                      uint16_t display_mode) {
    IT8951_write_cmd(DEF_CMD_DPY_AREA);
    IT8951_write_data(x);
    IT8951_write_data(y);
    IT8951_write_data(w);
    IT8951_write_data(h);
    IT8951_write_data(display_mode);
}

/* 
 * ===========================================================================
 * |                                API                                      |
 * ===========================================================================
 */

/*
 * Initialise the Driver, returns 0 on success. On failure return:
 *      1 = BCM2835 Initialisation Error
 *      2 = Frame Buffer Allocation Error
 */
uint8_t
IT8951_init(void) {
    if (!bcm2835_init()) return 1;

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);

    bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);  
    bcm2835_gpio_fsel(HRDY, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(RST, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_write(CS, HIGH);
    bcm2835_gpio_write(RST, LOW);
    bcm2835_delay(100);
    bcm2835_gpio_write(RST, HIGH);

    IT8951_get_system_info(&sys_info);

    if (!(frame_buffer = malloc(sys_info.pw * sys_info.ph))) return 2;

    image_buffer_addr = sys_info.ib_addr_l | (sys_info.ib_addr_h << 16);

    IT8951_reg_wr(SYS_REG_ADDR, 0x0001);
    if (VCOM != IT8951_get_vcom()) IT8951_set_vcom(VCOM);

    return 0;
}
void
IT8951_destroy(void) {
    free(frame_buffer);

    bcm2835_spi_end();
    bcm2835_close();
}
void
IT8951_get_system_info(struct IT8951_sys_info *buf) {
    uint16_t *data = (uint16_t *) buf;

    IT8951_write_cmd(DEF_CMD_GET_INFO);
    IT8951_read_partial_data(data, sizeof(struct IT8951_sys_info) / 2);
}
void
IT8951_wait_display_ready(void) {
    while(IT8951_reg_rd(SYS_REG_LUT));
}
void
IT8951_clear_display(uint8_t colour) {
    memset(frame_buffer, colour, sys_info.pw * sys_info.ph);
    IT8951_update_display();
}
void
IT8951_draw_pixel(uint16_t x, uint16_t y, uint8_t colour) {}
void
IT8951_draw_text(uint16_t x, uint16_t y, uint8_t c, uint8_t fg, uint8_t bg) {}
void
IT8951_update_display(void) {
    IT8951_update_partial_display(0, 0, sys_info.pw, sys_info.ph);
}
void
IT8951_update_partial_display(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    struct IT8951_img_info info;
    struct IT8951_partial_img_info rect;
    info.fb_addr = (uint32_t) frame_buffer;
    info.et      = ENDIAN_LITTLE;
    info.bpp     = PM_8BPP;
    info.rot     = ROT_0;
    info.ib_addr = image_buffer_addr;
    rect.x       = x;
    rect.y       = y;
    rect.w       = w;
    rect.h       = h;

    IT8951_pixel_buffer_wr(&info, &rect);
    IT8951_display_buffer(0, 0, sys_info.pw, sys_info.ph, 0);
}