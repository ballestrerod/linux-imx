/*
 * I2C access driver for TI TPS2388 PoE
 *
 * Original code is
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 *
 *	Modified by Sergio Biasi, Jul 2020
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

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/regmap.h>

#include <linux/power/tps2388_poe.h>
#include <linux/hwmon-sysfs.h>

#if defined(CONFIG_OF)
static const struct of_device_id tps2388_i2c_of_match_table[] = {
	{ .compatible = "ti,tps2388", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, tps2388_i2c_of_match_table);
#endif

#if 0 // TODO
static const struct regmap_range tps2388_yes_ranges[] = {
	regmap_reg_range(TPS2388_INT_STS, TPS2388_GPIO5),
};

static const struct regmap_access_table tps2388_volatile_table = {
	.yes_ranges = tps2388_yes_ranges,
	.n_yes_ranges = ARRAY_SIZE(tps2388_yes_ranges),
};

const struct regmap_config tps2388_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
	.volatile_table = &tps2388_volatile_table,
};
#endif // 0

#if 0
int tps2388_device_init(struct tps2388 *tps)
{
	int ret;

#if 0
	ret = regmap_add_irq_chip(tps->regmap, tps->irq, IRQF_ONESHOT, 0,
								&tps2388_irq_chip, &tps->irq_data);
	if (ret)
		return ret;
#endif //0

	ret = i2c_new_device(tps->dev, PLATFORM_DEVID_AUTO, tps2388_cells,
								ARRAY_SIZE(tps2388_cells), NULL, 0,
								regmap_irq_get_domain(tps->irq_data));
#if 0
	if (ret) {
		regmap_del_irq_chip(tps->irq, tps->irq_data);
		return ret;
	}
#endif //0

	return 0;
}

int tps2388_device_exit(struct tps2388 *tps)
{
    regmap_del_irq_chip(tps->irq, tps->irq_data);

    return 0;
}
#endif // 0

/* sysfs callback function */
static ssize_t show_status(struct device *dev, struct device_attribute *da,
				    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	int port_status, port_current, port_voltage, port_resistance;
	int index = attr->index;

	port_status     = i2c_smbus_read_byte_data(client, TPS2388_PORT_1_STATUS + index);
	port_current    = i2c_smbus_read_word_data(client, TPS2388_PORT_1_CURRENT + 4 * index);
	port_voltage    = i2c_smbus_read_word_data(client, TPS2388_PORT_1_VOLTAGE + 4 * index);
	port_resistance = i2c_smbus_read_byte_data(client, TPS2388_PORT_1_DET_RES + index);

	return sprintf(buf, "CL %d, DET %d %4dmA, %4dV, %dkOhm\n",
					(port_status & 0xF0)>>4, (port_status & 0x0F),
					((port_current & 0x3FFF) * 61035) / 1000000,
					((port_voltage & 0x3FFF) *  3662) / 1000000,
					(port_resistance * 1953125) / 10000000);
}

/* In celsius degree. Formula is: (value read * 0.652) - 20 */
static ssize_t show_temperature(struct device *dev, struct device_attribute *da,
				    char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int temperature, decimal;

	temperature = i2c_smbus_read_byte_data(client, TPS2388_TEMPERATURE);

	/* Sorry float ops not allowed... */
	decimal = ((temperature * 652) / 100) % 10;

	return sprintf(buf, "%d.%d (step 0.652)\n", (temperature * 652)/1000 - 20, decimal);
}

static ssize_t show_poe_enable(struct device *dev, struct device_attribute *da,
				    char *buf)
{
	return sprintf(buf, "Write port index to enable [1..4]\n");
}

static ssize_t store_poe_enable(struct device *dev, struct device_attribute *da,
				               const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tps2388 *tps = dev_get_drvdata(dev);
	long val;
	int err;

	err = kstrtol(buf, 10, &val);
	if (err)
		return err;

	/* switch from real world to kernel world */
	val--;

	if (val < 0 || val > 3)
	{
		return -EINVAL;
	}

	mutex_lock(&tps->update_lock);
	/* Re-enable detect/class in case it was disabled by an OFF port state */
	i2c_smbus_write_byte_data(client, TPS2388_DETECTCLASS_RES, 0x11 << val);

	i2c_smbus_write_byte_data(client, TPS2388_IEEE_POWER_EN, 1 << val);
	mutex_unlock(&tps->update_lock);

	led_trigger_event(&tps->trig[val], LED_FULL);

	return count;
}


static ssize_t show_poe_disable(struct device *dev, struct device_attribute *da,
				    char *buf)
{
	return sprintf(buf, "Write port index to disable [1..4]\n");
}


