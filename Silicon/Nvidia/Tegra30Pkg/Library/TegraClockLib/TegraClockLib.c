/*
 * Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/TegraClockLib.h>

#include <Device/Tegra.h>
#include <Device/Clock.h>

#include "TegraClock.h"

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

UINT64
GetClockRate ()
{
  UINT64 ParentClockRate = 0;

  // Get Clock Oscillator Freq
  ParentClockRate = OscFreq[GetOscClockFreq()];
  DEBUG ((EFI_D_WARN, "Parent Clock Rate: %d\n", ParentClockRate));

  // TODO: Add all other Clock Stuff

  return 0;
}