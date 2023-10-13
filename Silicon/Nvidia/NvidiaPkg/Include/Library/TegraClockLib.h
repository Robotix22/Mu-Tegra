/*
 * Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
#ifndef _TEGRA_CLOCK_LIB_H_
#define _TEGRA_CLOCK_LIB_H_

// The PLLs supported by the hardware
enum clock_id {
  CLOCK_ID_FIRST,
  CLOCK_ID_CGENERAL = CLOCK_ID_FIRST,
  CLOCK_ID_MEMORY,
  CLOCK_ID_PERIPH,
  CLOCK_ID_AUDIO,
  CLOCK_ID_USB,
  CLOCK_ID_DISPLAY,

  // now the simple ones
  CLOCK_ID_FIRST_SIMPLE,
  CLOCK_ID_XCPU = CLOCK_ID_FIRST_SIMPLE,
  CLOCK_ID_EPCI,
  CLOCK_ID_SFROM32KHZ,

  // These are the base clocks (inputs to the Tegra SOC)
  CLOCK_ID_32KHZ,
  CLOCK_ID_OSC,
  CLOCK_ID_CLK_M,

  CLOCK_ID_COUNT,                     // number of PLLs
  CLOCK_ID_DISPLAY2,                  // Placeholder
  CLOCK_ID_NONE = -1,
};

/**
 * Returns the clock rate of a specified clock, in Hz.
 *
 * @param parent	PLL id of clock to check
 * Return: rate of clock in Hz
 */
unsigned GetClockRate(enum clock_id clkid);

// return the current oscillator clock frequency
enum ClockOscFreq GetOscClockFreq(VOID);

#endif /* _TEGRA_CLOCK_LIB_H_ */