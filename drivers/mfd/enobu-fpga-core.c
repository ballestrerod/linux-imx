/*
 * Copyright (c) 2020 Leonardo SpA.
 * Author: Davide Ballestrero, Leonardo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/module.h>
// #include <linux/of_device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
// #include <linux/regmap.h>
#include <linux/mfd/core.h>
// #include <linux/mfd/enobu-fpga/registers.h>
#include <linux/fpga/machxo-efb.h>

#define ENOBU_HPS2FPGA_M2_SLOT1			0x000
#define ENOBU_HPS2FPGA_M2_SLOT2			0x001
#define ENOBU_HPS2FPGA_USB_HUB			0x002

#define ENOBU_HPS2FPGA_IRQ2			0x019
#define ENOBU_HPS2FPGA_DINP			0x01a
#define ENOBU_HPS2FPGA_KEYMNG			0x020

#define ENOBU_HPS2FPGA_TEST1                    0x100
#define ENOBU_HPS2FPGA_TEST2                    0x101
#define ENOBU_HPS2FPGA_TEST3                    0x102

#define ENOBU_HPS2FPGA_TEST4                    0x200

#define ENOBU_HPS2FPGA_HWVER			0x7FD
#define ENOBU_HPS2FPGA_VER			0x7FE
#define ENOBU_HPS2FPGA_REV			0x7FF

#define ENOBU_HPS2FPGA_REGISTER_COUNT		0x800
#define ENOBU_HPS2FPGA_MAX_REGISTER		0x7FF


#if 0
static bool enobu_hps2fpga_readable_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ENOBU_HPS2FPGA_TEST1:
	case ENOBU_HPS2FPGA_TEST2:
	case ENOBU_HPS2FPGA_TEST3:
		return false;
	default:
		return true;
	}
}


static bool enobu_hps2fpga_volatile_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ENOBU_HPS2FPGA_TEST4:
		return true;
	default:
		return false;
	}
}

static const struct reg_default enobu_hps2fpga_reg_defaults[] = {
	{ ENOBU_HPS2FPGA_M2_SLOT1,  0x00 },
	{ ENOBU_HPS2FPGA_M2_SLOT2,  0x00 },
	{ ENOBU_HPS2FPGA_USB_HUB,   0x00 },
};

const struct regmap_config enobu_hps2fpga_regmap_config = {
	.reg_bits = 11,
	.val_bits = 8,

	.max_register = ENOBU_HPS2FPGA_MAX_REGISTER,
	.volatile_reg = enobu_hps2fpga_volatile_register,

	.reg_defaults = enobu_hps2fpga_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(enobu_hps2fpga_reg_defaults),
	.readable_reg = enobu_hps2fpga_readable_register,

	.cache_type = REGCACHE_RBTREE,
};
EXPORT_SYMBOL_GPL(enobu_hps2fpga_regmap_config);
#endif


struct enobu_fpga_chip {
	struct device *dev;
//	struct regmap *regmap;

	int irq;

	u8 int_val;
	u8 digin_status_reg;
	u8 keymng_reg;
};

static const struct mfd_cell enobu_fpga_devs[] = {
	{
		.name = "enobu-fpga-gpioreg",
		.of_compatible = "leonardo,enobu-fpga-gpioreg"
	}, {
		.name = "enobu-fpga-ledmatrix",
		.of_compatible = "leonardo,enobu-fpga-ledmatrix"
	},
};



static void digin_irq_work_func(struct enobu_fpga_chip *enobufpga)
{
        //TODO send kobject_uevent_env signals

        return;
}


static irqreturn_t enobufpga_irq(int irq, void *data)
{
        int ret = IRQ_NONE;

#ifdef ENOBU_FPGA_IRQ_HANDLER        
	struct enobu_fpga_chip *enobufpga = data;
	int irqreg;

	/* Leggo il registro degli INTERRUPT2 per capire la fonte */
	if ((irqreg = efb_spi_read(ENOBU_HPS2FPGA_IRQ2)) < 0) {
		dev_err(enobufpga->dev, "cannot read from IRQ2 register\n");
		irqreg = 0;
	}

	/* save current digin state, inside IRQ procedure */
	if ((enobufpga->digin_status_reg = efb_spi_read(ENOBU_HPS2FPGA_DINP)) < 0) {
		dev_err(enobufpga->dev, "cannot read from DINP register\n");
	}

	/* save current keymng state, inside IRQ procedure */
	if ((enobufpga->keymng_reg = efb_spi_read(ENOBU_HPS2FPGA_KEYMNG)) < 0) {
		dev_err(enobufpga->dev, "cannot read from KEYMNG register\n");
	}

	/* resettare l'IRQ nel registro 0x19 */
	if (efb_spi_write(ENOBU_HPS2FPGA_IRQ2, 0) < 0) {
		dev_err(enobufpga->dev, "cannot write to IRQ2 register\n");
	}

	enobufpga->int_val = irqreg;

	if (enobufpga->int_val) {
		digin_irq_work_func(enobufpga);
		ret = IRQ_HANDLED;
	}