static ssize_t store_poe_disable(struct device *dev, struct device_attribute *da,
				               const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tps2388 *data = dev_get_drvdata(dev);
	long val;
	int err;

	err = kstrtol(buf, 10, &val);
	if (err)
		return err;

	/* switch from real world to kernel world */
	val--;

	if (val < 0 || val > 3)
	{
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);

	i2c_smbus_write_byte_data(client, TPS2388_POWER_EN, 0x10 << val);

	mutex_unlock(&data->update_lock);

	return count;
}


static ssize_t show_detect_class(struct device *dev, struct device_attribute *da,
				               char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int detect_enable;
	int i, n;
	char result[128];

	detect_enable = i2c_smbus_read_byte_data(client, TPS2388_DETECT_CLASS_EN);


	/* Detect / class */
	n = sprintf(result, "Class [ ");
	for (i = 0; i < 4; i++)
		n += sprintf(result + n, "%c ", (detect_enable & (0x10 << i)? '1'+i:'-'));
	n += sprintf(result + n, "] Detect [ ");
	for (i = 0; i < 4; i++)
		n += sprintf(result + n, "%c ", (detect_enable & (0x01 << i)? '1'+i:'-'));
	n += sprintf(result + n, "]\n");

	return sprintf(buf, "%s", result);
};


static const struct sensor_device_attribute tps2388_status[] = {
	POE_ATTR(p1_status, S_IRUGO, show_status, NULL, 0),
	POE_ATTR(p2_status, S_IRUGO, show_status, NULL, 1),
	POE_ATTR(p3_status, S_IRUGO, show_status, NULL, 2),
	POE_ATTR(p4_status, S_IRUGO, show_status, NULL, 3),
};

static const struct sensor_device_attribute tps2388_temperature[] = {
	POE_ATTR(temperature, S_IRUGO, show_temperature, NULL, 0),
};

static const struct sensor_device_attribute tps2388_poe_enable[] = {
	POE_ATTR(enable_poe, S_IWUSR | S_IRUGO, show_poe_enable, store_poe_enable, 0),
};

static const struct sensor_device_attribute tps2388_poe_disable[] = {
	POE_ATTR(disable_poe, S_IWUSR | S_IRUGO, show_poe_disable, store_poe_disable, 0),
};

static const struct sensor_device_attribute tps2388_detect_class[] = {
	POE_ATTR(detect_class, S_IRUGO, show_detect_class, NULL, 0),
};

static int tps2388_configure_single_port(struct i2c_client *client, struct tps2388 *tps, int index)
{
	struct device *dev = tps->dev; 
	unsigned char val;

	/* Configure default operating mode for each port */
	if (tps->port_data[index].enabled)
	{
		val = i2c_smbus_read_byte_data(client, TPS2388_OP_MODE);

		if (((val >> (index * 2)) & 0x03) == tps->port_data[index].op_mode)
		{
			dev_info(dev, "Op mode already set\n");
			return 0;
		}

		/* Mask 2 bits to change */
		val &= ~(0x03 << (index * 2));

		val |= tps->port_data[index].op_mode << (index*2);

		mutex_lock(&tps->update_lock);
		i2c_smbus_write_byte_data(client, TPS2388_OP_MODE, val);
		mutex_unlock(&tps->update_lock);

		if (tps->port_data[index].det_class_en)
		{
			msleep(200);

			/* Detect/class enable */
			mutex_lock(&tps->update_lock);
			//i2c_smbus_write_byte_data(client, TPS2388_DETECT_CLASS_EN, val);
			i2c_smbus_write_byte_data(client, TPS2388_DETECTCLASS_RES, 0x11 << index);

			/* Enable power by default */
			i2c_smbus_write_byte_data(client, TPS2388_IEEE_POWER_EN, 1 << val);
			mutex_unlock(&tps->update_lock);
		}
	}

	return 0;
};

#ifdef CONFIG_OF
static int tps2388_of_property_parse_op_mode(struct device_node *client,
				unsigned int *result)
{
		const char *mode;
		int err;

		err = of_property_read_string(client, "op_mode", &mode);
		if (err < 0)
				return err;

		if (strcmp(mode, "auto") == 0)
				*result = TPS2388_MODE_SEMIAUTO;
		else if (strcmp(mode, "manual") == 0)
				*result = TPS2388_MODE_MANUAL;
		else if (strcmp(mode, "off") == 0)
				*result = TPS2388_MODE_OFF;
		else
				return -EINVAL;

		return 0;
}

