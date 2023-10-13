/*
 * Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef _TEGRA_CLOCK_LIB_H_
#define _TEGRA_CLOCK_LIB_H_

// Get the Clock Rate
UINT64 GetClockRate(VOID);

// return the current oscillator clock frequency
enum ClockOscFreq GetOscClockFreq(VOID);

#endif /* _TEGRA_CLOCK_LIB_H_ */