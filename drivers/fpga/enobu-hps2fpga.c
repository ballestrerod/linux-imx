/*
 * Copyright (c) 2017, National Instruments Corp.
 * Copyright (c) 2017, Xilix Inc
 *
 * FPGA Bridge Driver for the Xilinx LogiCORE Partial Reconfiguration
 * Decoupler IP Core.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/regmap.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/fpga/fpga-bridge.h>

#include "machxo-efb.h"


struct enobu_hps2fpga_data {
        struct spi_device *spifpga;
};


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


static int enobu_hps2fpga_enable_set(struct fpga_bridge *bridge, bool enable)
{
	struct enobu_hps2fpga_data *priv = bridge->priv;
	// int err;

#if 0
	err = clk_enable(priv->clk);
	if (err)
		return err;
#endif
        
        dev_err(&bridge->dev, "Bridge ENABLE\n");

	// clk_disable(priv->clk);

	return 0;
}

static int enobu_hps2fpga_enable_show(struct fpga_bridge *bridge)
{
	const struct enobu_hps2fpga_data *priv = bridge->priv;
	u32 status = 1;
        //int err;

        dev_err(&bridge->dev, "Bridge SHOW\n");

/*
	err = clk_enable(priv->clk);
	if (err)
		return err;

	status = readl(priv->io_base);

	clk_disable(priv->clk);
*/

	return !status;
}

static struct fpga_bridge_ops enobu_hps2fpga_br_ops = {
	.enable_set = enobu_hps2fpga_enable_set,
	.enable_show = enobu_hps2fpga_enable_show,
};

static const struct of_device_id enobu_hps2fpga_of_match[] = {
	{ .compatible = "lnrd,enobu-hps2fpga", },
	{},
};
MODULE_DEVICE_TABLE(of, enobu_hps2fpga_of_match);




#if 0

#define MY_BUS_NUM 3
static struct spi_device *spi_device;
 
static int __init enobu_hps2fpga_spi_init(struct enobu_hps2fpga_data *priv)
{
    struct spi_master *master;
    int ret;
     
    //Register information about your slave device:
    struct spi_board_info spi_device_info = {
        .modalias = "enobu-hps2fpga-spi",
        .max_speed_hz = 100000, //speed your device (slave) can handle
        .bus_num = MY_BUS_NUM,
        .chip_select = 0,
        .mode = 0,
    };
     
    /* To send data we have to know what spi port/pins should be used. This information 
      can be found in the device-tree. */
    master = spi_busnum_to_master( spi_device_info.bus_num );
    if( !master ){
        printk("MASTER not found.\n");
            return -ENODEV;
    }
     
    // create a new slave device, given the master and device info
    spi_device = spi_new_device( master, &spi_device_info );
 
    if( !spi_device ) {
        printk("FAILED to create slave.\n");
        return -ENODEV;
    }
     
    priv->spifpga = spi_device;

    spi_device->bits_per_word = 8;
 
    ret = spi_setup( spi_device );
     
    if( ret ){
        printk("FAILED to setup slave.\n");
        spi_unregister_device( spi_device );
        return -ENODEV;
    }

    return 0;
}

#endif


static int enobu_hps2fpga_probe(struct platform_device *pdev)
{
	struct enobu_hps2fpga_data *priv;
	struct device_node *spifpga_np = NULL;
	struct platform_device *spifpga_pdev;
	struct fpga_bridge *br;
	int err;
	struct regmap *regmap;
//	struct resource *res;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	spifpga_np = of_parse_phandle(pdev->dev.of_node, "spi-ctrl", 0);
	if (!spifpga_np) {
		dev_err(&pdev->dev, "spi control interface phandle missing or invalid\n");
		err = -EINVAL;
                goto fail;
	}

	spifpga_pdev = of_find_device_by_node(spifpga_np);
	if (!spifpga_pdev) {
		dev_err(&pdev->dev, "failed to find SPI platform device\n");
		err = -EINVAL;
		goto fail;
	}

        dev_err(&pdev->dev, "Platform device name: %s\n", spifpga_pdev->name);


//DEBUG        enobu_hps2fpga_spi_init(priv);


        priv->spifpga = to_spi_device(&spifpga_pdev->dev);
// 
// 	priv->spifpga->bits_per_word = 8;
// 	
//        err = spi_setup(priv->spifpga);
// 	if (err < 0)
// 		return err;
//
//	regmap = devm_regmap_init_spi(priv->spifpga, 
//                                     &enobu_hps2fpga_regmap_config);
//	if (IS_ERR(regmap))
//		return PTR_ERR(regmap);

        /*
	priv->clk = devm_clk_get(&pdev->dev, "aclk");
	if (IS_ERR(priv->clk)) {
		if (PTR_ERR(priv->clk) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "input clock not found\n");
		return PTR_ERR(priv->clk);
	}

	err = clk_prepare_enable(priv->clk);
	if (err) {
		dev_err(&pdev->dev, "unable to enable clock\n");
		return err;
	}

	clk_disable(priv->clk);
        */

	br = devm_fpga_bridge_create(&pdev->dev, "eNOBU HPS2FPGA Bridge",
				   &enobu_hps2fpga_br_ops, priv);
	if (!br) {
		err = -ENOMEM;
		goto fail;
	}

	platform_set_drvdata(pdev, br);

//TEMP  br->regmap = regmap;

	err = fpga_bridge_register(br);
	if (err)
	        dev_err(&pdev->dev, "unable to register eNOBU HPS2FPGA Bridge");

        dev_err(&pdev->dev, "FPGA: ver %d.%d [hw ver %d]\n", 
                        efb_spi_read(ENOBU_HPS2FPGA_VER), 
                        efb_spi_read(ENOBU_HPS2FPGA_REV), 
                        efb_spi_read(ENOBU_HPS2FPGA_HWVER));

fail:
	// clk_unprepare(priv->clk);

	return err;
}

static int enobu_hps2fpga_remove(struct platform_device *pdev)
{
	struct fpga_bridge *bridge = platform_get_drvdata(pdev);
//	struct enobu_hps2fpga_data *p = bridge->priv;

	fpga_bridge_unregister(bridge);

	// clk_unprepare(p->clk);

	return 0;
}

static struct platform_driver enobu_hps2fpga_driver = {
	.probe = enobu_hps2fpga_probe,
	.remove = enobu_hps2fpga_remove,
	.driver = {
		.name = "enobu_hps2fpga",
		.of_match_table = of_match_ptr(enobu_hps2fpga_of_match),
	},
};

module_platform_driver(enobu_hps2fpga_driver);

MODULE_DESCRIPTION("Xilinx Partial Reconfiguration Decoupler");
MODULE_AUTHOR("Moritz Fischer <mdf@kernel.org>");
MODULE_AUTHOR("Michal Simek <michal.simek@xilinx.com>");
MODULE_LICENSE("GPL v2");
