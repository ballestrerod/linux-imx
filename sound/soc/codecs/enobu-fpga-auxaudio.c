/*
 * This is a simple driver for the enobu_fpga_auxaudio Voice PCM interface
 *
 * Copyright (C) 2015 Goldelico GmbH
 *
 * Author: Marek Belisko <marek@goldelico.com>
 *
 * Based on wm8727.c driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/ac97_codec.h>
#include <sound/initval.h>
#include <sound/soc.h>

static const struct snd_soc_dapm_widget enobu_fpga_auxaudio_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("AOUT"),
	SND_SOC_DAPM_INPUT("AIN"),
};

static const struct snd_soc_dapm_route enobu_fpga_auxaudio_dapm_routes[] = {
	{ "AOUT", NULL, "Playback" },
	{ "Capture", NULL, "AIN" },
};

static struct snd_soc_dai_driver enobu_fpga_auxaudio_dai = {
	.name = "enobu-fpga-aux",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
};

static const struct snd_soc_component_driver soc_component_dev_enobu_fpga_auxaudio = {
	.dapm_widgets		= enobu_fpga_auxaudio_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(enobu_fpga_auxaudio_dapm_widgets),
	.dapm_routes		= enobu_fpga_auxaudio_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(enobu_fpga_auxaudio_dapm_routes),
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	.non_legacy_dai_naming	= 1,
};

static int enobu_fpga_auxaudio_platform_probe(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev,
			&soc_component_dev_enobu_fpga_auxaudio, &enobu_fpga_auxaudio_dai, 1);
}

#if defined(CONFIG_OF)
static const struct of_device_id enobu_fpga_auxaudio_codec_of_match[] = {
	{ .compatible = "leonardo,enobu-fpga-auxaudio", },
	{},
};
MODULE_DEVICE_TABLE(of, enobu_fpga_auxaudio_codec_of_match);
#endif

static struct platform_driver enobu_fpga_auxaudio_codec_driver = {
	.driver = {
		.name = "enobu-fpga-auxaudio",
		.of_match_table = of_match_ptr(enobu_fpga_auxaudio_codec_of_match),
	},
	.probe = enobu_fpga_auxaudio_platform_probe,
};

module_platform_driver(enobu_fpga_auxaudio_codec_driver);

MODULE_DESCRIPTION("ASoC eNOBU FPGA Aux Audio driver");
MODULE_AUTHOR("Davide Ballestrero <davide.ballestrero@leonardocompany.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:enobu-fpga-auxaudio");
