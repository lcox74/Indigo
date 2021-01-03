#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Requires bcm2835 Library http://www.airspayce.com/mikem/bcm2835 */
#include <bcm2835.h>


#define CS                  0x0008 /* SPI Chip Selection (LOW active) */
#define HRDY 	            0x0018 /* Busy status output (LOW active) */
#define RST 	            0x0011 /* External reset (LOW active)     */
#define VCOM	            0x09C4 /* VCOM Voltage val 2500 = -2.50V  */

/* Endian Type Value */
#define ENDIAN_LITTLE       0x0000
#define ENDIAN_BIG          0x0001

/* Pixel Mode Value (Bits Per Pixel) */
#define PM_2BPP             0x0000
#define PM_3BPP             0x0001
#define PM_4BPP             0x0002
#define PM_8BPP             0x0003

/* Image Rotation */
#define ROT_0               0x0000
#define ROT_90              0x0001
#define ROT_180             0x0002
#define ROT_270             0x0003

/* SPI Preamble Words */
#define IT8951_SPI_CD       0x6000 /* Command */
#define IT8951_SPI_WR       0x0000 /* Write Data */
#define IT8951_SPI_RD       0x1000 /* Read Data */

/* Host Controller Functions */
void     IT8951_wait_ready(void);
void     IT8951_write_cmd(uint16_t);
void     IT8951_write_data(uint16_t);
uint16_t IT8951_read_data(void);
void     IT8951_write_partial_data(uint16_t *, uint32_t);
void     IT8951_read_partial_data(uint16_t *, uint32_t);
void     IT8951_write_arg(uint16_t, uint16_t *, uint16_t);

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
void     IT8951_draw_text(uint16_t, uint16_t, uint8_t, uint8_t, uint8_t);
void     IT8951_update_display(void);
void     IT8951_update_partial_display(uint16_t, uint16_t, uint16_t, uint16_t);

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

/* System Registers */
#define SYS_REG_BASE               0x0000
#define SYS_REG_DISPLAY            0x1000
#define SYS_REG_MCSR               0x0200
#define SYS_REG_ADDR (SYS_REG_BASE + 0x04)    /* Address of System Registers */
#define SYS_REG_LUT (SYS_REG_DISPLAY + 0x224) /* Address LUT Status Register */
#define SYS_REG_LISAR (SYS_REG_MCSR + 0x0008)

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

struct IT8951_partial_img_info {
    uint16_t x, y, w, h;
};

void     IT8951_ld_img_start(struct IT8951_img_info *);
void     IT8951_ld_img_partial_start(struct IT8951_img_info *, 
                             struct IT8951_partial_img_info *);
void     IT8951_ld_img_end(void);

void IT8951_set_ib_base_addr(uint32_t ib_addr);

/* Driver Registers */
uint16_t IT8951_get_register(uint16_t);
void     IT8951_set_register(uint16_t, uint16_t);

/* VCOM GET/SET */
uint16_t IT8951_get_vcom(void);
void     IT8951_set_vcom(uint16_t);

#endif /* _DRIVER_H_ */