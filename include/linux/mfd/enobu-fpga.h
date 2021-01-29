/*
 * linux/mfd/enobu-fpga.h
 *
 * Functions to access eNOBU FPGA chip.
 *
 * Copyright (C) 2020 Leonardo S.p.A.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_MFD_ENOBU_FPGA_H
#define __LINUX_MFD_ENOBU_FPGA_H

/* All register addresses */
#define ENOBU_FPGA_REG_M2SLOT1		0x000
#define ENOBU_FPGA_REG_M2SLOT2		0x001
#define ENOBU_FPGA_REG_USBHUB		0x002

/* UART REGS */
#define ENOBU_FPGA_REG_UARTMODE		0x003
#define ENOBU_FPGA_REG_TTY1_CNF		0x004
#define ENOBU_FPGA_REG_TTY2_CNF		0x005

#define ENOBU_FPGA_REG_POELED		0x00b
#define ENOBU_FPGA_REG_SPARELED		0x00c

#define ENOBU_FPGA_REG_DEVRESET		0x016
#define ENOBU_FPGA_REG_KEYMNG		0x017

#define ENOBU_FPGA_REG_IRQ1		0x018
#define ENOBU_FPGA_REG_IRQ2		0x019
#define ENOBU_FPGA_REG_IRQ1MASK		0x01a
#define ENOBU_FPGA_REG_IRQ2MASK		0x01b


#define ENOBU_FPGA_REG_MTRXDISP         0x020
#define ENOBU_FPGA_REG_MTRXDISPMODE     0x034

#define ENOBU_FPGA_REG_TEST1            0x100
#define ENOBU_FPGA_REG_TEST2            0x101
#define ENOBU_FPGA_REG_TEST3            0x102
#define ENOBU_FPGA_REG_TEST4            0x200

#define ENOBU_FPGA_REG_HWVER		0x7FD
#define ENOBU_FPGA_REG_VER		0x7FE
#define ENOBU_FPGA_REG_REV		0x7FF

#define ENOBU_FPGA_REG_MAX		ENOBU_FPGA_REV
#define ENOBU_FPGA_REGISTER_COUNT	0x800


/* Register field definitions */


#define MATRIXDISP_MODE_FIXED	                0x00
#define MATRIXDISP_MODE_SCROLL	                0x01



struct enobu_fpga {
	struct device *dev;
	struct regmap *regmap;

	int irq;

	u8 int_val;
	u8 digin_status_reg;
	u8 keymng_reg;
};



int enobu_fpga_reg_read(struct enobu_fpga *enobufpga, unsigned int reg,
					unsigned int *val);
int enobu_fpga_reg_write(struct enobu_fpga *enobufpga, unsigned int reg,
			unsigned int val);
int enobu_fpga_set_bits(struct enobu_fpga *enobufpga, unsigned int reg,
		unsigned int mask, unsigned int val);
int enobu_fpga_clear_bits(struct enobu_fpga *enobufpga, unsigned int reg,
		unsigned int mask);

#endif /*  __LINUX_MFD_ENOBU_FPGA_H */



#if 0
/* Register field definitions */
#define TPS65217_CHIPID_CHIP_MASK	0xF0
#define TPS65217_CHIPID_REV_MASK	0x0F

#define TPS65217_PPATH_ACSINK_ENABLE	BIT(7)
#define TPS65217_PPATH_USBSINK_ENABLE	BIT(6)
#define TPS65217_PPATH_AC_PW_ENABLE	BIT(5)
#define TPS65217_PPATH_USB_PW_ENABLE	BIT(4)
#define TPS65217_PPATH_AC_CURRENT_MASK	0x0C
#define TPS65217_PPATH_USB_CURRENT_MASK	0x03

#define TPS65217_INT_PBM		BIT(6)
#define TPS65217_INT_ACM		BIT(5)
#define TPS65217_INT_USBM		BIT(4)
#define TPS65217_INT_PBI		BIT(2)
#define TPS65217_INT_ACI		BIT(1)
#define TPS65217_INT_USBI		BIT(0)
#define TPS65217_INT_SHIFT		4
#define TPS65217_INT_MASK		(TPS65217_INT_PBM | TPS65217_INT_ACM | \
					TPS65217_INT_USBM)

#define TPS65217_CHGCONFIG0_TREG	BIT(7)
#define TPS65217_CHGCONFIG0_DPPM	BIT(6)
#define TPS65217_CHGCONFIG0_TSUSP	BIT(5)
#define TPS65217_CHGCONFIG0_TERMI	BIT(4)
#define TPS65217_CHGCONFIG0_ACTIVE	BIT(3)
#define TPS65217_CHGCONFIG0_CHGTOUT	BIT(2)
#define TPS65217_CHGCONFIG0_PCHGTOUT	BIT(1)
#define TPS65217_CHGCONFIG0_BATTEMP	BIT(0)

