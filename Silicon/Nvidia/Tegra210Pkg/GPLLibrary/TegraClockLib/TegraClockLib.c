/** @file
  Tegra Clock Library.

  Copyright (c) 2011 The Chromium OS Authors.
  Copyright (C) 2010-2015 NVIDIA Corporation <www.nvidia.com>

  SPDX-License-Identifier: GPL-2.0+
**/

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TegraClockLib.h>

// CLK_RST_CONTROLLER_OSC_CTRL_0
#define OSC_FREQ_SHIFT  28
#define OSC_FREQ_MASK   (0xF << OSC_FREQ_SHIFT)

enum OscClockFreq
GetOscClockFreq ()
{
  UINT32 Value;

  ClockResetController *ClockReset = (ClockResetController *)TEGRA_CLOCK_BASE;

  Value = MmioRead32 ((UINTN)&ClockReset->crc_osc_ctrl);
  Value = (Value & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;

  return Value << 2;
}
