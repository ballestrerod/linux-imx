/*
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether expressed or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * Based on the TPS65218 driver and the previous TPS2388 driver by
 * Margarita Olaya Cabrera <magi@slimlogic.co.uk>
 */

#ifndef __LINUX_TPS2388_POE_H
#define __LINUX_TPS2388_POE_H

#define POE_ATTR(_name, _mode, _show, _store, _index)    \
		{ .dev_attr = __ATTR(_name, _mode, _show, _store),  \
		.index = _index }

/* 8 ports total but only 4 for each I2C address */
#define TPS2388_PORT_MAX		4

#define TPS2388_MODE_OFF		0
#define TPS2388_MODE_MANUAL		1
#define TPS2388_MODE_SEMIAUTO	2
#define TPS2388_MODE_AUTO		3

#define TPS2388_DEFAULT_OP_MODE	TPS2388_MODE_OFF
#define TPS2388_DEFAULT_DETECT_CLASS	0
#define TPS2388_DEFAULT_LED_POS			0

/* List of registers for TPS2388 */

/* Interrupts */
#define TPS2388_INTERRUPT		0x00
#define TPS2388_INTERRUPT_MASK	0x01
/* Event */
#define TPS2388_POWER_EV		0x02
#define TPS2388_POWER_EV_CLEAR	0x03
#define TPS2388_DETECT_EV		0x04
#define TPS2388_DETECT_EV_CLEAR	0x05
#define TPS2388_FAULT_EV		0x06
#define TPS2388_FAULT_EV_CLEAR	0x07
#define TPS2388_ILIM_EV			0x08
#define TPS2388_ILIM_EV_CLEAR	0x09
#define TPS2388_SUPPLY_EV		0x0A
#define TPS2388_SUPPLY_EV_CLEAR	0x0B
/* Status */
#define TPS2388_PORT_1_STATUS	0x0C
#define TPS2388_PORT_2_STATUS	0x0D
#define TPS2388_PORT_3_STATUS	0x0E
#define TPS2388_PORT_4_STATUS	0x0F
#define TPS2388_POWER_STATUS	0x10
#define TPS2388_PIN_STATUS		0x11
/* Configuration */
#define TPS2388_OP_MODE			0x12
#define TPS2388_DISCONNECT_EN	0x13
#define TPS2388_DETECT_CLASS_EN	0x14
#define TPS2388_POWER_PRIORITY	0x15
#define TPS2388_TIMING_CONFIG	0x16
#define TPS2388_GENERAL_MASK	0x17
/* Push buttons */
#define TPS2388_DETECTCLASS_RES	0x18
#define TPS2388_POWER_EN		0x19
#define TPS2388_RESET			0x1A
/* General/specialized */
#define TPS2388_ID				0x1B
#define TPS2388_RESERV1			0x1C	/* Unused */
#define TPS2388_RESERV2			0x1D	/* Unused */
#define TPS2388_POLICE_21_CONF	0x1E
#define TPS2388_POLICE_43_CONF	0x1F
#define TPS2388_RESERV3			0x20	/* Unused */
#define TPS2388_RESERV4			0x21	/* Unused */
#define TPS2388_RESERV5			0x22	/* Unused */
#define TPS2388_IEEE_POWER_EN	0x23
#define TPS2388_PWRON_FAULT		0x24
#define TPS2388_PWRON_FAULT_CLR	0x25
#define TPS2388_PORT_REMAPPING	0x26
#define TPS2388_PORT_21_PRIO	0x27
#define TPS2388_PORT_43_PRIO	0x28
#define TPS2388_RESERV6			0x29
#define TPS2388_RESERV7			0x2A
#define TPS2388_RESERV8			0x2B
#define TPS2388_TEMPERATURE		0x2C
#define TPS2388_RESERV9			0x2D	/* Unused */
#define TPS2388_INPUT_VOLTAGE	0x2E
#define TPS2388_RESERV10		0x2F	/* Unused */
/* Extended register set - Port parametric measurement */
#define TPS2388_PORT_1_CURRENT	0x30
#define TPS2388_RESERV11		0x31	/* Unused */
#define TPS2388_PORT_1_VOLTAGE	0x32
#define TPS2388_RESERV13		0x33	/* Unused */
#define TPS2388_PORT_2_CURRENT	0x34
#define TPS2388_RESERV14		0x35	/* Unused */
#define TPS2388_PORT_2_VOLTAGE	0x36
#define TPS2388_RESERV16		0x37	/* Unused */
#define TPS2388_PORT_3_CURRENT	0x38
#define TPS2388_RESERV17		0x39	/* Unused */
#define TPS2388_PORT_3_VOLTAGE	0x3A
#define TPS2388_RESERV19		0x3B	/* Unused */
#define TPS2388_PORT_4_CURRENT	0x3C
#define TPS2388_RESERV20		0x3D	/* Unused */
#define TPS2388_PORT_4_VOLTAGE	0x3E
#define TPS2388_RESERV21		0x3F	/* Unused */
/* Configuration/Others */
#define TPS2388_POE_PLUS		0x40
#define TPS2388_FIRMWARE_REV	0x41
#define TPS2388_I2C_WATCHDOG	0x42
#define TPS2388_DEVICE_ID		0x43
/* Port signature measurements */
#define TPS2388_PORT_1_DET_RES	0x44
#define TPS2388_PORT_2_DET_RES	0x45
#define TPS2388_PORT_3_DET_RES	0x46
#define TPS2388_PORT_4_DET_RES	0x47