#if IS_ENABLED(CONFIG_LEDS_TRIGGERS)
/* POE LEDs triggers */
static void poe_led_trigger_activate(struct led_classdev *led_cdev)
{
	dev_warn(led_cdev->dev, "Activated POE led\n");

	/* Su sola attivazione, senza device collegato, si potrebbe far lampeggiare il led poe */
#if 0
	struct kbd_led_trigger *trigger =
			container_of(cdev->trigger, struct kbd_led_trigger, trigger);

	tasklet_disable(&keyboard_tasklet);
	if (ledstate != -1U)
			led_trigger_event(&trigger->trigger,
						ledstate & trigger->mask ?
						LED_FULL : LED_OFF);
	tasklet_enable(&keyboard_tasklet);
#endif
}
#endif

static int tps2388_get_port_config_of(struct i2c_client *client)
{
	struct tps2388 *data = i2c_get_clientdata(client);
	struct device_node *node;
	unsigned int ports = 0;
	u32 pval;

	if (!client->dev.of_node
					|| !of_get_next_child(client->dev.of_node, NULL))
		return -EINVAL;

	if (of_property_read_u32(client->dev.of_node, "ieee_enable", &pval)) {
			dev_err(&client->dev, "Invalid ieee_enable field\n");
	}

	if (pval == 1)
	{
		data->ieee_mode = 1;
	}

	for_each_child_of_node(client->dev.of_node, node) {
		unsigned int port;
		unsigned int op_mode      = TPS2388_DEFAULT_OP_MODE;
		unsigned int led_pos      = TPS2388_DEFAULT_LED_POS;
		unsigned int det_class_en = TPS2388_DEFAULT_DETECT_CLASS;

		if (of_property_read_u32(node, "id", &pval)) {
				dev_err(&client->dev, "Invalid id on %pOF\n", node);
				continue;
		}
		port = pval;

		if (port < 0 || port >= TPS2388_PORT_MAX) {
			dev_err(&client->dev, "Invalid port index %d on %pOF\n", port, node);
			continue;
		}

		if (tps2388_of_property_parse_op_mode(node, &pval) < 0)
		{
			dev_err(&client->dev, "Invalid op mode on %pOF\n", node);
			continue;
		}
		op_mode = pval;

		/* POE ports does not match with physical position. Remap LEDs in DTS */
		if (of_property_read_u32(node, "led_pos", &pval)) {
			dev_warn(&client->dev, "Invalid LED position on %pOF\n", node);
		}
		if (pval > 0 || pval < 9)	/* Only admitted values */
			led_pos = pval;

		if (!of_property_read_u32(node, "det_class_en", &pval)) {
			det_class_en = pval;
			if (det_class_en < 0) {
				dev_err(&client->dev, "Invalid detect/class on %pOF\n", node);
				continue;
			}
		}

		data->port_data[port].enabled = true;
		data->port_data[port].op_mode = op_mode;
		data->port_data[port].det_class_en = (det_class_en > 0);

#if IS_ENABLED(CONFIG_LEDS_TRIGGERS)
		data->trig[port].name = kasprintf(GFP_KERNEL, "poe_%d", led_pos);
		data->trig[port].activate = poe_led_trigger_activate;

		devm_led_trigger_register(&client->dev, &data->trig[port]);
#endif
		ports++;
	}

	dev_err(&client->dev, "Found %d ports in DTS\n", ports);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_LEDS_TRIGGERS)
static void tps3288_turn_off_led(struct tps2388 *tps, int leds)
{
	int n;

	for (n = 0; n < 4; n++)
	{
		if (leds & (1 << n))
		{
			led_trigger_event(&tps->trig[n], LED_OFF);
		}
	}
}

static void tps3288_turn_on_led(struct tps2388 *tps, int leds)
{
	int n;

	for (n = 0; n < 4; n++)
	{
		if (leds & (1 << n))
		{
			led_trigger_event(&tps->trig[n], LED_FULL);
		}
	}
}
#endif


