// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for the Microchip USB5744 4-port hub.
 *
 * Copyright (C) 2018 Google, Inc.
 */

#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>


struct pi7c9x2g404 {
	struct gpio_desc *reset_gpio;
};



int pericom_write_word_data(const struct i2c_client *client, u8 port, 
                                u8 reg, u16 value)
{
        int val;

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | reg));
	if (val < 0) {
		dev_err(&client->dev, "failed to write reg %x\n", reg);
                return val;
        }

        val = i2c_smbus_write_word_data(client, 0x08, value);
	if (val < 0) {
		dev_err(&client->dev, "failed to write reg %x\n", reg);
                return val;
        }

        return val;
}



int pericom_read_word_data(const struct i2c_client *client, u8 port,
                                u16 reg)
{
	int val;

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | reg));
        if (val < 0) {
		dev_err(&client->dev, "failed to write reg %x\n", reg);
                return val;
        }

        val = i2c_smbus_read_word_data(client, 0x08);
        if (val < 0) {
		dev_err(&client->dev, "failed to read register %x\n", reg);
                return val;
        }

        return val;
}






static int pi7c9x2g404_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct pi7c9x2g404 *data =
		devm_kzalloc(dev, sizeof(struct pi7c9x2g404), GFP_KERNEL);
	// u8 val[I2C_SMBUS_BLOCK_MAX] = {0};
	u8 port = 0x00;
	u16 val;
        int i, ret;
	
        i2c_set_clientdata(client, data);

        /* VendorID and ProductID */
        ret = pericom_read_word_data(client, port, 0x00);
	if (ret >= 0)
                dev_info(dev, "PCIe Bus VendorID Reg: 0x%04x\n", (u16) ret);

        ret = pericom_read_word_data(client, port, 0x02);
	if (ret >= 0)
                dev_info(dev, "PCIe Bus DeviceID Reg: 0x%04x\n", (u16) ret);


#if 0 /* PROVASENZACONFIG */

        /* PORT 0 - Uplink */
        val = i2c_smbus_write_word_data(client, 0x08, 0x18);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0201);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x1a);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0005);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x1c);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x00f0);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x20);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2010);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x22);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2040);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x24);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0xfff1);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x26);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0001);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }


        /* PORT 1 */
        port = 0x01;

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x18));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0302);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1a));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0003);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1c));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x00f0);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x20));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2010);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x22));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2020);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x24));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0xfff1);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x26));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0001);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }


        /* PORT 2 */
        port = 0x02;

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x18));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0402);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1a));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0004);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1c));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x00f0);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x20));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2020);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x22));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2030);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }


        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x24));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0xfff1);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x26));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0001);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }


        /* PORT 3 */
        port = 0x03;

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x18));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0502);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1a));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0005);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1c));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x00f0);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x20));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2030);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x22));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x2040);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x24));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0xfff1);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x26));
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0001);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }


        /* Port 0 - Uplink */
        port = 0;
        val = i2c_smbus_write_word_data(client, 0x08, 0x04);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

        val = i2c_smbus_write_word_data(client, 0x08, 0x0007);
	if (val < 0) {
	        dev_err(&client->dev, "failed to read: %d\n", val);
        }

#endif  /* PROVASENZACONFIG */


#if 0
        data->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(data->reset_gpio)) {
		dev_err(dev, "Failed to bind reset gpio");
		return -ENODEV;
	}

	/* Toggle RESET_N to reset the hub. */
	gpiod_set_value(data->reset_gpio, 1);
	usleep_range(5, 20);
	gpiod_set_value(data->reset_gpio, 0);
	msleep(5);

        for (i = 0x00; i < 0xff; i++, i++) {
                val = i2c_smbus_write_word_data(client, 0x08, i);
	        if (val < 0) {
	        	dev_err(&client->dev, "failed to read: %d\n", val);
                }

                val = i2c_smbus_read_word_data(client, 0x08);
	        if (val < 0) {
	        	dev_err(&client->dev, "failed to write: %d\n", val);
                }

                dev_info(dev, "PCIe Reg (0x%02x): 0x%04x\n", i, val);
        }
#endif


#ifdef PERICOM_DEBUG
        for (port = 0; port <= 3; port++) {
                for (i = 0x00; i < 0xff; i++, i++) {
                        val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | i));
	                if (val < 0) {
	                	dev_err(&client->dev, "failed to read: %d\n", val);
                        }

                        val = i2c_smbus_read_word_data(client, 0x08);
	                if (val < 0) {
	                	dev_err(&client->dev, "failed to write: %d\n", val);
                        }

                        dev_dbg(dev, "PCIeReg - Port %d (0x%02x): 0x%04x\n", port, i, val);
                }
        }

        for (port = 0; port <= 3; port++) {
                /* Funziona la prima lettura a word, bisogna capire come fare la seconda */
                val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x18));
	        if (val < 0) {
	        	dev_err(&client->dev, "failed to read: %d\n", val);
                }

                val = i2c_smbus_read_word_data(client, 0x08);
	        if (val < 0) {
	        	dev_err(&client->dev, "failed to write: %d\n", val);
                }

                dev_info(dev, "PCIe Bus Number - Port %d [0x18]: 0x%04x\n", port, val);


                val = i2c_smbus_write_word_data(client, 0x08, ((port << 8) | 0x1a));
	        if (val < 0) {
	        	dev_err(&client->dev, "failed to read: %d\n", val);
                }

                val = i2c_smbus_read_word_data(client, 0x08);
	        if (val < 0) {
	        	dev_err(&client->dev, "failed to write: %d\n", val);
                }

                dev_info(dev, "PCIe Bus Number - Port %d [0x1a]: 0x%04x\n", port, val);
        }
#endif /* PERICOM_DEBUG */

	return 0;
}




static const struct i2c_device_id pi7c9x2g404_id[] = {
	{ "pi7c9x2g404", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, pi7c9x2g404_id);

#ifdef CONFIG_OF
static const struct of_device_id pi7c9x2g404_of_match[] = {
	{ .compatible = "pericom,pi7c9x2g404" },
	{}
};
MODULE_DEVICE_TABLE(of, pi7c9x2g404_of_match);
#endif

static struct i2c_driver pi7c9x2g404_driver = {
	.driver = {
		.name = "pciesw-pi7c9x2g404",
		.of_match_table = of_match_ptr(pi7c9x2g404_of_match),
	},
	.probe = pi7c9x2g404_probe,
	.id_table = pi7c9x2g404_id,
};

module_i2c_driver(pi7c9x2g404_driver);

MODULE_AUTHOR("Davide Ballestrero <davide.ballestrero@leonardocompany.com>");
MODULE_DESCRIPTION("Pericom PCIe Switch driver");
MODULE_LICENSE("GPL v2");
