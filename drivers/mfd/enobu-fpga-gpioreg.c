#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/gpio-reg.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

struct enobu_gpioreg {
	struct gpio_chip gc;
	spinlock_t lock;
	u32 direction;
	u32 out;
	void __iomem *reg;
	struct irq_domain *irqdomain;
	const int *irqs;
};

#define to_enobu_gpioreg(x) container_of(x, struct enobu_gpioreg, gc)

static int enobu_gpioreg_get_direction(struct gpio_chip *gc, unsigned offset)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);

	return r->direction & BIT(offset) ? 1 : 0;
}

static int enobu_gpioreg_direction_output(struct gpio_chip *gc, unsigned offset,
	int value)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);

	if (r->direction & BIT(offset))
		return -ENOTSUPP;

	gc->set(gc, offset, value);
	return 0;
}

static int enobu_gpioreg_direction_input(struct gpio_chip *gc, unsigned offset)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);

	return r->direction & BIT(offset) ? 0 : -ENOTSUPP;
}

static void enobu_gpioreg_set(struct gpio_chip *gc, unsigned offset, int value)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);
	unsigned long flags;
	u32 val, mask = BIT(offset);

	spin_lock_irqsave(&r->lock, flags);
	val = r->out;
	if (value)
		val |= mask;
	else
		val &= ~mask;
	r->out = val;
	writel_relaxed(val, r->reg);
	spin_unlock_irqrestore(&r->lock, flags);
}

static int enobu_gpioreg_get(struct gpio_chip *gc, unsigned offset)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);
	u32 val, mask = BIT(offset);

	if (r->direction & mask) {
		/*
		 * double-read the value, some registers latch after the
		 * first read.
		 */
		readl_relaxed(r->reg);
		val = readl_relaxed(r->reg);
	} else {
		val = r->out;
	}
	return !!(val & mask);
}

static void enobu_gpioreg_set_multiple(struct gpio_chip *gc, unsigned long *mask,
	unsigned long *bits)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);
	unsigned long flags;

	spin_lock_irqsave(&r->lock, flags);
	r->out = (r->out & ~*mask) | (*bits & *mask);
	writel_relaxed(r->out, r->reg);
	spin_unlock_irqrestore(&r->lock, flags);
}

static int enobu_gpioreg_to_irq(struct gpio_chip *gc, unsigned offset)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);
	int irq = r->irqs[offset];

	if (irq >= 0 && r->irqdomain)
		irq = irq_find_mapping(r->irqdomain, irq);

	return irq;
}

/**
 * gpio_reg_init - add a fixed in/out register as gpio
 * @dev: optional struct device associated with this register
 * @base: start gpio number, or -1 to allocate
 * @num: number of GPIOs, maximum 32
 * @label: GPIO chip label
 * @direction: bitmask of fixed direction, one per GPIO signal, 1 = in
 * @def_out: initial GPIO output value
 * @names: array of %num strings describing each GPIO signal or %NULL
 * @irqdom: irq domain or %NULL
 * @irqs: array of %num ints describing the interrupt mapping for each
 *        GPIO signal, or %NULL.  If @irqdom is %NULL, then this
 *        describes the Linux interrupt number, otherwise it describes
 *        the hardware interrupt number in the specified irq domain.
 *
 * Add a single-register GPIO device containing up to 32 GPIO signals,
 * where each GPIO has a fixed input or output configuration.  Only
 * input GPIOs are assumed to be readable from the register, and only
 * then after a double-read.  Output values are assumed not to be
 * readable.
 */
struct gpio_chip *enobu_gpioreg_init(struct device *dev, void __iomem *reg,
	int base, int num, const char *label, u32 direction, u32 def_out,
	const char *const *names, struct irq_domain *irqdom, const int *irqs)
{
	struct enobu_gpioreg *r;
	int ret;

	if (dev)
		r = devm_kzalloc(dev, sizeof(*r), GFP_KERNEL);
	else
		r = kzalloc(sizeof(*r), GFP_KERNEL);

	if (!r)
		return ERR_PTR(-ENOMEM);

	spin_lock_init(&r->lock);

	r->gc.label = label;
	r->gc.get_direction = enobu_gpioreg_get_direction;
	r->gc.direction_input = enobu_gpioreg_direction_input;
	r->gc.direction_output = enobu_gpioreg_direction_output;
	r->gc.set = enobu_gpioreg_set;
	r->gc.get = enobu_gpioreg_get;
	r->gc.set_multiple = enobu_gpioreg_set_multiple;
	if (irqs)
		r->gc.to_irq = enobu_gpioreg_to_irq;
	r->gc.base = base;
	r->gc.ngpio = num;
	r->gc.names = names;
	r->direction = direction;
	r->out = def_out;
	r->reg = reg;
	r->irqs = irqs;

	if (dev)
		ret = devm_gpiochip_add_data(dev, &r->gc, r);
	else
		ret = gpiochip_add_data(&r->gc, r);

	return ret ? ERR_PTR(ret) : &r->gc;
}

int gpio_reg_resume(struct gpio_chip *gc)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);
	unsigned long flags;

	spin_lock_irqsave(&r->lock, flags);
	writel_relaxed(r->out, r->reg);
	spin_unlock_irqrestore(&r->lock, flags);

	return 0;
}

static int enobu_gpioreg_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret = 0;
        
        dev_info(dev, "eNOBU GPIO registers initialized\n");
        return ret;
}

static int enobu_gpioreg_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret = 0;
        
        dev_info(dev, "eNOBU GPIO registers removed\n");
        return ret;
}

static const struct of_device_id enobu_gpioreg_of_match[] = {
        { .compatible = "leonardo,enobu-fpga-gpioreg" },
        { }
};
MODULE_DEVICE_TABLE(of, enobu_gpioreg_of_match);

static struct platform_driver enobu_gpioreg_driver = {
        .probe = enobu_gpioreg_probe,
        .remove = enobu_gpioreg_remove,
        .driver = {
                .name = "enobu-fpga-gpioreg",
                .of_match_table = enobu_gpioreg_of_match,
        },
};

module_platform_driver(enobu_gpioreg_driver);

MODULE_AUTHOR("Davide Ballestrero, Leonardo");
MODULE_DESCRIPTION("Driver for eNOBU FPGA GPIO registers");
MODULE_LICENSE("GPL");
