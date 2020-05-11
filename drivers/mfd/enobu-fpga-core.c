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

#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/mfd/core.h>
// #include <linux/mfd/enobu-fpga/registers.h>
#include <linux/fpga/machxo-efb.h>

#define ENOBU_HPS2FPGA_M2_SLOT1			0x000
#define ENOBU_HPS2FPGA_M2_SLOT2			0x001
#define ENOBU_HPS2FPGA_USB_HUB			0x002

#define ENOBU_HPS2FPGA_TEST1                    0x100
#define ENOBU_HPS2FPGA_TEST2                    0x101
#define ENOBU_HPS2FPGA_TEST3                    0x102

#define ENOBU_HPS2FPGA_TEST4                    0x200

#define ENOBU_HPS2FPGA_HWVER			0x7FD
#define ENOBU_HPS2FPGA_VER			0x7FE
#define ENOBU_HPS2FPGA_REV			0x7FF

#define ENOBU_HPS2FPGA_REGISTER_COUNT		0x800
#define ENOBU_HPS2FPGA_MAX_REGISTER		0x7FF


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



struct enobu_fpga_chip {
	struct device *dev;
	struct regmap *regmap;
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

static int enobu_fpga_probe(struct platform_device *pdev)
{
	int ret;
//NEXT	unsigned int id;
	struct enobu_fpga_chip *enobufpga;

	enobufpga = devm_kzalloc(&pdev->dev, sizeof(*enobufpga), GFP_KERNEL);
	if (!enobufpga)
		return -ENOMEM;

	enobufpga->dev = &pdev->dev;

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
        
        dev_err(&pdev->dev, "eNOBU-FPGA: ver %d.%d [hw ver %d]\n", 
                        efb_spi_read(ENOBU_HPS2FPGA_VER), 
                        efb_spi_read(ENOBU_HPS2FPGA_REV), 
                        efb_spi_read(ENOBU_HPS2FPGA_HWVER));

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
