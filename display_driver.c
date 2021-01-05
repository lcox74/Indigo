#include "display_driver.h"

IT8951_sys_info sys_info;
uint8_t *frame_buffer;
uint32_t image_buffer_addr;

void
IT8951_wr16(uint16_t data) {
    bcm2835_spi_transfer(data >> 8);
    bcm2835_spi_transfer(data);
}

uint16_t
IT8951_rd16() {
    return (bcm2835_spi_transfer(0x00) << 8) | bcm2835_spi_transfer(0x00);
}

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

/* VCOM get/set */
uint16_t
IT8951_get_vcom(void) {
    return IT8951_reg_vcom_rd(DEF_CMD_VCOM, 0x00);
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
IT8951_write_func(const uint16_t preamble, const uint16_t data) {
    bcm2835_gpio_write(CS,LOW);

    IT8951_wait_ready();
    IT8951_wr16(preamble);
        
    IT8951_wait_ready();
    IT8951_wr16(data);
        
    bcm2835_gpio_write(CS,HIGH);
}

void
IT8951_read_func(const uint16_t preamble) {
    IT8951_wait_ready();
    IT8951_wr16(preamble);
        
    IT8951_wait_ready();
    IT8951_rd16(); /* Dummy Read */
}

void
IT8951_write_cmd(uint16_t cmd) {
    const uint16_t preamble = IT8951_SPI_CD;
    IT8951_write_func(preamble, cmd);
}

void
IT8951_write_data(uint16_t data) {
    const uint16_t preamble = IT8951_SPI_WR;
    IT8951_write_func(preamble, data);
}

void
IT8951_write_data_n(uint16_t *buf, uint32_t size) {
    const uint16_t preamble = IT8951_SPI_WR;

    bcm2835_gpio_write(CS, LOW);

    IT8951_wait_ready();
    IT8951_wr16(preamble);

    IT8951_wait_ready();
    for (uint32_t i = 0; i < size; i++) IT8951_wr16(buf[i]);

    bcm2835_gpio_write(CS, HIGH);
}

uint16_t
IT8951_read_data(void) {
    const uint16_t preamble = IT8951_SPI_RD;
    uint16_t data;

    bcm2835_gpio_write(CS,LOW);
    IT8951_read_func(preamble);

    IT8951_wait_ready();
    data = IT8951_rd16();

    bcm2835_gpio_write(CS,HIGH);

    return data;
}

void
IT8951_read_data_n(uint16_t *buf, uint32_t size) {
    const uint16_t preamble = IT8951_SPI_RD;

    bcm2835_gpio_write(CS,LOW);
    IT8951_read_func(preamble);

    IT8951_wait_ready();
    for (uint32_t i = 0; i < size; i++) buf[i] = IT8951_rd16();

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
        U32_L(addr),                           /* Addr[15:0] */
        U32_H(addr),                           /* Addr[25:16] */
        U32_L(size),                           /* Cnt[15:0] */
        U32_H(size)                            /* Cnt[25:16] */
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
    IT8951_write_data(U16_EBR(info->et, info->bpp, info->rot));
}
void
IT8951_ld_img_end(void) {
    IT8951_write_cmd(IT8951_HIC_LD_IMG_END);
}

void
IT8951_set_ib_base_addr(uint32_t addr) {
    IT8951_reg_wr(SYS_REG_LISAR + 2, U32_H(addr));
    IT8951_reg_wr(SYS_REG_LISAR    , U32_L(addr));
}

void
IT8951_pixel_buffer_wr(struct IT8951_img_info *info) {
    uint16_t *fbuf = (uint16_t *)info->fb_addr;

    IT8951_set_ib_base_addr(info->ib_addr);
    IT8951_ld_img_start(info);

    for (uint32_t j = 0; j < sys_info.ph; j++) {
        for (uint32_t i = 0; i < sys_info.pw / 2; i++) {
            IT8951_write_data(*fbuf);
            fbuf++;
        }
    }
    
    IT8951_ld_img_end();
}

void
IT8951_display_buffer() {
    IT8951_write_cmd(DEF_CMD_DPY_AREA);
    IT8951_write_data(0x00);            /* x0 (top-left) */
    IT8951_write_data(0x00);            /* y0 (top-left) */
    IT8951_write_data(sys_info.pw);     /* x1 (bottom-right) */
    IT8951_write_data(sys_info.ph);     /* y1 (bottom-right) */
    IT8951_write_data(DEF_VAL_SCR_REFR);
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

    image_buffer_addr = U16_U32(sys_info.ib_addr_l, sys_info.ib_addr_h);

    IT8951_reg_wr(SYS_REG_ADDR, 0x0001);
    if (VCOM != IT8951_get_vcom()) IT8951_set_vcom(VCOM);

    printf("Display Size %d x %d\n", sys_info.pw, sys_info.ph);
    printf("Image Buffer Addr %x\n", U16_U32(sys_info.ib_addr_l, 
                                             sys_info.ib_addr_h));
    printf("VCOM Value %d\n", IT8951_get_vcom());

    return 0;
}
void
IT8951_destroy(void) {
    free(frame_buffer);

    bcm2835_spi_end();
    bcm2835_close();
}
void
IT8951_get_system_info(void *buf) {
    uint16_t *data = (uint16_t *) buf;

    IT8951_write_cmd(DEF_CMD_GET_INFO);
    IT8951_read_partial_data(data, sizeof(IT8951_sys_info) / 2);
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
IT8951_draw_pixel(uint16_t x, uint16_t y, uint8_t colour) {
    const uint16_t colours[0x10] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    colour = (colour >= 0x10) ? colour >> 0xf : colour;
    x %= sys_info.pw;
    y %= sys_info.ph;

    frame_buffer[y * sys_info.pw + x] = colours[colour];
}
void
IT8951_draw_text(uint16_t x, uint16_t y, uint8_t c, uint8_t fg, uint8_t bg) {}
void
IT8951_update_display(void) {
    struct IT8951_img_info info;
    info.fb_addr = (uint32_t) frame_buffer;
    info.et      = 0x0000;                     /* Little Endian */
    info.bpp     = 0x0003;                     /* 8 Bits Per Pixel */
    info.rot     = 0x0000;                     /* 0 Deg Rotation */
    info.ib_addr = image_buffer_addr;

    IT8951_pixel_buffer_wr(&info);
    IT8951_display_buffer();
}
