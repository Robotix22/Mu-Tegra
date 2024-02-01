/** @file
  Tegra Clock Driver.

  Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.

  SPDX-License-Identifier: GPL-2.0
**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TegraClockLib.h>
#include <Library/IoLib.h>
#include <Library/UefiDriverEntryPoint.h>

#include <Device/Tegra.h>

// Hmm
#define do_div(n,base) ({					\
	UINT32 __base = (base);				\
	UINT32 __rem;						\
	__rem = ((UINT64)(n)) % __base;			\
	(n) = ((UINT64)(n)) / __base;				\
	__rem;							\
 })

STATIC UINTN PllRate[CLOCK_ID_COUNT];

STATIC UINTN OscFreq[CLOCK_OSC_FREQ_COUNT] = {
  13000000, 16800000,
  0,        0,
  19200000, 38400000,
  0,        0,
  12000000, 48000000,
  0,        0,
  26000000,
};

STATIC
ClockPll*
GetPll (enum ClockID ClockID)
{
  ClockResetController *ClockReset = (ClockResetController *)TEGRA_CLOCK_BASE;

  ASSERT (clock_id_is_pll(ClockID));

  if (ClockID >= (enum ClockID)TEGRA_CLK_PLLS) {
    DEBUG ((EFI_D_ERROR, "Invalid PLL!\n"));
    return NULL;
  }

  return &ClockReset->crc_pll[ClockID];
}

// TODO: Move this Function to the Library Part.
SimpleClockPll*
GetSimplePll (enum ClockID ClockID)
{
  ClockResetController *ClockReset = (ClockResetController *)TEGRA_CLOCK_BASE;

  switch (ClockID) {
    case CLOCK_ID_XCPU:
    case CLOCK_ID_EPCI:
    case CLOCK_ID_SFROM32KHZ:
      return &ClockReset->crc_pll_simple[ClockID - CLOCK_ID_FIRST_SIMPLE];

    default:
      return NULL;
  }
}

EFI_STATUS
GetClockRate (
  IN  enum ClockID ClockID,
  OUT UINTN       *ClockRate)
{
  UINT64 ParentRate;
  UINT32 Base;
  UINT32 Divm;

  ClockPll       *Pll       = NULL;
  SimpleClockPll *SimplePll = NULL;
  ClockPllInfo   *PllInfo   = GetPllInfoTable (ClockID);

  ParentRate = OscFreq[GetOscClockFreq ()];

  // NOTE: CLOCK_ID_CLK_M might need its own Case
  if (ClockID == CLOCK_ID_OSC || ClockID == CLOCK_ID_CLK_M) {
    *ClockRate = ParentRate;
    return EFI_SUCCESS;
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

  *ClockRate = ParentRate * ((Base >> PllInfo->n_shift) & PllInfo->n_mask);
  Divm       = (Base >> PllInfo->m_shift) & PllInfo->m_mask;

  // TODO: Make this Tegra210 only.
  if (ClockID != CLOCK_ID_PERIPH) {
    Divm <<= (Base >> PllInfo->p_shift) & PllInfo->p_mask;
  }

  do_div (*ClockRate, Divm);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitClocks (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

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
  //Status |= GetClockRate (CLOCK_ID_DISPLAY2, PllRate[CLOCK_ID_DISPLAY2]);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Get Clock Rates!\n"));
    ASSERT_EFI_ERROR (Status);
  }

  // Print Clock Rates
  DEBUG ((EFI_D_WARN, "PLLC = %d Khz\n", PllRate[CLOCK_ID_CGENERAL]));
  DEBUG ((EFI_D_WARN, "PLLM = %d Khz\n", PllRate[CLOCK_ID_MEMORY]));
  DEBUG ((EFI_D_WARN, "PLLP = %d Khz\n", PllRate[CLOCK_ID_PERIPH]));
  DEBUG ((EFI_D_WARN, "PLLU = %d Khz\n", PllRate[CLOCK_ID_USB]));
  DEBUG ((EFI_D_WARN, "PLLD = %d Khz\n", PllRate[CLOCK_ID_DISPLAY]));
  DEBUG ((EFI_D_WARN, "PLLX = %d Khz\n", PllRate[CLOCK_ID_XCPU]));
  DEBUG ((EFI_D_WARN, "Osc  = %d Khz\n", PllRate[CLOCK_ID_OSC]));
  DEBUG ((EFI_D_WARN, "CLKM = %d Khz\n", PllRate[CLOCK_ID_CLK_M]));

  // TODO: Add Real Clock Init here.

  return EFI_SUCCESS;
}
