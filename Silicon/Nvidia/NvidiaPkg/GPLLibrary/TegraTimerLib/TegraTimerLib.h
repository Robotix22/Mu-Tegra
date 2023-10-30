/** @file
  Generic ARM implementation of Tegra Timer

  Copyright (c) 2011 - 2021, Arm Limited. All rights reserved.<BR>
  Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>

  SPDX-License-Identifier: GPL-2.0+

**/
#ifndef _TEGRA_TIMER_LIB_H_
#define _TEGRA_TIMER_LIB_H_

#include <Device/Tegra.h>

// Tegra Timer Registers
#define TEGRA_TIMER_USEC_CNTR       NV_PA_TMRUS_BASE

// Raw Tegra Timer
#define TEGRA_TIMER_RATE            1000000                // 1 MHz

#endif /* _TEGRA_TIMER_LIB_H_ */