#include "display_driver.h"
#include "res/font.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>

#include <jpeglib.h>    // -ljpeg


/* SPI Pins */
#define CS                  0x0008 /* SPI Chip Selection (LOW active) */
#define HRDY 	            0x0018 /* Busy status output (LOW active) */
#define RST 	            0x0011 /* External reset (LOW active)     */

/* SPI Preamble Words */
#define IT8951_SPI_CD       0x6000 /* Command */
#define IT8951_SPI_WR       0x0000 /* Write Data */
#define IT8951_SPI_RD       0x1000 /* Read Data */

/* Host Interface Command Codes */
#define IT8951_HIC_SYS_RUN         0x0001 /* System running CMD */
#define IT8951_HIC_STANDBY         0x0002 /* Standby CMD */
#define IT8951_HIC_SLEEP           0x0003 /* Sleep CMD */
#define IT8951_HIC_REG_RD          0x0010 /* Register Read CMD */
#define IT8951_HIC_REG_WR          0x0011 /* Register Write CMD */
#define IT8951_HIC_MEM_BST_RD_T    0x0012 /* Memory Burst Read Trigger CMD */
#define IT8951_HIC_MEM_BST_RD_S    0x0013 /* Memory Burst Read Start CMD */
#define IT8951_HIC_MEM_BST_WR      0x0014 /* Memory Burst Write CMD */
#define IT8951_HIC_MEM_BST_END     0x0015 /* End Memory Burst Cycle CMD */
#define IT8951_HIC_LD_IMG          0x0020 /* Load Full Image CMD */
#define IT8951_HIC_LD_IMG_AREA     0x0021 /* Load Partial Image CMD */
#define IT8951_HIC_LD_IMG_END      0x0022 /* End Image Load CMD */

/* User Defined Commands */
#define DEF_CMD_VCOM               0x0039
#define DEF_CMD_GET_INFO           0x0302
#define DEF_CMD_DPY_AREA           0x0034
#define DEF_VAL_SCR_REFR           0x0002
#define DEF_VAL_VCOM	           0x09C4 /* VCOM Voltage val 2500 = -2.50V
                                             This is based on the number on
                                             the display ribbon */

/* System Registers */
#define SYS_REG_BASE               0x0000
#define SYS_REG_DISPLAY            0x1000
#define SYS_REG_MCSR               0x0200
#define SYS_REG_ADDR (SYS_REG_BASE + 0x04)     /* Addr of System Registers */
#define SYS_REG_LUT  (SYS_REG_DISPLAY + 0x224) /* Addr LUT Status Register */
#define SYS_REG_LISAR (SYS_REG_MCSR + 0x0008)  /* Addr Image Buffer Register */

/* Host Controller Functions */
void     IT8951_wait_ready(void);
void     IT8951_write_cmd(uint16_t);
void     IT8951_write_data(uint16_t);
void     IT8951_write_data_n(uint16_t *, uint32_t);
void     IT8951_write_arg(uint16_t, uint16_t *, uint16_t);
uint16_t IT8951_read_data(void);
void     IT8951_read_data_n(uint16_t *, uint32_t);

/* Host Interface Command Functions */
void     IT8951_sys_run(void);
void     IT8951_standby(void);
void     IT8951_sleep(void);
uint16_t IT8951_reg_rd(uint16_t);
void     IT8951_reg_wr(uint16_t, uint16_t);
void     IT8951_mem_bst_rd_t(uint32_t, uint32_t);
void     IT8951_mem_bst_rd_s(void);
void     IT8951_mem_bst_wr(uint32_t, uint32_t);
void     IT8951_mem_bst_end(void);

/* Host Interface Command Display Functions */
struct IT8951_img_info {
    uint16_t et;        /* Endian Type (Little/Big) */
    uint16_t bpp;       /* Pixel Format (Bits Per Pixel) */
    uint16_t rot;       /* Rotation */
    uint32_t fb_addr;   /* Frame Buffer Address */
    uint32_t ib_addr;   /* Image Buffer Address */
};

void     IT8951_ld_img_start(struct IT8951_img_info *);
void     IT8951_ld_img_end(void);

void IT8951_set_ib_base_addr(uint32_t);

/* Driver Registers */
uint16_t IT8951_get_register(uint16_t);
void     IT8951_set_register(uint16_t, uint16_t);

/* VCOM GET/SET */
uint16_t IT8951_get_vcom(void);
void     IT8951_set_vcom(uint16_t);

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
    if (DEF_VAL_VCOM != IT8951_get_vcom()) IT8951_set_vcom(DEF_VAL_VCOM);

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
    IT8951_read_data_n(data, sizeof(IT8951_sys_info) / 2);
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
    if (x > sys_info.ph || y > sys_info.pw) return;

    frame_buffer[x * sys_info.pw + y] = colour;
}

void
IT8951_draw_pixel_rgb(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    /* Using the liminosity method to convert to greyscale */
    uint8_t greyscale = (uint8_t)( (0.299f * (float)r) + 
                                   (0.587f * (float)g) + 
                                   (0.114f * (float)b) );

    IT8951_draw_pixel(x, y, greyscale);
}

#define FONT_DEFAULT 0
#define FONT_ROBOTO  1
#define FONT_WEATHER 2
struct glyph_t
font_selection(uint8_t font, const font_t *f, uint8_t index) {
    
    switch (font) {
        case FONT_DEFAULT:
        case FONT_ROBOTO:
            f = &default_font;
            break;
        case FONT_WEATHER:
            f = &weather_font;
            break;
    }

    index = (index <= f->start) ? index : index - f->start;
    if (index >= f->size) return f->glyphs[0];
    return f->glyphs[index];
}