/* Polling function because IRQ is not available... */
static void tps2388_poll_work(struct work_struct *work)
{
	struct tps2388 *tps = container_of(work, struct tps2388, dis_connect);

	int poll_class, fault_detect, power_change;

	poll_class = i2c_smbus_read_byte_data(tps->client, TPS2388_DETECT_EV_CLEAR);

	if (poll_class > 0)
	{
		dev_dbg(tps->dev, "Detect/class event 0x%02X\n", poll_class);

		/* Reset check used below */
		if ((poll_class & 0xF0) < (tps->last_read & 0xF0))
		{
			tps->last_read = 0;
		}

		/* Classification is set in high nibble */
		/* Second condition is needed because in the interval passed between the read (and clear)
		 * of first classification and the power to the port, other classifications occur
		 * on that port causing a second power up */
		if ((poll_class & 0xF0) && (tps->last_read != poll_class))
		{
			mutex_lock(&tps->update_lock);
			tps->last_read = poll_class;
			i2c_smbus_write_byte_data(tps->client, TPS2388_IEEE_POWER_EN, poll_class >> 4);
			mutex_unlock(&tps->update_lock);

			dev_info(tps->dev, "PoE: at least one classification occurred, power on the port(s)\n");

			/* Let the device(s) power up */
			msleep(200);
		}
	}
	
	/* Look at power change */
	power_change = i2c_smbus_read_byte_data(tps->client, TPS2388_POWER_EV_CLEAR);

	/* Read disconnections and overloads */
	fault_detect = i2c_smbus_read_byte_data(tps->client, TPS2388_FAULT_EV_CLEAR);

	if (fault_detect & 0x0F)
	{
		/* At least one overload occurred!
		   By default, port is automatically powered off
		   Report it */
		dev_warn(tps->dev, "Overload occurred: 0x%02X\n", fault_detect & 0x0F);
	}

	/* If power status changed, should be connect or disconnect. Check it */
	if (power_change > 0)
	{
		dev_dbg(tps->dev, "Power change event 0x%02X\n", power_change);

		if (fault_detect > 0)
		{
			/* Is a disconnection? */
			if ((fault_detect & power_change) != (fault_detect & 0xF0))
			{
				dev_warn(tps->dev, "Fault and power disagree: 0x%02X / 0x%02X\n", fault_detect & 0xF0, power_change);
			}
#if IS_ENABLED(CONFIG_LEDS_TRIGGERS)
			tps3288_turn_off_led(tps, fault_detect >> 4);
#endif
			dev_err(tps->dev, "Fault/disconnect event 0x%02X\n", fault_detect);
		} else
		{
			/* This is a connection */
#if IS_ENABLED(CONFIG_LEDS_TRIGGERS)
			tps3288_turn_on_led(tps, power_change >> 4);
#endif
		}
	}

	/* Leave these two or led triggers above cause a "scheduling while atomic" (understand why!) */
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(1 * HZ);

	return;
}


static void tps2388_poll_function(unsigned long _data)
{
	struct tps2388*data = (struct tps2388*)_data;

	schedule_work(&data->dis_connect);

	/* Reschedule poll in 1 second */
	mod_timer(&data->poll_timer, jiffies + msecs_to_jiffies(1000));
}


/* Return device index (0/1 in Configuration A or 0 in configuration B) if detection is successful, -ENODEV otherwise */
static int tps2388_detect(struct i2c_client *client)
{
	struct i2c_adapter *adapter = client->adapter;
	struct tps2388 *data = i2c_get_clientdata(client);
	int address = client->addr;
	int dev_id, chip_id, temper, fw_ver, general_mask;
	int dev_index = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
			return -ENODEV;

	/* detection and identification */
	dev_id = i2c_smbus_read_byte_data(client, TPS2388_DEVICE_ID);

	if ((dev_id >> 5) != 0x06)	/* 110x.xxxx b */
	{
		dev_err(&adapter->dev, "Bad dev id (0x%02X) at 0x%02x\n", dev_id, TPS2388_DEVICE_ID);
		return -ENODEV;
	}

	chip_id = i2c_smbus_read_byte_data(client, TPS2388_ID);
	temper  = i2c_smbus_read_byte_data(client, TPS2388_TEMPERATURE);
	fw_ver  = i2c_smbus_read_byte_data(client, TPS2388_FIRMWARE_REV);

	if (chip_id < 0 || temper < 0)
	{
			return -ENODEV;
	}

	dev_warn(&adapter->dev, "Found TPS2388 @ 0x%02x - Mfg.Ver %02d.%02d Silicon %02d Firmware 0x%02x\n",
					address, chip_id >> 3, chip_id & 0x07, (dev_id & 0x1F), fw_ver);

	/* Read Configuration type */
	general_mask = i2c_smbus_read_byte_data(client, TPS2388_GENERAL_MASK);

	/* If Configuration is A set device index, else it is zero */
	if (!(general_mask & TPS2388_GMASK_NBITACC))
			dev_index = ((i2c_smbus_read_byte_data(client, TPS2388_PIN_STATUS) & TPS2388_PSTATUS_SLA0) > 0);

	dev_err(&adapter->dev, "dev_index %d, general_mask 0x%02X\n", dev_index, general_mask);

	/* Disable INTEN, actually not used (Jul20) */
	mutex_lock(&data->update_lock);
	i2c_smbus_write_byte_data(client, TPS2388_GENERAL_MASK, general_mask & ~TPS2388_GMASK_INTEN);
	mutex_unlock(&data->update_lock);

	return dev_index;
}

