/*
 * Copyright 2017 NXP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/of.h>
#include <linux/of_address.h>
#ifdef CONFIG_ARCH_MXC_ARM64
#include <soc/imx8/sc/sci.h>
#endif
#include "fec.h"

#define PHY_ID_AR8031   0x004dd074

#define PHY_ID_ADIN1300			0x0283bc30
#define ADIN1300_MII_EXT_REG_PTR	0x0010
#define ADIN1300_MII_EXT_REG_DATA	0x0011
#define ADIN1300_GE_RGMII_CFG_REG	0xff23
#define ADIN1300_GE_RGMII_RXID_EN	BIT(2)
#define ADIN1300_GE_RGMII_TXID_EN	BIT(1)
#define ADIN1300_GE_RGMII_EN		BIT(0)

#define IMX8QM_FUSE_MAC0_WORD0		452
#define IMX8QM_FUSE_MAC0_WORD1		453
#define IMX8QM_FUSE_MAC1_WORD0		454
#define IMX8QM_FUSE_MAC1_WORD1		455
#define IMX8QXP_FUSE_MAC0_WORD0		708
#define IMX8QXP_FUSE_MAC0_WORD1		709
#define IMX8QXP_FUSE_MAC1_WORD0		710
#define IMX8QXP_FUSE_MAC1_WORD1		711
#define IMX8M_OCOTP_MAC_ADDR0_OFF	0x640
#define IMX8M_OCOTP_MAC_ADDR1_OFF	0x650

enum imx_soc_type {
	IMX8QM_FUSE = 0,
	IMX8QXP_FUSE,
};

struct imx_fuse_mac_addr {
	u32 fuse_mac0_word0;
	u32 fuse_mac0_word1;
	u32 fuse_mac1_word0;
	u32 fuse_mac1_word1;
};

static struct imx_fuse_mac_addr imx8_fuse_mapping[] = {
	{
		.fuse_mac0_word0 = IMX8QM_FUSE_MAC0_WORD0,
		.fuse_mac0_word1 = IMX8QM_FUSE_MAC0_WORD1,
		.fuse_mac1_word0 = IMX8QM_FUSE_MAC1_WORD0,
		.fuse_mac1_word1 = IMX8QM_FUSE_MAC1_WORD1,
	}, {
		.fuse_mac0_word0 = IMX8QXP_FUSE_MAC0_WORD0,
		.fuse_mac0_word1 = IMX8QXP_FUSE_MAC0_WORD1,
		.fuse_mac1_word0 = IMX8QXP_FUSE_MAC1_WORD0,
		.fuse_mac1_word1 = IMX8QXP_FUSE_MAC1_WORD1,
	}, {
		/* sentinel */
	}
};

static int cpu_is_mx8mq(void)
{
	int ret;
	ret = of_machine_is_compatible("fsl,imx8mq");
	return ret;
}

static int ar8031_phy_fixup(struct phy_device *dev)
{
	u16 val;

	/* Set RGMII IO voltage to 1.8V */
	phy_write(dev, 0x1d, 0x1f);
	phy_write(dev, 0x1e, 0x8);

	/* Disable phy AR8031 SmartEEE function */
	phy_write(dev, 0xd, 0x3);
	phy_write(dev, 0xe, 0x805d);
	phy_write(dev, 0xd, 0x4003);
	val = phy_read(dev, 0xe);
	val &= ~(0x1 << 8);
	phy_write(dev, 0xe, val);

	/* Introduce tx clock delay */
	phy_write(dev, 0x1d, 0x5);
	phy_write(dev, 0x1e, 0x100);

	return 0;
}

static u16 adin_ext_read(struct phy_device *phydev, const u32 regnum) {
	u16 val;

	phy_write(phydev, ADIN1300_MII_EXT_REG_PTR, regnum);
	val = phy_read(phydev, ADIN1300_MII_EXT_REG_DATA);

	phydev_dbg(phydev, "adin_ext_read: adin1300@0x%x 0x%x=0x%x\n", phydev->mdio.addr, regnum, val);

	return val;
}

