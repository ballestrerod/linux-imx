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

#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#include <linux/mfd/core.h>
#include <linux/mfd/enobu-fpga.h>

// #include <linux/fpga/machxo-efb.h>


/**
 * enobu_fpga_reg_read: Read a single eNOBU FPGA register.
 *
 * @enobufpga: Device to read from.
 * @reg: Register to read.
 * @val: Contians the value
 */
int enobu_fpga_reg_read(struct enobu_fpga *enobufpga, unsigned int reg,
			unsigned int *val)
{
	return regmap_read(enobufpga->regmap, reg, val);
}
EXPORT_SYMBOL_GPL(enobu_fpga_reg_read);


/**
 * enobu_fpga_reg_write: Write a single eNOBU FPGA register.
 *
 * @enobufpga: Device to write to.
 * @reg: Register to write to.
 * @val: Value to write.
 */
int enobu_fpga_reg_write(struct enobu_fpga *enobufpga, unsigned int reg,
			unsigned int val)
{
        return regmap_write(enobufpga->regmap, reg, val);
}
EXPORT_SYMBOL_GPL(enobu_fpga_reg_write);


/**
 * enobu_fpga_update_bits: Modify bits w.r.t mask, val and level.
 *
 * @enobufpga: Device to write to.
 * @reg: Register to read-write to.
 * @mask: Mask.
 * @val: Value to write.
 */
static int enobu_fpga_update_bits(struct enobu_fpga *enobufpga, unsigned int reg,
		unsigned int mask, unsigned int val)
{
	int ret;
	unsigned int data;

	ret = enobu_fpga_reg_read(enobufpga, reg, &data);
	if (ret) {
		dev_err(enobufpga->dev, "Read from reg 0x%x failed\n", reg);
		return ret;
	}

	data &= ~mask;
	data |= val & mask;

	ret = enobu_fpga_reg_write(enobufpga, reg, data);
	if (ret)
		dev_err(enobufpga->dev, "Write for reg 0x%x failed\n", reg);

	return ret;
}


int enobu_fpga_set_bits(struct enobu_fpga *enobufpga, unsigned int reg,
		unsigned int mask, unsigned int val)
{
	return enobu_fpga_update_bits(enobufpga, reg, mask, val);
}
EXPORT_SYMBOL_GPL(enobu_fpga_set_bits);


int enobu_fpga_clear_bits(struct enobu_fpga *enobufpga, unsigned int reg,
		unsigned int mask)
{
	return enobu_fpga_update_bits(enobufpga, reg, mask, 0);
}
EXPORT_SYMBOL_GPL(enobu_fpga_clear_bits);


static bool enobu_fpga_readable_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ENOBU_FPGA_REG_TEST1:
	case ENOBU_FPGA_REG_TEST2:
	case ENOBU_FPGA_REG_TEST3:
		return false;
	default:
		return true;
	}
}


static bool enobu_fpga_volatile_register(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case ENOBU_FPGA_REG_TEST4:
		return true;
	default:
		return false;
	}
}

static const struct reg_default enobu_fpga_reg_defaults[] = {
	{ ENOBU_FPGA_REG_M2SLOT1,  0x00 },
	{ ENOBU_FPGA_REG_M2SLOT2,  0x00 },
	{ ENOBU_FPGA_REG_USBHUB,   0x00 },
};


#if 0
static int enobu_fpga_regmap_write(void *context, const void *data, size_t count)
{
	struct device *dev = context;
	struct i2c_client *client = to_i2c_client(dev);
	int ret;

	ret = i2c_master_send(client, data, count);
	if (ret != count)
		return ret < 0 ? ret : -EIO;

	return 0;
}


static int enobu_fpga_regmap_read(void *context, const void *reg, size_t reg_size,
				void *val, size_t val_size)
{
	struct device *dev = context;
	struct i2c_client *client = to_i2c_client(dev);
	int ret;

//        dev_err(&client->dev, "--->>> reg_size=%ld - reg[0]=%x reg[1]=%x\n", 
//                                reg_size, ((u8 *)reg)[0], ((u8 *)reg)[1]);

	if (WARN_ON(reg_size != 2))
		return -EINVAL;

	ret = i2c_master_send(client, reg, reg_size);
	if (ret != reg_size)
		return ret < 0 ? ret : -EIO;

	ret = i2c_master_recv(client, val, val_size);
	if (ret != val_size)
		return ret < 0 ? ret : -EIO;

	return 0;
}

#endif


/*
 * We implement own I2C access routines for regmap in order to get manual access
 * to I2C adapter lock, which is needed for I2C mux adapter.
 */