#else
        ret = IRQ_HANDLED;
#endif

	return ret;
}



static int enobu_fpga_probe(struct platform_device *pdev)
{
        u8 ver, rev, hwver;
	int ret;
//NEXT	unsigned int id;
	struct enobu_fpga_chip *enobufpga;

	enobufpga = devm_kzalloc(&pdev->dev, sizeof(*enobufpga), GFP_KERNEL);
	if (!enobufpga)
		return -ENOMEM;

	enobufpga->irq = platform_get_irq(pdev, 0);
	if (enobufpga->irq < 0) {
		dev_err(&pdev->dev, "enobufpga irq number not available\n");
		return -EINVAL;
	}

	enobufpga->dev = &pdev->dev;

	ret = devm_request_threaded_irq(&pdev->dev, enobufpga->irq, NULL,
					enobufpga_irq,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					/* dev_name(&pdev->dev) */ "enobu-fpga", enobufpga);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to request irq: %d\n", ret);
		return ret;
	}

	/*
	 * mt6397 MFD is child device of soc pmic wrapper.
	 * Regmap is set from its parent.
	 */
//NEXT	enobufpga->regmap = dev_get_regmap(pdev->dev.parent, NULL);
//NEXT	if (!enobufpga->regmap)
//NEXT		return -ENODEV;

	platform_set_drvdata(pdev, enobufpga);

//NEXT  ret = regmap_read(enobufpga->regmap, MT6397_CID, &id);
//NEXT 	if (ret) {
//NEXT		dev_err(enobufpga->dev, "Failed to read chip id: %d\n", ret);
//NEXT		return ret;
//NEXT	}

        efb_spi_read(ENOBU_HPS2FPGA_VER, &ver); 
        efb_spi_read(ENOBU_HPS2FPGA_REV, &rev);
        efb_spi_read(ENOBU_HPS2FPGA_HWVER, &hwver);

        dev_err(&pdev->dev, "eNOBU-FPGA: ver %d.%d [hw ver %d]\n", ver, rev, hwver);

        ret = devm_mfd_add_devices(&pdev->dev, -1, enobu_fpga_devs,
                                   ARRAY_SIZE(enobu_fpga_devs), NULL,
                                   0, NULL);
	if (ret) {
		dev_err(&pdev->dev, "failed to add child devices: %d\n", ret);
	}

	return ret;
}

static const struct of_device_id enobu_fpga_of_match[] = {
	{ .compatible = "leonardo,enobu-fpga-1.5" },
	{ }
};
MODULE_DEVICE_TABLE(of, enobu_fpga_of_match);

static struct platform_driver enobu_fpga_driver = {
	.probe = enobu_fpga_probe,
	.driver = {
		.name = "enobu-fpga-1.5",
		.of_match_table = enobu_fpga_of_match,
	},
};

module_platform_driver(enobu_fpga_driver);

MODULE_AUTHOR("Davide Ballestrero, Leonardo");
MODULE_DESCRIPTION("Driver for eNOBU FPGA");
MODULE_LICENSE("GPL");