static int tps2388_probe(struct i2c_client *client,
			      const struct i2c_device_id *ids)
{
	struct tps2388 *tps;
	struct device *dev = &client->dev;
	int k, err, val, disconnect = 0;

	tps = devm_kzalloc(dev, sizeof(*tps), GFP_KERNEL);
	if (tps == NULL)
	{
		dev_err(dev, "failed to allocate device struct\n");
		return -ENOMEM;
	}

	tps->client = client;

	i2c_set_clientdata(client, tps);
	mutex_init(&tps->update_lock);

	tps->dev = &client->dev;
	//tps->irq = client->irq;

#if 0 // TODO
	tps->regmap = devm_regmap_init_i2c(client, &tps2388_regmap_config);
	if (IS_ERR(tps->regmap)) {
		dev_err(dev, "Failed to initialize register map\n");
		return PTR_ERR(tps->regmap);
	}
#endif // TODO

	/* Indentify device */
	tps->dev_index = tps2388_detect(client);

	if (tps->dev_index < 0)
	{
		return -ENODEV;
	}

	/* Read DTS configuration */
	tps2388_get_port_config_of(client);

	if (tps->ieee_mode)
	{
		val = i2c_smbus_read_byte_data(client, TPS2388_POE_PLUS);

		mutex_lock(&tps->update_lock);
		i2c_smbus_write_byte_data(client, TPS2388_POE_PLUS, val | TPS2388_POE_PLUS_TPON);
		mutex_unlock(&tps->update_lock);

		dev_info(&client->dev, "IEEE mode enabled\n");
	}


	/* Build sysfs attribute group */
    for (k = 0; k < TPS2388_PORT_MAX; ++k) {
//		if (!tps->port_data[k].enabled)
//			continue;

		dev_err(dev, "device_create_file %d (%d %d %d)\n", k, tps->port_data[k].enabled, tps->port_data[k].op_mode, tps->port_data[k].det_class_en);
		err = device_create_file(&client->dev, &tps2388_status[k].dev_attr);
		if (err)
			goto exit_remove;

		tps2388_configure_single_port(client, tps, k);

		/* Build disconnect value to write in register */
		if (tps->port_data[k].enabled)
			disconnect |= 1 << k;
	}

	mutex_lock(&tps->update_lock);
	i2c_smbus_write_byte_data(client, TPS2388_DISCONNECT_EN, disconnect);
	mutex_unlock(&tps->update_lock);

	err = device_create_file(&client->dev, &tps2388_temperature->dev_attr);
	if (err)
		goto exit_remove;

	err = device_create_file(&client->dev, &tps2388_poe_enable->dev_attr);
	if (err)
		goto exit_remove;

	err = device_create_file(&client->dev, &tps2388_poe_disable->dev_attr);
	if (err)
		goto exit_remove;

	err = device_create_file(&client->dev, &tps2388_detect_class->dev_attr);
	if (err)
		goto exit_remove;

	/* Setup work task to poll for changes */
	INIT_WORK(&tps->dis_connect, tps2388_poll_work);

	/* Setup timer for detection (no GPIO, no IRQ!) */
	setup_timer(&tps->poll_timer, tps2388_poll_function, (unsigned long)tps);

	/* Start timer for polling function */
	mod_timer(&tps->poll_timer, jiffies + msecs_to_jiffies(2000));

	/* Look at fw revision */
	dev_info(tps->dev, "Registered device\n");

	return 0;

exit_remove: 
	for (k = 0; k < TPS2388_PORT_MAX; k++)
		device_remove_file(&client->dev, &tps2388_status[k].dev_attr);
	return err;
}


static int tps2388_remove(struct i2c_client *client)
{
	//struct tps2388 *tps = i2c_get_clientdata(client);

	//kfree(tps);

	return 0; //tps2388_device_exit(tps);
}

static const struct i2c_device_id tps2388_id_table[] = {
	{ "tps2388", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, tps2388_id_table);

static struct i2c_driver tps2388_i2c_driver = {
	.driver		= {
		.name	= "tps2388",
		.of_match_table = tps2388_i2c_of_match_table,
	},
	.id_table	= tps2388_id_table,
	.probe		= tps2388_probe,
	.remove		= tps2388_remove,
};

module_i2c_driver(tps2388_i2c_driver);

MODULE_AUTHOR("Sergio Biasi <belinux@gmail.com>");
MODULE_DESCRIPTION("TPS2388 I2C Interface Driver");
MODULE_LICENSE("GPL v2");
