/** @file
  Tegra Timer Protocol driver of the ARM flavor

  Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>

  SPDX-License-Identifier: GPL-2.0+

**/
#ifndef _TEGRA_TIMER_H_
#define _TEGRA_TIMER_H_

#include <Device/Tegra.h>

// Tegra Clock Registers
#define TEGRA_OSC_CLK_ENB_L_SET		(NV_PA_CLK_RST_BASE + 0x320)
#define TEGRA_OSC_SET_CLK_ENB_TMR	(1 << 5)

// Tegra Timer Base Address
#define TIMER_TMR1_TMR_PTV_0      0x60005000
#define TIMER_TMR1_TMR_PCR_0      (TIMER_TMR1_TMR_PTV_0 + 0x4)

// Tegra Timer Registers
#define TEGRA_TIMER_USEC_CNTR     NV_PA_TMRUS_BASE
#define TEGRA_TIMER_USEC_CFG		  (NV_PA_TMRUS_BASE + 0x4)

#endif /* _TEGRA_TIMER_H_ */