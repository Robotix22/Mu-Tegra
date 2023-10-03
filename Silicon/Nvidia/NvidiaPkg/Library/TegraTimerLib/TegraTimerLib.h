/** @file
  Generic ARM implementation of Tegra Timer

  Copyright (c) 2011 - 2021, Arm Limited. All rights reserved.<BR>
  Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _TEGRA_TIMER_LIB_H_
#define _TEGRA_TIMER_LIB_H_

/* Tegra Timer Registers */
#define TEGRA_TIMER_USEC_CNTR       0x60005010

/* Raw Tegra Timer */
#define TEGRA_TIMER_RATE            1000000        /* 1 MHz */

#endif /* _TEGRA_TIMER_LIB_H_ */
