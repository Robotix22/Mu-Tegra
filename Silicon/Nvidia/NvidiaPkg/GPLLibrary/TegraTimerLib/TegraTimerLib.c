/** @file
  Generic ARM implementation of Tegra Timer

  Copyright (c) 2011 - 2021, Arm Limited. All rights reserved.<BR>
  Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>

  SPDX-License-Identifier: GPL-2.0+

**/
#include <Base.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>

#include "TegraTimerLib.h"

RETURN_STATUS
EFIAPI
TimerConstructor (
  VOID
  )
{
  return RETURN_SUCCESS;
}

/**
  Stalls the CPU for the number of microseconds specified by MicroSeconds.

  @param  MicroSeconds  The minimum number of microseconds to delay.

  @return The value of MicroSeconds input.

**/
UINTN
EFIAPI
MicroSecondDelay (
  IN      UINTN  MicroSeconds
  )
{
  UINT32  TimerTicks32;
  UINT32  SystemCounterVal;

  // Calculate counter ticks that represent requested delay:
  //  = MicroSeconds x TICKS_PER_MICRO_SEC
  //  = MicroSeconds x Frequency.10^-6
  TimerTicks32 = DivU64x32 (MultU64x32 (MicroSeconds, TEGRA_TIMER_RATE), 1000000U);

  // Read System Counter value
  SystemCounterVal = MmioRead32 (TEGRA_TIMER_USEC_CNTR);

  TimerTicks32 += SystemCounterVal;

  // Wait until delay count expires.
  while (SystemCounterVal < TimerTicks32) {
    SystemCounterVal = MmioRead32 (TEGRA_TIMER_USEC_CNTR);
  }

  return MicroSeconds;
}

/**
  Stalls the CPU for at least the given number of nanoseconds.

  Stalls the CPU for the number of nanoseconds specified by NanoSeconds.

  When the timer frequency is 1MHz, each tick corresponds to 1 microsecond.
  Therefore, the nanosecond delay will be rounded up to the nearest 1 microsecond.

  @param  NanoSeconds The minimum number of nanoseconds to delay.

  @return The value of NanoSeconds inputted.

**/
UINTN
EFIAPI
NanoSecondDelay (
  IN  UINTN  NanoSeconds
  )
{
  UINTN  MicroSeconds;

  // Round up to 1us Tick Number
  MicroSeconds  = NanoSeconds / 1000;
  MicroSeconds += ((NanoSeconds % 1000) == 0) ? 0 : 1;

  MicroSecondDelay (MicroSeconds);

  return NanoSeconds;
}

/**
  Retrieves the current value of a 64-bit free running performance counter.

  The counter can either count up by 1 or count down by 1. If the physical
  performance counter counts by a larger increment, then the counter values
  must be translated. The properties of the counter can be retrieved from
  GetPerformanceCounterProperties().

  @return The current value of the free running performance counter.

**/
UINT64
EFIAPI
GetPerformanceCounter (
  VOID
  )
{
  return MmioRead64 (TEGRA_TIMER_USEC_CNTR);
}

/**
  Retrieves the 64-bit frequency in Hz and the range of performance counter
  values.

  If StartValue is not NULL, then the value that the performance counter starts
  with immediately after is it rolls over is returned in StartValue. If
  EndValue is not NULL, then the value that the performance counter end with
  immediately before it rolls over is returned in EndValue. The 64-bit
  frequency of the performance counter in Hz is always returned. If StartValue
  is less than EndValue, then the performance counter counts up. If StartValue
  is greater than EndValue, then the performance counter counts down. For
  example, a 64-bit free running counter that counts up would have a StartValue
  of 0 and an EndValue of 0xFFFFFFFFFFFFFFFF. A 24-bit free running counter
  that counts down would have a StartValue of 0xFFFFFF and an EndValue of 0.

  @param  StartValue  The value the performance counter starts with when it
                      rolls over.
  @param  EndValue    The value that the performance counter ends with before
                      it rolls over.

  @return The frequency in Hz.

**/
UINT64
EFIAPI
GetPerformanceCounterProperties (
  OUT      UINT64  *StartValue   OPTIONAL,
  OUT      UINT64  *EndValue     OPTIONAL
  )
{
  if (StartValue != NULL) {
    // Timer starts at 0
    *StartValue = (UINT64)0ULL;
  }

  if (EndValue != NULL) {
    // Timer counts up.
    *EndValue = 0xFFFFFFFFUL;
  }

  return (UINT64)TEGRA_TIMER_RATE;
}

/**
  Converts elapsed ticks of performance counter to time in nanoseconds.

  This function converts the elapsed ticks of running performance counter to
  time value in unit of nanoseconds.

  @param  Ticks     The number of elapsed ticks of running performance counter.

  @return The elapsed time in nanoseconds.

**/
UINT64
EFIAPI
GetTimeInNanoSecond (
  IN      UINT64  Ticks
  )
{
  UINT64  NanoSeconds;
  UINT32  Remainder;
  UINT32  TimerFreq;

  TimerFreq = MmioRead32 (TEGRA_TIMER_USEC_CNTR);

  //
  //          Ticks
  // Time = --------- x 1,000,000,000
  //        Frequency
  //
  NanoSeconds = MultU64x32 (DivU64x32Remainder (Ticks, TimerFreq, &Remainder), 1000000000U);

  //
  // Frequency < 0x100000000, so Remainder < 0x100000000, then (Remainder * 1,000,000,000)
  // will not overflow 64-bit.
  //
  NanoSeconds += DivU64x32 (MultU64x32 ((UINT64)Remainder, 1000000000U), TimerFreq);

  return NanoSeconds;
}