#define TPS65217_CHGCONFIG1_TMR_MASK	0xC0
#define TPS65217_CHGCONFIG1_TMR_ENABLE	BIT(5)
#define TPS65217_CHGCONFIG1_NTC_TYPE	BIT(4)
#define TPS65217_CHGCONFIG1_RESET	BIT(3)
#define TPS65217_CHGCONFIG1_TERM	BIT(2)
#define TPS65217_CHGCONFIG1_SUSP	BIT(1)
#define TPS65217_CHGCONFIG1_CHG_EN	BIT(0)

#define TPS65217_CHGCONFIG2_DYNTMR	BIT(7)
#define TPS65217_CHGCONFIG2_VPREGHG	BIT(6)
#define TPS65217_CHGCONFIG2_VOREG_MASK	0x30

#define TPS65217_CHGCONFIG3_ICHRG_MASK	0xC0
#define TPS65217_CHGCONFIG3_DPPMTH_MASK	0x30
#define TPS65217_CHGCONFIG2_PCHRGT	BIT(3)
#define TPS65217_CHGCONFIG2_TERMIF	0x06
#define TPS65217_CHGCONFIG2_TRANGE	BIT(0)

#define TPS65217_WLEDCTRL1_ISINK_ENABLE	BIT(3)
#define TPS65217_WLEDCTRL1_ISEL		BIT(2)
#define TPS65217_WLEDCTRL1_FDIM_MASK	0x03

#define TPS65217_WLEDCTRL2_DUTY_MASK	0x7F

#define TPS65217_MUXCTRL_MUX_MASK	0x07

#define TPS65217_STATUS_OFF		BIT(7)
#define TPS65217_STATUS_ACPWR		BIT(3)
#define TPS65217_STATUS_USBPWR		BIT(2)
#define TPS65217_STATUS_PB		BIT(0)

#define TPS65217_PASSWORD_REGS_UNLOCK	0x7D

#define TPS65217_PGOOD_LDO3_PG		BIT(6)
#define TPS65217_PGOOD_LDO4_PG		BIT(5)
#define TPS65217_PGOOD_DC1_PG		BIT(4)
#define TPS65217_PGOOD_DC2_PG		BIT(3)
#define TPS65217_PGOOD_DC3_PG		BIT(2)
#define TPS65217_PGOOD_LDO1_PG		BIT(1)
#define TPS65217_PGOOD_LDO2_PG		BIT(0)

#define TPS65217_DEFPG_LDO1PGM		BIT(3)
#define TPS65217_DEFPG_LDO2PGM		BIT(2)
#define TPS65217_DEFPG_PGDLY_MASK	0x03

#define TPS65217_DEFDCDCX_XADJX		BIT(7)
#define TPS65217_DEFDCDCX_DCDC_MASK	0x3F

#define TPS65217_DEFSLEW_GO		BIT(7)
#define TPS65217_DEFSLEW_GODSBL		BIT(6)
#define TPS65217_DEFSLEW_PFM_EN1	BIT(5)
#define TPS65217_DEFSLEW_PFM_EN2	BIT(4)
#define TPS65217_DEFSLEW_PFM_EN3	BIT(3)
#define TPS65217_DEFSLEW_SLEW_MASK	0x07

#define TPS65217_DEFLDO1_LDO1_MASK	0x0F

#define TPS65217_DEFLDO2_TRACK		BIT(6)
#define TPS65217_DEFLDO2_LDO2_MASK	0x3F

#define TPS65217_DEFLDO3_LDO3_EN	BIT(5)
#define TPS65217_DEFLDO3_LDO3_MASK	0x1F

#define TPS65217_DEFLDO4_LDO4_EN	BIT(5)
#define TPS65217_DEFLDO4_LDO4_MASK	0x1F

#define TPS65217_ENABLE_LS1_EN		BIT(6)
#define TPS65217_ENABLE_LS2_EN		BIT(5)
#define TPS65217_ENABLE_DC1_EN		BIT(4)
#define TPS65217_ENABLE_DC2_EN		BIT(3)
#define TPS65217_ENABLE_DC3_EN		BIT(2)
#define TPS65217_ENABLE_LDO1_EN		BIT(1)
#define TPS65217_ENABLE_LDO2_EN		BIT(0)

#define TPS65217_DEFUVLO_UVLOHYS	BIT(2)
#define TPS65217_DEFUVLO_UVLO_MASK	0x03

