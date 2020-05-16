#include <linux/module.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/gpio-reg.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/fpga/machxo-efb.h>


struct enobu_gpioreg {
	struct gpio_chip gc;
	spinlock_t lock;
	u8 direction;
	u8 out;
	u16 fpga_reg;
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
	u8 val, mask = BIT(offset);

	spin_lock_irqsave(&r->lock, flags);
	val = r->out;
	if (value)
		val |= mask;
	else
		val &= ~mask;
	r->out = val;
	// CHG writel_relaxed(val, r->reg);
        efb_spi_write(r->fpga_reg, val);
	spin_unlock_irqrestore(&r->lock, flags);
}

static int enobu_gpioreg_get(struct gpio_chip *gc, unsigned offset)
{
	struct enobu_gpioreg *r = to_enobu_gpioreg(gc);
	u8 val, mask = BIT(offset);

	if (r->direction & mask) {
		/*
		 * double-read the value, some registers latch after the
		 * first read.
		 */
		// read readl_relaxed(r->reg);
		// val = readl_relaxed(r->reg);
                efb_spi_read(r->fpga_reg, &val);
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
	// CHG writel_relaxed(r->out, r->reg);
        efb_spi_write(r->fpga_reg, r->out);
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
struct gpio_chip *enobu_gpioreg_init(struct device *dev, u16 fpga_reg,
	int base, int num, const char *label, u8 direction, u8 def_out,
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
	r->fpga_reg = fpga_reg;
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
	// writel_relaxed(r->out, r->reg);
	spin_unlock_irqrestore(&r->lock, flags);

	return 0;
}







static const char *m2_slot1_names[] = {
	"sl1_reset", "sl1_w_disable1", "sl1_fullcard_poweroff",
};

#define ENOBU_FPGA_M2_SL1               0x00
#define ENOBU_FPGA_M2_SL1_RST           (1<<0)
#define ENOBU_FPGA_M2_SL1_DIS           (1<<1)
#define ENOBU_FPGA_M2_SL1_POFF          (1<<2)

static const char *m2_slot2_names[] = {
	"sl2_reset", "sl2_w_disable1", "sl2_fullcard_poweroff",
};


#define ENOBU_FPGA_M2_SL2               0x01
#define ENOBU_FPGA_M2_SL2_RST           (1<<0)
#define ENOBU_FPGA_M2_SL2_DIS           (1<<1)
#define ENOBU_FPGA_M2_SL2_POFF          (1<<2)

static const char *usb_hub_names[] = {
	"hub_ctl1", "hub_ctl2", "hub_ctl3", "hub_ctl4",
        NULL, NULL, "hub_reset", "hub_vbus", 
};

#define ENOBU_FPGA_USB_HUB              0x02
#define ENOBU_FPGA_USB_HUB_CTL1         (1<<0)
#define ENOBU_FPGA_USB_HUB_CTL2         (1<<1)
#define ENOBU_FPGA_USB_HUB_CTL3         (1<<2)
#define ENOBU_FPGA_USB_HUB_CTL4         (1<<3)
#define ENOBU_FPGA_USB_HUB_RST          (1<<6)
#define ENOBU_FPGA_USB_HUB_VBUS         (1<<7)

#define ENOBU_FPGA_USB_HUB_DEFAULT \
	(ENOBU_FPGA_USB_HUB_CTL1  | ENOBU_FPGA_USB_HUB_CTL2 | \
	 ENOBU_FPGA_USB_HUB_CTL3 | ENOBU_FPGA_USB_HUB_CTL4 )

static const char *uart_comm_names[] = {
	NULL, "fast_mode", "loopback",
};

#define ENOBU_FPGA_UART_COMM            0x03
#define ENOBU_FPGA_UART_COMM_FASTM      (1<<1)
#define ENOBU_FPGA_UART_COMM_LOOPB      (1<<2)

static const char *uart2_conf_names[] = {
	"uart2_term", "uart2_mode", "uart2_txena", "uart2_rxena",
        NULL, NULL, NULL, "uart2_duplex",
};

#define ENOBU_FPGA_UART2_CONF           0x04
#define ENOBU_FPGA_UART2_CONF_TERM      (1<<0)
#define ENOBU_FPGA_UART2_CONF_MODE      (1<<1)
#define ENOBU_FPGA_UART2_CONF_TXEN      (1<<2)
#define ENOBU_FPGA_UART2_CONF_RXEN      (1<<2)
#define ENOBU_FPGA_UART2_CONF_DUPL      (1<<7)

static const char *uart3_conf_names[] = {
	"uart3_term", "uart3_mode", "uart3_txena", "uart3_rxena",
        NULL, NULL, NULL, "uart3_duplex",
};

#define ENOBU_FPGA_UART3_CONF           0x05
#define ENOBU_FPGA_UART3_CONF_TERM      (1<<0)
#define ENOBU_FPGA_UART3_CONF_MODE      (1<<1)
#define ENOBU_FPGA_UART3_CONF_TXEN      (1<<2)
#define ENOBU_FPGA_UART3_CONF_RXEN      (1<<2)
#define ENOBU_FPGA_UART3_CONF_DUPL      (1<<7)

static const char *digital_out_names[] = {
	"out1", "out2", "out3",
};

#define ENOBU_FPGA_DOUT       0x09
#define ENOBU_FPGA_DOUT1      (1<<0)
#define ENOBU_FPGA_DOUT2      (1<<1)
#define ENOBU_FPGA_DOUT3      (1<<2)

static const char *digital_inp_names[] = {
	"inp1", "inp2", "inp3", "inp4",
        "inp5", "inp6", "inp7"
};

#define ENOBU_FPGA_DINP       0x0a
#define ENOBU_FPGA_DINP1      (1<<0)
#define ENOBU_FPGA_DINP2      (1<<1)
#define ENOBU_FPGA_DINP3      (1<<2)
#define ENOBU_FPGA_DINP4      (1<<3)
#define ENOBU_FPGA_DINP5      (1<<4)
#define ENOBU_FPGA_DINP6      (1<<5)
#define ENOBU_FPGA_DINP7      (1<<6)

static const char *reset_dev_names[] = {
	"reset_pcie", "reset_hd",
};

#define ENOBU_FPGA_RSTDEV       0x16
#define ENOBU_FPGA_RSTDEV_PCIE  (1<<0)
#define ENOBU_FPGA_RSTDEV_HD    (1<<1)



static int enobu_gpioreg_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	// struct gpio_chip *gc;
	int ret = 0;
	u8 def_val = 0;

	enobu_gpioreg_init(dev, ENOBU_FPGA_M2_SL1, -1, 3, "m2_slot1", 0,
                           def_val, m2_slot1_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_M2_SL2, -1, 3, "m2_slot2", 0,
                           def_val, m2_slot2_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_USB_HUB, -1, 8, "usb_hub", 0,
                           ENOBU_FPGA_USB_HUB_DEFAULT, usb_hub_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_UART_COMM, -1, 3, "uart_common", 0,
                           def_val, uart_comm_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_UART2_CONF, -1, 8, "uart2_conf", 0,
                           def_val, uart2_conf_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_UART3_CONF, -1, 8, "uart3_conf", 0,
                           def_val, uart3_conf_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_DOUT, -1, 3, "digital_out", 0,
                           def_val, digital_out_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_DINP, -1, 7, "digital_inp", 0x7f,
                           def_val, digital_inp_names, NULL, NULL);

	enobu_gpioreg_init(dev, ENOBU_FPGA_RSTDEV, -1, 2, "reset_dev", 0,
                           def_val, reset_dev_names, NULL, NULL);

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
