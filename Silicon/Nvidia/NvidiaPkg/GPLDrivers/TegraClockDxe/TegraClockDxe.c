/** @file
  Tegra Clock Driver.

  Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.

  SPDX-License-Identifier: GPL-2.0
**/

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/TegraClockLib.h>

STATIC UINTN PllRate[CLOCK_ID_COUNT];

STATIC UINTN OscFreq[CLOCK_OSC_FREQ_COUNT] = {
  13000000, 16800000,
  0,        0,
  19200000, 8400000,
  0,        0,
  12000000, 48000000,
  0,        0,
  26000000
};

STATIC
EFI_STATUS
GetClockRate (
  enum ClockID ClockID,
  UINTN        ClockRate)
{
  UINT64 ParentRate;
  UINT64 Rate;
  UINT32 Base;
  UINT32 Divm;

  ClockPll       *Pll       = NULL;
  SimpleClockPll *SimplePll = NULL;
  ClockPllInfo   *PllInfo   = &PllInfoTable[ClockID];

  ParentRate = OscFreq[GetOscClockFreq ()];
  if (ClockID == CLOCK_ID_OSC) {
    return ParentRate;
  } else if (ClockID == CLOCK_ID_CLK_M) {
    return (UINT32)ParentRate;
  }

  if (ClockID < (enum ClockID)TEGRA_CLK_PLLS) {
    Pll = GetPll (ClockID);
  } else {
    SimplePll = GetSimplePll (ClockID);
  }

  if (!SimplePll && !Pll) {
    DEBUG ((EFI_D_ERROR, "Unknown PLL ID %d\n", ClockID));
    return EFI_UNSUPPORTED;
  }

  if (Pll) {
    Base = MmioRead32 ((UINTN)&Pll->PllBase);
  } else {
    Base = MmioRead32 ((UINTN)&SimplePll->PllBase);
  }

  Rate = ParentRate * ((Base >> PllInfo->NShift) & PllInfo->NMask);
  Divm = (Base >> PllInfo->MShift) & PllInfo->MMask;
}

EFI_STATUS
EFIAPI
InitClocks (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable)
{
  // Get Clock Rates
  Status  = GetClockRate (CLOCK_ID_CGENERAL, &PllRate[CLOCK_ID_CGENERAL]);
  Status |= GetClockRate (CLOCK_ID_MEMORY,   &PllRate[CLOCK_ID_MEMORY]);
  Status |= GetClockRate (CLOCK_ID_PERIPH,   &PllRate[CLOCK_ID_PERIPH]);
  Status |= GetClockRate (CLOCK_ID_USB,      &PllRate[CLOCK_ID_USB]);
  Status |= GetClockRate (CLOCK_ID_DISPLAY,  &PllRate[CLOCK_ID_DISPLAY]);
  Status |= GetClockRate (CLOCK_ID_XCPU,     &PllRate[CLOCK_ID_XCPU]);
  Status |= GetClockRate (CLOCK_ID_OSC,      &PllRate[CLOCK_ID_OSC]);
  Status |= GetClockRate (CLOCK_ID_CLK_M,    &PllRate[CLOCK_ID_CLK_M]);

  // For Tegra 20 only.
  //Status |= GetClockRate (CLOCK_ID_DISPLAY2, &PllRate[CLOCK_ID_DISPLAY2]);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Get Clock Rates!\n"));
    ASSERT_EFI_ERROR (Status);
  }

  // Print Clock Rates
  DEBUG ((EFI_D_WARN, "PLLC = %d\n", PllRate[CLOCK_ID_CGENERAL]));
  DEBUG ((EFI_D_WARN, "PLLM = %d\n", PllRate[CLOCK_ID_MEMORY]));
  DEBUG ((EFI_D_WARN, "PLLP = %d\n", PllRate[CLOCK_ID_PERIPH]));
  DEBUG ((EFI_D_WARN, "PLLU = %d\n", PllRate[CLOCK_ID_USB]));
  DEBUG ((EFI_D_WARN, "PLLD = %d\n", PllRate[CLOCK_ID_DISPLAY]));
  DEBUG ((EFI_D_WARN, "PLLX = %d\n", PllRate[CLOCK_ID_XCPU]));
  DEBUG ((EFI_D_WARN, "Osc  = %d\n", PllRate[CLOCK_ID_OSC]));
  DEBUG ((EFI_D_WARN, "CLKM = %d\n", PllRate[CLOCK_ID_CLK_M]));

  // For Tegra 20 only.
  //DEBUG ((EFI_D_WARN, "PLLC = %d\n", PllRate[CLOCK_ID_DISPLAY2]));

Loop:
  goto Loop;

  return EFI_SUCCESS;
}