#define TPS2388_MAX_REGISTER	0x47

/* Interrupt Register field definitions */
#define TPS2388_INT_PEC			BIT(0)
#define TPS2388_INT_PGC			BIT(1)
#define TPS2388_INT_DISF		BIT(2)
#define TPS2388_INT_DETC		BIT(3)
#define TPS2388_INT_CLASC		BIT(4)
#define TPS2388_INT_IFAULT		BIT(5)
#define TPS2388_INT_STRTF		BIT(6)
#define TPS2388_INT_SUPF		BIT(7)

/* Interrupt mask field definitions */
#define TPS2388_INT_PEMSK		BIT(0)
#define TPS2388_INT_PGMSK		BIT(1)
#define TPS2388_INT_DIMSK		BIT(2)
#define TPS2388_INT_DEMSK		BIT(3)
#define TPS2388_INT_CLMSK		BIT(4)
#define TPS2388_INT_IFMSK		BIT(5)
#define TPS2388_INT_STMSK		BIT(6)
#define TPS2388_INT_SUMSK		BIT(7)

/* Power event (0x02 0x03) Register field definitions */
#define PGC4_MASK		0x80
#define PGC4_SHIFT		7
#define PGC3_MASK		0x40
#define PGC3_SHIFT		6
#define PGC2_MASK		0x20
#define PGC2_SHIFT		5
#define PGC1_MASK		0x10
#define PGC1_SHIFT		4
#define PEC4_MASK		0x08
#define PEC4_SHIFT		3
#define PEC3_MASK		0x04
#define PEC3_SHIFT		2
#define PEC2_MASK		0x02
#define PEC2_SHIFT		1
#define PEC1_MASK		0x01
#define PEC1_SHIFT		0

/* Detection event (0x4 0x05) Register field definitions */
#define CLSC4_MASK		0x80
#define CLSC4_SHIFT		7
#define CLSC3_MASK		0x40
#define CLSC3_SHIFT		6
#define CLSC2_MASK		0x20
#define CLSC2_SHIFT		5
#define CLSC1_MASK		0x10
#define CLSC1_SHIFT		4
#define DECT4_MASK		0x08
#define DECT4_SHIFT		3
#define DECT3_MASK		0x04
#define DECT3_SHIFT		2
#define DECT2_MASK		0x02
#define DECT2_SHIFT		1
#define DECT1_MASK		0x01
#define DECT1_SHIFT		0