static int adin_ext_write(struct phy_device *phydev, const u32 regnum, const u16 val) {

	phydev_dbg(phydev, "adin_ext_write: adin1300@0x%x 0x%x=0x%x\n", phydev->mdio.addr, regnum, val);

	phy_write(phydev, ADIN1300_MII_EXT_REG_PTR, regnum);

	return phy_write(phydev, ADIN1300_MII_EXT_REG_DATA, val);
}


static int adin_config_rgmii_mode(struct phy_device *phydev)
{
	u16 val;

	val = adin_ext_read(phydev, ADIN1300_GE_RGMII_CFG_REG);

	if (!phy_interface_is_rgmii(phydev)) {
		/* Disable RGMII */
		val &= ~ADIN1300_GE_RGMII_EN;
		return adin_ext_write(phydev, ADIN1300_GE_RGMII_CFG_REG, val);
	}

	/* Enable RGMII */
	val |= ADIN1300_GE_RGMII_EN;

	/* Enable / Disable RGMII RX Delay */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
		val |= ADIN1300_GE_RGMII_RXID_EN;
	} else {
		val &= ~ADIN1300_GE_RGMII_RXID_EN;
	}

	/* Enable / Disable RGMII RX Delay */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
		val |= ADIN1300_GE_RGMII_TXID_EN;
	} else {
		val &= ~ADIN1300_GE_RGMII_TXID_EN;
	}

	return adin_ext_write(phydev, ADIN1300_GE_RGMII_CFG_REG, val);
}

static int adin1300_phy_fixup(struct phy_device *dev)
{
	dev_info(&dev->mdio.dev, "adin1300@0x%x\n", dev->mdio.addr);
	return adin_config_rgmii_mode(dev);
}

void fec_enet_register_fixup(struct net_device *ndev)
{
	struct fec_enet_private *fep = netdev_priv(ndev);
	static int adin1300_registered = 0;
	int err;

	if (!IS_BUILTIN(CONFIG_PHYLIB))
		return;

	if (!adin1300_registered && cpu_is_mx8mq()) {
		err = phy_register_fixup_for_uid(PHY_ID_ADIN1300, 0xffffffff,
							adin1300_phy_fixup);
		if (err)
			netdev_info(ndev, "Cannot register adin1300 PHY board fixup\n");
		adin1300_registered = 1;
	}

	if (fep->fixups & FEC_QUIRK_AR8031_FIXUP) {
		static int ar8031_registered = 0;

		if (ar8031_registered)
			return;
		err = phy_register_fixup_for_uid(PHY_ID_AR8031, 0xffffffef,
					ar8031_phy_fixup);
		if (err)
			netdev_info(ndev, "Cannot register PHY board fixup\n");
		ar8031_registered = 1;
	}
}

int of_fec_enet_parse_fixup(struct device_node *np)
{
	int fixups = 0;

	if (of_get_property(np, "fsl,ar8031-phy-fixup", NULL))
		fixups |= FEC_QUIRK_AR8031_FIXUP;

	return fixups;
}

static void imx8mq_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct device_node *ocotp_np;
	void __iomem *base;
	u32 value;

	ocotp_np = of_find_compatible_node(NULL, NULL, "fsl,imx7d-ocotp");
	if (!ocotp_np) {
		pr_warn("failed to find ocotp node\n");
		return;
	}

	base = of_iomap(ocotp_np, 0);
	if (!base) {
		pr_warn("failed to map ocotp\n");
		goto put_ocotp_node;
	}

	value = readl_relaxed(base + IMX8M_OCOTP_MAC_ADDR1_OFF);
	mac[0] = (value >> 8);
	mac[1] = value;

	value = readl_relaxed(base + IMX8M_OCOTP_MAC_ADDR0_OFF);
	mac[2] = value >> 24;
	mac[3] = value >> 16;
	mac[4] = value >> 8;
	mac[5] = value;

put_ocotp_node:
	of_node_put(ocotp_np);
}

#ifdef CONFIG_ARCH_MXC_ARM64
static void imx8qm_get_mac_from_fuse(int dev_id, unsigned char *mac,
				     struct imx_fuse_mac_addr *fuse_mapping)
{
	uint32_t mu_id;
	sc_ipc_t ipc_handle;
	sc_err_t sc_err = SC_ERR_NONE;
	uint32_t val1 = 0, val2 = 0;
	uint32_t word1, word2;