static int enobu_fpga_regmap_read(void *context, const void *reg_buf,
			       size_t reg_size, void *val_buf, size_t val_size)
{
	struct device *dev = context;
	struct i2c_client *client = to_i2c_client(dev);
	int ret;
	struct i2c_msg msg[2] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = reg_size,
			.buf = (u8 *)reg_buf,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = val_size,
			.buf = val_buf,
		}
	};

        dev_err(&client->dev, "READ <<< reg_size=%ld - reg[0]=%x reg[1]=%x\n", 
                                reg_size, ((u8 *)reg_buf)[0], ((u8 *)reg_buf)[1]);

	ret = __i2c_transfer(client->adapter, msg, 2);
	if (ret != 2) {
		dev_warn(&client->dev, "i2c reg read failed %d\n", ret);
		if (ret >= 0)
			ret = -EREMOTEIO;
		return ret;
	}

	return 0;
}


static int enobu_fpga_regmap_write(void *context, const void *data, size_t count)
{
	struct device *dev = context;
	struct i2c_client *client = to_i2c_client(dev);
	int ret;
	struct i2c_msg msg[1] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = count,
			.buf = (u8 *)data,
		}
	};

        dev_err(&client->dev, "WRITE >>> count=%ld - data=%x\n", 
                                count, ((u8 *)data)[0]);

	ret = __i2c_transfer(client->adapter, msg, 1);
	if (ret != 1) {
		dev_warn(&client->dev, "i2c reg write failed %d\n", ret);
		if (ret >= 0)
			ret = -EREMOTEIO;
		return ret;
	}

	return 0;
}



static const struct regmap_bus enobu_fpga_regmap_bus = {
	.read = enobu_fpga_regmap_read,
	.write = enobu_fpga_regmap_write,
};


const struct regmap_config enobu_fpga_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,

	.cache_type = REGCACHE_NONE,

	// .max_register = ENOBU_FPGA_REG_MAX,
	// .volatile_reg = enobu_fpga_volatile_register,

	//.reg_defaults = enobu_fpga_reg_defaults,
	//.num_reg_defaults = ARRAY_SIZE(enobu_fpga_reg_defaults),
	//.readable_reg = enobu_fpga_readable_register,
};
EXPORT_SYMBOL_GPL(enobu_fpga_regmap_config);


static const struct mfd_cell enobu_fpga_devs[] = {
	{
		.name = "enobu-fpga-gpioreg",
		.of_compatible = "leonardo,enobu-fpga-gpioreg"
	}, {
		.name = "enobu-fpga-ledmatrix",
		.of_compatible = "leonardo,enobu-fpga-ledmatrix"
	}, {
		.name = "enobu-fpga-leds",
		.of_compatible = "leonardo,enobu-fpga-leds"
	},
};



static void digin_irq_work_func(struct enobu_fpga *enobufpga)
{
        //TODO send kobject_uevent_env signals

        return;
}


