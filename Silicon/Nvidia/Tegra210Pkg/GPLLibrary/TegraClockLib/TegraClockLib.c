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

#include <Device/Tegra.h>

// CLK_RST_CONTROLLER_OSC_CTRL_0
#define OSC_FREQ_SHIFT  28
#define OSC_FREQ_MASK   (0xF << OSC_FREQ_SHIFT)

//
// PLL Divider Shift/Mask Tables for all PLL IDs.
//
ClockPllInfo PllInfoTable[CLOCK_ID_PLL_COUNT] = {
  //
  // NOTE: If kcp_mask/kvco_mask == 0, they're not used in that PLL (PLLC, etc.)
  //       If lock_ena or lock_det are >31, they're not used in that PLL (PLLC, etc.)
  //
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 10, .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F, .lock_ena = 32, .lock_det = 27, .kcp_shift = 0,  .kcp_mask = 0, .kvco_shift = 0,  .kvco_mask = 0 },  // PLLC
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F, .lock_ena = 4,  .lock_det = 27, .kcp_shift = 1,  .kcp_mask = 3, .kvco_shift = 0,  .kvco_mask = 1 },  // PLLM
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 10, .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F, .lock_ena = 18, .lock_det = 27, .kcp_shift = 0,  .kcp_mask = 3, .kvco_shift = 2,  .kvco_mask = 1 },  // PLLP
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F, .lock_ena = 28, .lock_det = 27, .kcp_shift = 25, .kcp_mask = 3, .kvco_shift = 24, .kvco_mask = 1 },  // PLLA
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 16, .p_mask = 0x1F, .lock_ena = 29, .lock_det = 27, .kcp_shift = 25, .kcp_mask = 3, .kvco_shift = 24, .kvco_mask = 1 },  // PLLU
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 11, .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x07, .lock_ena = 18, .lock_det = 27, .kcp_shift = 23, .kcp_mask = 3, .kvco_shift = 22, .kvco_mask = 1 },  // PLLD
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 20, .p_mask = 0x1F, .lock_ena = 18, .lock_det = 27, .kcp_shift = 1,  .kcp_mask = 3, .kvco_shift = 0,  .kvco_mask = 1 },  // PLLX
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 0,  .p_mask = 0,    .lock_ena = 9,  .lock_det = 11, .kcp_shift = 6,  .kcp_mask = 3, .kvco_shift = 0,  .kvco_mask = 1 },  // PLLE
  { .m_shift = 0, .m_mask = 0,    .n_shift = 0,  .n_mask = 0,    .p_shift = 0,  .p_mask = 0,    .lock_ena = 0,  .lock_det = 0,  .kcp_shift = 0,  .kcp_mask = 0, .kvco_shift = 0,  .kvco_mask = 0 },  // PLLS (Gone)
  { .m_shift = 0, .m_mask = 0xFF, .n_shift = 8,  .n_mask = 0xFF, .p_shift = 19, .p_mask = 0x1F, .lock_ena = 30, .lock_det = 27, .kcp_shift = 25, .kcp_mask = 3, .kvco_shift = 24, .kvco_mask = 1 }   // PLLDP
};

ClockPllInfo*
GetPllInfoTable (enum ClockID ClockID)
{
  return &PllInfoTable[ClockID];
}

enum OscClockFreq
GetOscClockFreq ()
{
  UINT32 Value;

  ClockResetController *ClockReset = (ClockResetController *)TEGRA_CLOCK_BASE;

  Value = MmioRead32 ((UINTN)&ClockReset->crc_osc_ctrl);
  Value = (Value & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;

  return Value << 2;
}