/* Fault event (0x6 0x07) Register field definitions */
#define DISF4_MASK		0x80
#define DISF4_SHIFT		7
#define DISF3_MASK		0x40
#define DISF3_SHIFT		6
#define DISF2_MASK		0x20
#define DISF2_SHIFT		5
#define DISF1_MASK		0x10
#define DISF1_SHIFT		4
#define ICUT4_MASK		0x08
#define ICUT4_SHIFT		3
#define ICUT3_MASK		0x04
#define ICUT3_SHIFT		2
#define ICUT2_MASK		0x02
#define ICUT2_SHIFT		1
#define ICUT1_MASK		0x01
#define ICUT1_SHIFT		0

/* Pin Status (0x11) Register field definitions */
#define TPS2388_PSTATUS_SLA0		BIT(2)

/* General mask (0x17) Register field definitions */
#define TPS2388_GMASK_NBITACC		BIT(5)
#define TPS2388_GMASK_INTEN			BIT(7)

/* PoE Plus (0x40) */
#define TPS2388_POE_PLUS_TPON		BIT(0)

/* Define the TPS2388 IRQ numbers */
enum tps2388_irqs {
	/* INT_STS registers */
	TPS2388_IRQ_PWRHOLD_F,
	TPS2388_IRQ_VMON,
	TPS2388_IRQ_PWRON,
	TPS2388_IRQ_PWRON_LP,
	TPS2388_IRQ_PWRHOLD_R,
	TPS2388_IRQ_HOTDIE,
	TPS2388_IRQ_GPIO1_R,
	TPS2388_IRQ_GPIO1_F,
	/* INT_STS2 registers */
	TPS2388_IRQ_GPIO2_R,
	TPS2388_IRQ_GPIO2_F,
	TPS2388_IRQ_GPIO3_R,
	TPS2388_IRQ_GPIO3_F,
	TPS2388_IRQ_GPIO4_R,
	TPS2388_IRQ_GPIO4_F,
	TPS2388_IRQ_GPIO5_R,
	TPS2388_IRQ_GPIO5_F,
	/* INT_STS3 registers */
	TPS2388_IRQ_PGOOD_DCDC1,
	TPS2388_IRQ_PGOOD_DCDC2,
	TPS2388_IRQ_PGOOD_DCDC3,
	TPS2388_IRQ_PGOOD_DCDC4,
	TPS2388_IRQ_PGOOD_LDO1,
	TPS2388_IRQ_PGOOD_LDO2,
	TPS2388_IRQ_PGOOD_LDO3,
	TPS2388_IRQ_PGOOD_LDO4,
	/* INT_STS4 registers */
	TPS2388_IRQ_PGOOD_LDO5,
	TPS2388_IRQ_PGOOD_LDO6,
	TPS2388_IRQ_PGOOD_LDO7,
	TPS2388_IRQ_PGOOD_LDO8,
	TPS2388_IRQ_PGOOD_LDO9,
	TPS2388_IRQ_PGOOD_LDO10,
};

struct tps2388_port_data {
	bool enabled;
	unsigned int op_mode;
	bool det_class_en;
};

/*
 * struct tps2388 - state holder for the tps2388 driver
 *
 * Device data may be used to access the TPS2388 chip
 */
struct tps2388 {
	struct device		*dev;
	struct regmap		*regmap;
	struct i2c_client	*client;
	struct mutex		update_lock; /* mutex protect updates */

	struct timer_list   poll_timer;
	//struct delayed_work dis_connect;	/* Connect/disconnect check */
	struct work_struct  dis_connect;	/* Connect/disconnect check */

	struct led_trigger	trig[TPS2388_PORT_MAX];		/* POE led trigger */

	struct tps2388_port_data port_data[TPS2388_PORT_MAX];

	int configAB;	/* Configuration type (default A) */
	int ieee_mode;	/* IEEE mode on */
	int dev_index;	/* In Configuration A, device acts as two logical devices with 4 ports each */

	int last_read;	/* Needed to remember last read on detect/class event register */

	/* IRQ Data */
	//int irq;
	//struct regmap_irq_chip_data *irq_data;
};

extern const struct regmap_config tps2388_regmap_config;

#endif /*  __LINUX_TPS2388_POE_H */
