/*
 * Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0
 */
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/TegraClockLib.h>

#include <Device/Tegra.h>
#include <Device/Clock.h>

// CLK_RST_CONTROLLER_OSC_CTRL_0
#define OSC_FREQ_SHIFT          28
#define OSC_FREQ_MASK           (0xF << OSC_FREQ_SHIFT)

//
// Get the oscillator frequency, from the corresponding hardware configuration
// field. Note that T30+ supports 3 new higher freqs.
//
enum ClockOscFreq GetOscClockFreq()
{
  UINT32 Reg = 0;

  struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;

  Reg = MmioRead32((UINTN)&clkrst->crc_osc_ctrl);
  return (Reg & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
}

unsigned GetClockRate(enum clock_id clkid)
{
  UINT64 ParentClockRate = 0;
  UINT64 ClockRate       = 0;

  // Get Clock Oscillator Freq
  ParentClockRate = OscFreq[GetOscClockFreq()];
  DEBUG ((EFI_D_WARN, "Parent Clock Rate: %d\n", ParentClockRate));

  if (clkid == CLOCK_ID_OSC || clkid == CLOCK_ID_CLK_M) { return ParentClockRate; }

  // TODO: Add all other Clock Stuff

  return ClockRate;
}