	sc_err = sc_ipc_getMuID(&mu_id);
	if (sc_err != SC_ERR_NONE) {
		pr_err("FEC MAC fuse: Get MU ID failed\n");
		return;
	}

	sc_err = sc_ipc_open(&ipc_handle, mu_id);
	if (sc_err != SC_ERR_NONE) {
		pr_err("FEC MAC fuse: Open MU channel failed\n");
		return;
	}

	if (dev_id == 0) {
		word1 = fuse_mapping->fuse_mac0_word0;
		word2 = fuse_mapping->fuse_mac0_word1;
	} else {
		word1 = fuse_mapping->fuse_mac1_word0;
		word2 = fuse_mapping->fuse_mac1_word1;
	}

	sc_err = sc_misc_otp_fuse_read(ipc_handle, word1, &val1);
	if (sc_err != SC_ERR_NONE) {
		pr_err("FEC MAC fuse %d read error: %d\n", word1, sc_err);
		sc_ipc_close(ipc_handle);
		return;
	}

	sc_err = sc_misc_otp_fuse_read(ipc_handle, word2, &val2);
	if (sc_err != SC_ERR_NONE) {
		pr_err("FEC MAC fuse %d read error: %d\n", word2, sc_err);
		sc_ipc_close(ipc_handle);
		return;
	}

	mac[0] = val1;
	mac[1] = val1 >> 8;
	mac[2] = val1 >> 16;
	mac[3] = val1 >> 24;
	mac[4] = val2;
	mac[5] = val2 >> 8;

	sc_ipc_close(ipc_handle);
}

static void imx8qm_ipg_stop_enable(int dev_id, bool enabled)
{
	uint32_t mu_id;
	sc_ipc_t ipc_handle;
	sc_err_t sc_err = SC_ERR_NONE;
	uint32_t rsrc_id, val;

	sc_err = sc_ipc_getMuID(&mu_id);
	if (sc_err != SC_ERR_NONE) {
		pr_err("FEC ipg stop: Get MU ID failed\n");
		return;
	}

	sc_err = sc_ipc_open(&ipc_handle, mu_id);
	if (sc_err != SC_ERR_NONE) {
		pr_err("FEC ipg stop: Open MU channel failed\n");
		return;
	}

	if (dev_id == 0)
		rsrc_id = SC_R_ENET_0;
	else
		rsrc_id = SC_R_ENET_1;

	val = enabled ? 1 : 0;
	sc_err = sc_misc_set_control(ipc_handle, rsrc_id, SC_C_IPG_STOP, val);
	if (sc_err != SC_ERR_NONE)
		pr_err("FEC ipg stop set error: %d\n", sc_err);

	sc_ipc_close(ipc_handle);
}
#else
static void imx8qm_get_mac_from_fuse(int dev_id, unsigned char *mac,
				     struct imx_fuse_mac_addr *fuse_mapping) {}
static void imx8qm_ipg_stop_enable(int dev_id, bool enabled) {}
#endif

void fec_enet_get_mac_from_fuse(struct device_node *np, unsigned char *mac)
{
	int idx;

	if (!np)
		return;

	idx = of_alias_get_id(np, "ethernet");
	if (idx < 0)
		idx = 0;

	if (of_machine_is_compatible("fsl,imx8qm"))
		imx8qm_get_mac_from_fuse(idx, mac,
					 &imx8_fuse_mapping[IMX8QM_FUSE]);
	else if (of_machine_is_compatible("fsl,imx8qxp"))
		imx8qm_get_mac_from_fuse(idx, mac,
					 &imx8_fuse_mapping[IMX8QXP_FUSE]);
	else if (of_machine_is_compatible("fsl,imx8mq") ||
		 of_machine_is_compatible("fsl,imx8mm"))
		imx8mq_get_mac_from_fuse(idx, mac);
}

void fec_enet_ipg_stop_misc_set(struct device_node *np, bool enabled)
{
	int idx;

	if (!np)
		return;

	idx = of_alias_get_id(np, "ethernet");
	if (idx < 0)
		idx = 0;

	if (of_machine_is_compatible("fsl,imx8qm") ||
	    of_machine_is_compatible("fsl,imx8qxp"))
		imx8qm_ipg_stop_enable(idx, enabled);
}
