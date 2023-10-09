/** @file
  Tegra Timer Protocol driver of the ARM flavor

  Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _TEGRA_TIMER_H_
#define _TEGRA_TIMER_H_

#include <Device/Tegra.h>

// Tegra Clock Registers
#define TEGRA_OSC_CLK_ENB_L_SET		(NV_PA_CLK_RST_BASE + 0x320)
#define TEGRA_OSC_SET_CLK_ENB_TMR	0x5

// Tegra Timer Registers
#define TEGRA_TIMER_USEC_CFG		(NV_PA_TMRUS_BASE + 4)

#endif /* _TEGRA_TIMER_H_ */