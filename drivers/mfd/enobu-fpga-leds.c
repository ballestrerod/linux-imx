/*
 * Bachmann enobu leds driver.
 *
 * Author: Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *         Christian Gmeiner <christian.gmeiner@gmail.com>
 *
 * License: GPL as published by the FSF.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/io.h>

#include <linux/mfd/enobu-fpga.h>
//#include <linux/fpga/machxo-efb.h>


/* FPGA registers to handle front LEDs */
#define ENOBU_FPGA_MAX_LEDS     10


struct enobu_led {
	int                     id;
	//? const char              *name;
	struct enobu_leds       *parent;
	struct led_classdev     cdev;
	enum led_brightness     current_brightness;
	u16                     reg;
	u8                      mask;
};

struct enobu_leds {
        struct device           *dev;
        struct enobu_fpga       *enobufpga;
        struct mutex            lock;
        struct enobu_led        *led[ENOBU_FPGA_MAX_LEDS];
};


//TODO configure in devicetree and delete
struct enobu_led_reg {
	const char   *name;
	u16          reg;
	u8           mask;
};


static struct enobu_led_reg led_reg[] = {
	{ .name = "spare_2",    .reg = ENOBU_FPGA_REG_SPARELED,   .mask = BIT(1), },
	{ .name = "spare_1",    .reg = ENOBU_FPGA_REG_SPARELED,   .mask = BIT(0), },
	{ .name = "poe_8",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(7), },
	{ .name = "poe_7",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(6), },
	{ .name = "poe_6",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(5), },
	{ .name = "poe_5",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(4), },
	{ .name = "poe_4",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(3), },
	{ .name = "poe_3",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(2), },
	{ .name = "poe_2",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(1), },
	{ .name = "poe_1",      .reg = ENOBU_FPGA_REG_POELED,     .mask = BIT(0), }
};

#define to_enobu_led(x) container_of(x, struct enobu_led, cdev)

static int enobu_led_brightness_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	//struct enobu_led *led = container_of(led_cdev, struct enobu_led, cdev);
	struct enobu_led *led = to_enobu_led(led_cdev);
        struct enobu_leds *leds = led->parent;
	unsigned int val;
        int ret;

        mutex_lock(&leds->lock);

        ret = enobu_fpga_reg_read(leds->enobufpga, led->reg, &val);
 	if (ret < 0)
		goto out;

	if (value == LED_OFF)
		val &= ~led->mask;
	else
		val |= led->mask;

        ret = enobu_fpga_reg_write(leds->enobufpga, led->reg, val);
 	if (ret < 0)
		goto out;

        led->current_brightness = val;

out:        
        mutex_unlock(&leds->lock);

        return ret;
}



static enum led_brightness enobu_led_brightness_get(struct led_classdev *led_cdev)
{
	//struct enobu_led *led = container_of(led_cdev, struct enobu_led, cdev);
	struct enobu_led *led = to_enobu_led(led_cdev);
        struct enobu_leds *leds = led->parent;
	unsigned int val;

        mutex_lock(&leds->lock);

        enobu_fpga_reg_read(leds->enobufpga, led->reg, &val);
        
        mutex_unlock(&leds->lock);

	return val & led->mask ? LED_FULL : LED_OFF;
}



static int enobu_led_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
        struct enobu_fpga *enobufpga = dev_get_drvdata(pdev->dev.parent);
        struct enobu_leds *leds;
        struct enobu_led  *led;
	int i;
	int ret;

	leds = devm_kzalloc(dev, sizeof(*leds), GFP_KERNEL);
	if (!leds)
		return -ENOMEM;

	platform_set_drvdata(pdev, leds);
        leds->dev = dev;

       	/*
	 * leds->enobufpga points to the underlying bus for the register
	 * controlled.
	 */
	leds->enobufpga = enobufpga;
	mutex_init(&leds->lock);

	for (i = 0; i < ENOBU_FPGA_MAX_LEDS; i++) {
                led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
		if (!led) {
			ret = -ENOMEM;
			goto out_err;
		}

                leds->led[i] = led;
                leds->led[i]->id = i;
                leds->led[i]->reg = led_reg[i].reg;
                leds->led[i]->mask = led_reg[i].mask;
                leds->led[i]->current_brightness = 0;
		leds->led[i]->cdev.name = led_reg[i].name;
		leds->led[i]->cdev.brightness_set_blocking = 
                                        enobu_led_brightness_set;
		leds->led[i]->cdev.brightness_get = 
                                        enobu_led_brightness_get;
		leds->led[i]->cdev.default_trigger = led_reg[i].name;

		leds->led[i]->parent = leds;

		ret = devm_led_classdev_register(dev, &leds->led[i]->cdev);
		if (ret < 0)
			return ret;
	}

	dev_info(dev, "eNOBU LEDs initialized\n");

	return 0;

out_err:
	return ret;
}


#ifdef CONFIG_OF
static const struct of_device_id enobu_led_of_match[] = {
        { .compatible = "leonardo,enobu-fpga-leds", },
        { }
};
MODULE_DEVICE_TABLE(of, enobu_led_of_match);
#endif

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
