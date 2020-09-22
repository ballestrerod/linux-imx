/*
 * Bachmann enobu leds driver.
 *
 * Author: Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *         Christian Gmeiner <christian.gmeiner@gmail.com>
 *
 * License: GPL as published by the FSF.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/io.h>
#include <linux/fpga/machxo-efb.h>


struct enobu_led {
	struct led_classdev cdev;
	const char *name;
	u8 mask;
};

static struct enobu_led leds[] = {
	{
		.name = "poe_8",
		.mask = BIT(7),
	},
	{
		.name = "poe_7",
		.mask = BIT(6),
	},
	{
		.name = "poe_6",
		.mask = BIT(5),
	},
	{
		.name = "poe_1",
		.mask = BIT(4),
	},
	{
		.name = "poe_5",
		.mask = BIT(3),
	},
	{
		.name = "poe_3",
		.mask = BIT(2),
	},
	{
		.name = "poe_2",
		.mask = BIT(1),
	},
	{
		.name = "poe_4",
		.mask = BIT(0),
	}
};

#define ENOBU_FPGA_LED_POE          0x0b


#define to_enobu_led(x) container_of(x, struct enobu_led, cdev)

static void enobu_led_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	//struct enobu_led *led = container_of(led_cdev, struct enobu_led, cdev);
	struct enobu_led *led = to_enobu_led(led_cdev);
	u8 val;

	efb_spi_read(ENOBU_FPGA_LED_POE, &val);

	if (value == LED_OFF)
		val &= ~led->mask;
	else
		val |= led->mask;

	efb_spi_write(ENOBU_FPGA_LED_POE, val);
}

static enum led_brightness enobu_led_brightness_get(struct led_classdev *led_cdev)
{
	//struct enobu_led *led = container_of(led_cdev, struct enobu_led, cdev);
	struct enobu_led *led = to_enobu_led(led_cdev);
	u8 val;

	efb_spi_read(ENOBU_FPGA_LED_POE, &val);

	return val & led->mask ? LED_FULL : LED_OFF;
}

static int enobu_led_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int i;
	int ret;
        
	for (i = 0; i < ARRAY_SIZE(leds); i++) {

		leds[i].cdev.name = leds[i].name;
		leds[i].cdev.brightness_set = enobu_led_brightness_set;
		leds[i].cdev.brightness_get = enobu_led_brightness_get;
		leds[i].cdev.default_trigger = leds[i].name;

		ret = devm_led_classdev_register(&pdev->dev, &leds[i].cdev);
		if (ret < 0)
			return ret;
	}

	dev_info(dev, "eNOBU LEDS initialized\n");

	return 0;
}

static const struct of_device_id enobu_led_of_match[] = {
        { .compatible = "leonardo,enobu-fpga-leds" },
        { }
};
MODULE_DEVICE_TABLE(of, enobu_led_of_match);

static struct platform_driver enobu_led_driver = {
	.probe = enobu_led_probe,
	.driver	= {
		.name = "enobu-fpga-leds",
		.of_match_table = enobu_led_of_match,
	},
};

module_platform_driver(enobu_led_driver);

MODULE_AUTHOR("Davide Ballestrero, Leonardo");
MODULE_DESCRIPTION("eNOBU FPGA LED driver");
MODULE_LICENSE("GPL");