#define TPS65217_SEQ1_DC1_SEQ_MASK	0xF0
#define TPS65217_SEQ1_DC2_SEQ_MASK	0x0F

#define TPS65217_SEQ2_DC3_SEQ_MASK	0xF0
#define TPS65217_SEQ2_LDO1_SEQ_MASK	0x0F

#define TPS65217_SEQ3_LDO2_SEQ_MASK	0xF0
#define TPS65217_SEQ3_LDO3_SEQ_MASK	0x0F

#define TPS65217_SEQ4_LDO4_SEQ_MASK	0xF0

#define TPS65217_SEQ5_DLY1_MASK		0xC0
#define TPS65217_SEQ5_DLY2_MASK		0x30
#define TPS65217_SEQ5_DLY3_MASK		0x0C
#define TPS65217_SEQ5_DLY4_MASK		0x03

#define TPS65217_SEQ6_DLY5_MASK		0xC0
#define TPS65217_SEQ6_DLY6_MASK		0x30
#define TPS65217_SEQ6_SEQUP		BIT(2)
#define TPS65217_SEQ6_SEQDWN		BIT(1)
#define TPS65217_SEQ6_INSTDWN		BIT(0)

#define TPS65217_MAX_REGISTER		0x1E
#define TPS65217_PROTECT_NONE		0
#define TPS65217_PROTECT_L1		1
#define TPS65217_PROTECT_L2		2


enum tps65217_regulator_id {
	/* DCDC's */
	TPS65217_DCDC_1,
	TPS65217_DCDC_2,
	TPS65217_DCDC_3,
	/* LDOs */
	TPS65217_LDO_1,
	TPS65217_LDO_2,
	TPS65217_LDO_3,
	TPS65217_LDO_4,
};

#define TPS65217_MAX_REG_ID		TPS65217_LDO_4

/* Number of step-down converters available */
#define TPS65217_NUM_DCDC		3
/* Number of LDO voltage regulators available */
#define TPS65217_NUM_LDO		4
/* Number of total regulators available */
#define TPS65217_NUM_REGULATOR		(TPS65217_NUM_DCDC + TPS65217_NUM_LDO)

enum tps65217_bl_isel {
	TPS65217_BL_ISET1 = 1,
	TPS65217_BL_ISET2,
};

enum tps65217_bl_fdim {
	TPS65217_BL_FDIM_100HZ,
	TPS65217_BL_FDIM_200HZ,
	TPS65217_BL_FDIM_500HZ,
	TPS65217_BL_FDIM_1000HZ,
};

struct tps65217_bl_pdata {
	enum tps65217_bl_isel isel;
	enum tps65217_bl_fdim fdim;
	int dft_brightness;
};

/* Interrupt numbers */
#define TPS65217_IRQ_USB		0
#define TPS65217_IRQ_AC			1
#define TPS65217_IRQ_PB			2
#define TPS65217_NUM_IRQ		3

/**
 * struct tps65217_board - packages regulator init data
 * @tps65217_regulator_data: regulator initialization values
 *
 * Board data may be used to initialize regulator.
 */
struct tps65217_board {
	struct regulator_init_data *tps65217_init_data[TPS65217_NUM_REGULATOR];
	struct device_node *of_node[TPS65217_NUM_REGULATOR];
	struct tps65217_bl_pdata *bl_pdata;
};

/**
 * struct tps65217 - tps65217 sub-driver chip access routines
 *
 * Device data may be used to access the TPS65217 chip
 */

struct tps65217 {
	struct device *dev;
	struct tps65217_board *pdata;
	unsigned long id;
	struct regulator_desc desc[TPS65217_NUM_REGULATOR];
	struct regmap *regmap;
	u8 *strobes;
	struct irq_domain *irq_domain;
	struct mutex irq_lock;
	u8 irq_mask;
	int irq;
};

static inline struct tps65217 *dev_to_tps65217(struct device *dev)
{
	return dev_get_drvdata(dev);
}

static inline unsigned long tps65217_chip_id(struct tps65217 *tps65217)
{
	return tps65217->id;
}

int tps65217_reg_read(struct tps65217 *tps, unsigned int reg,
					unsigned int *val);
int tps65217_reg_write(struct tps65217 *tps, unsigned int reg,
			unsigned int val, unsigned int level);
int tps65217_set_bits(struct tps65217 *tps, unsigned int reg,
		unsigned int mask, unsigned int val, unsigned int level);
int tps65217_clear_bits(struct tps65217 *tps, unsigned int reg,
		unsigned int mask, unsigned int level);

#endif /*  __LINUX_MFD_TPS65217_H */