uint16_t
IT8951_draw_glyph(uint8_t glyph, uint16_t x, uint16_t y, uint8_t font) {
    const font_t f;
    const struct glyph_t g = font_selection(font, &f, glyph);

    uint16_t w, h, iy, ix;
    for ((iy = g.o_y, h = 0); iy < g.o_y + g.o_h; (iy++, h++)) {     
        for ((ix = g.o_x, w = 0); ix < g.o_x + g.o_w; ix++) {
            IT8951_draw_pixel(x + w++, y + h, f.d[iy * f.w + ix]);
        }
    }

    return g.w; /* Returns the width of the glyph */
}
void
IT8951_draw_text(const char *text, uint16_t x, uint16_t y, uint8_t font) {
    const uint16_t sx = x; /* Starting x position for alignment */
    
    while (*text) {
        switch(*text) {
            case '\n':
            case '\r':
                y += 65; /* Fix this later */
                x = sx;
                break;
            case '\t':
                for (uint8_t i = 0; i < 4; i++) 
                    x += IT8951_draw_glyph(*text, x, y, font);
                break;
            default:
                x += IT8951_draw_glyph(*text, x, y, font);
        }
        text++;
    }
}

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



/* Modifying teh sample code from https://gist.github.com/PhirePhly/3080633 */
void
IT8951_draw_jpeg(uint16_t x, uint16_t y, const char *file) {
    int rc, i;
    struct stat file_info;
    unsigned long jpg_size;
    unsigned char *jpg_buffer;

    // Variables for the decompressor itself
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Variables for the output buffer, and how long each row is
    unsigned long bmp_size;
    unsigned char *bmp_buffer;
    int row_stride, width, height, pixel_size;

    // Load the jpeg data from a file into a memory buffer for 
	// the purpose of this demonstration.
	// Normally, if it's a file, you'd use jpeg_stdio_src, but just
	// imagine that this was instead being downloaded from the Internet
	// or otherwise not coming from disk
	rc = stat(file, &file_info);
	if (rc) return;

	jpg_size = file_info.st_size;
	jpg_buffer = (unsigned char*) malloc(jpg_size + 100);

	int fd = open(file, O_RDONLY);
	i = 0;
	while (i < jpg_size) {
		rc = read(fd, jpg_buffer + i, jpg_size - i);
		i += rc;
	}
	close(fd);

	// Allocate a new decompress struct, with the default error handler.
	// The default error handler will exit() on pretty much any issue,
	// so it's likely you'll want to replace it or supplement it with
	// your own.
	cinfo.err = jpeg_std_error(&jerr);	
	jpeg_create_decompress(&cinfo);


	// Configure this decompressor to read its data from a memory 
	// buffer starting at unsigned char *jpg_buffer, which is jpg_size
	// long, and which must contain a complete jpg already.
	//
	// If you need something fancier than this, you must write your 
	// own data source manager, which shouldn't be too hard if you know
	// what it is you need it to do. See jpeg-8d/jdatasrc.c for the 
	// implementation of the standard jpeg_mem_src and jpeg_stdio_src 
	// managers as examples to work from.
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);


	// Have the decompressor scan the jpeg header. This won't populate
	// the cinfo struct output fields, but will indicate if the
	// jpeg is valid.
	rc = jpeg_read_header(&cinfo, TRUE);

	if (rc != 1) {
        free(jpg_buffer);
        return;
	}

	// By calling jpeg_start_decompress, you populate cinfo
	// and can then allocate your output bitmap buffers for
	// each scanline.
	jpeg_start_decompress(&cinfo);
	
	width = cinfo.output_width;
	height = cinfo.output_height;
	pixel_size = cinfo.output_components;

	bmp_size = width * height * pixel_size;
	bmp_buffer = (unsigned char*) malloc(bmp_size);

	// The row_stride is the total number of bytes it takes to store an
	// entire scanline (row). 
	row_stride = width * pixel_size;

	//
	// Now that you have the decompressor entirely configured, it's time
	// to read out all of the scanlines of the jpeg.
	//
	// By default, scanlines will come out in RGBRGBRGB...  order, 
	// but this can be changed by setting cinfo.out_color_space
	//
	// jpeg_read_scanlines takes an array of buffers, one for each scanline.
	// Even if you give it a complete set of buffers for the whole image,
	// it will only ever decompress a few lines at a time. For best 
	// performance, you should pass it an array with cinfo.rec_outbuf_height
	// scanline buffers. rec_outbuf_height is typically 1, 2, or 4, and 
	// at the default high quality decompression setting is always 1.
	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}

	// Once done reading *all* scanlines, release all internal buffers,
	// etc by calling jpeg_finish_decompress. This lets you go back and
	// reuse the same cinfo object with the same settings, if you
	// want to decompress several jpegs in a row.
	//
	// If you didn't read all the scanlines, but want to stop early,
	// you instead need to call jpeg_abort_decompress(&cinfo)
	jpeg_finish_decompress(&cinfo);

	// At this point, optionally go back and either load a new jpg into
	// the jpg_buffer, or define a new jpeg_mem_src, and then start 
	// another decompress operation.
	
	// Once you're really really done, destroy the object to free everything
	jpeg_destroy_decompress(&cinfo);
	// And free the input buffer
	free(jpg_buffer);

    for (i = 0; i < bmp_size/3; i++) {
        IT8951_draw_pixel_rgb(width - (x + (i % width)), y + (i / width), 
                bmp_buffer[i*3], bmp_buffer[(i*3)+1], bmp_buffer[(i*3)+2]);
    }

	free(bmp_buffer);

}