static irqreturn_t enobufpga_irq(int irq, void *data)
{
        int ret = IRQ_NONE;

#ifdef ENOBU_FPGA_IRQ_HANDLER        
	struct enobu_fpga *enobufpga = data;
	u8 irqreg;

	/* Leggo il registro degli INTERRUPT2 per capire la fonte */
	if ((irqreg = efb_spi_read(ENOBU_FPGA_IRQ2)) < 0) {
		dev_err(enobufpga->dev, "cannot read from IRQ2 register\n");
		irqreg = 0;
	}

	/* save current digin state, inside IRQ procedure */
	if ((enobufpga->digin_status_reg = efb_spi_read(ENOBU_FPGA_DINP)) < 0) {
		dev_err(enobufpga->dev, "cannot read from DINP register\n");
	}

	/* save current keymng state, inside IRQ procedure */
	if ((enobufpga->keymng_reg = efb_spi_read(ENOBU_FPGA_KEYMNG)) < 0) {
		dev_err(enobufpga->dev, "cannot read from KEYMNG register\n");
	}

	/* resettare l'IRQ nel registro 0x19 */
	if (efb_spi_write(ENOBU_FPGA_IRQ2, 0) < 0) {
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


static ssize_t show_fpga_irq_test(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct enobu_fpga *enobufpga = dev_get_drvdata(dev);
        unsigned int irq2;
        int err;
        
        //efb_spi_read(ENOBU_FPGA_IRQ2, &irq2);
        err = enobu_fpga_reg_read(enobufpga, ENOBU_FPGA_REG_IRQ2, &irq2);
 	if (err < 0) {
		dev_err(dev, "Failed to read IRQ Test reg: %d\n", err);
		return err;
	}

       	return sprintf(buf, "%x\n", irq2);
}


static ssize_t store_fpga_irq_test(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct enobu_fpga *enobufpga = dev_get_drvdata(dev);
        //struct regmap *regmap = dev_get_drvdata(dev);
        unsigned int val;
        int err;

        err = kstrtouint(buf, 10, &val);
	if (err)
                return err;

        // efb_spi_write(ENOBU_FPGA_IRQ2, 0x80);
	err = enobu_fpga_reg_write(enobufpga, ENOBU_FPGA_REG_IRQ2, 0x80);
	if (err)
		return err;

	return n;
}

static DEVICE_ATTR(fpga_irq_test, 0644, show_fpga_irq_test, store_fpga_irq_test);





static int enobu_fpga_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	// struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
        unsigned int ver, rev, hwver;
	int ret;
//NEXT	unsigned int id;
	struct enobu_fpga *enobufpga;

	enobufpga = devm_kzalloc(&client->dev, sizeof(*enobufpga), GFP_KERNEL);
	if (!enobufpga)
		return -ENOMEM;

	// enobufpga->irq = platform_get_irq(pdev, 0);
	// if (enobufpga->irq < 0) {
	// 	dev_err(&pdev->dev, "enobufpga irq number not available\n");
	// 	return -EINVAL;
	// }
        
	i2c_set_clientdata(client, enobufpga);
        enobufpga->irq = client->irq;
	enobufpga->dev = &client->dev;

	ret = devm_request_threaded_irq(&client->dev, enobufpga->irq, NULL,
					enobufpga_irq,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					/* dev_name(&pdev->dev) */ "enobu-fpga", enobufpga);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to request irq: %d\n", ret);
		return ret;
	}

	enobufpga->regmap = devm_regmap_init_i2c(client, &enobu_fpga_regmap_config);
	//enobufpga->regmap = devm_regmap_init(&client->dev,
        //                                     &enobu_fpga_regmap_bus,
        //                                     &client->dev, &enobu_fpga_regmap_config);
	if (IS_ERR(enobufpga->regmap)) {
		dev_err(enobufpga->dev, "Failed to initialize register map\n");
		return PTR_ERR(enobufpga->regmap);
	}

//NEXT	enobufpga->regmap = dev_get_regmap(pdev->dev.parent, NULL);
//NEXT	if (!enobufpga->regmap)
//NEXT		return -ENODEV;

	// platform_set_drvdata(pdev, enobufpga);

        // efb_spi_read(ENOBU_FPGA_VER, &ver); 
        // efb_spi_read(ENOBU_FPGA_REV, &rev);
        // efb_spi_read(ENOBU_FPGA_HWVER, &hwver);

        ret = enobu_fpga_reg_read(enobufpga, ENOBU_FPGA_REG_VER, &ver);
 	if (ret < 0) {
		dev_err(enobufpga->dev, "Failed to read chip ver: %d\n", ret);
		goto out;
	}

        ret = enobu_fpga_reg_read(enobufpga, ENOBU_FPGA_REG_REV, &rev);
 	if (ret < 0) {
		dev_err(enobufpga->dev, "Failed to read chip rev: %d\n", ret);
		goto out;
	}

        ret = enobu_fpga_reg_read(enobufpga, ENOBU_FPGA_REG_HWVER, &hwver);
 	if (ret < 0) {
		dev_err(enobufpga->dev, "Failed to read chip hwver: %d\n", ret);
		goto out;
	}

        dev_info(&client->dev, "eNOBU-FPGA: ver %d.%d [hw ver %d]\n", ver, rev, hwver);

	/* IRQ test trigger file and init */
        //WAS efb_spi_write(ENOBU_FPGA_IRQ2_MASK, 0x7F);
	ret = enobu_fpga_reg_write(enobufpga, ENOBU_FPGA_REG_IRQ2MASK, 0x7f);
	if (ret)
		goto out;

	ret = device_create_file(enobufpga->dev, &dev_attr_fpga_irq_test);
	if (ret < 0)
		goto out;

        ret = devm_mfd_add_devices(&client->dev, -1, enobu_fpga_devs,
                                   ARRAY_SIZE(enobu_fpga_devs), NULL,
                                   0, NULL);
	if (ret) {
		dev_err(&client->dev, "failed to add child devices: %d\n", ret);
	}

out:
	return ret;
}

static int enobu_fpga_remove(struct i2c_client *client)
{
	return 0;
}


static const struct of_device_id enobu_fpga_of_match[] = {
	{ .compatible = "leonardo,enobu-fpga" },
	{ }
};
MODULE_DEVICE_TABLE(of, enobu_fpga_of_match);

static const struct i2c_device_id enobu_fpga_id[] = {
	{ "enobu-fpga-core", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, enobu_fpga_id);

static struct i2c_driver enobu_fpga_driver = {
	.probe = enobu_fpga_probe,
	.remove	= enobu_fpga_remove,
	.id_table = enobu_fpga_id,
	.driver = {
		.name = "enobu-fpga",
		.of_match_table = enobu_fpga_of_match,
	},
};

module_i2c_driver(enobu_fpga_driver);

MODULE_AUTHOR("Davide Ballestrero, Leonardo");
MODULE_DESCRIPTION("Driver for eNOBU FPGA");
MODULE_LICENSE("GPL");
