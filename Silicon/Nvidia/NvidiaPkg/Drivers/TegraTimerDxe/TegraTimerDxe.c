/** @file
  Tegra Timer Protocol driver of the ARM flavor

  Copyright (c) 2011-2021, Arm Limited. All rights reserved.<BR>
  Copyright (C) 2022 Svyatoslav Ryhel <clamor95@gmail.com>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiDxe.h>

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/TegraClockLib.h>

#include <Protocol/Timer.h>
#include <Protocol/HardwareInterrupt.h>

#include "TegraTimer.h"

// The notification function to call on every timer interrupt.
EFI_TIMER_NOTIFY  mTimerNotifyFunction     = (EFI_TIMER_NOTIFY)NULL;
EFI_EVENT         EfiExitBootServicesEvent = (EFI_EVENT)NULL;

// The Current Clock/Timer Rate
UINT64 ClockRate = 0;

// The current period of the timer interrupt
UINT64 mTimerPeriod = 0;

// The latest Timer Tick calculated for mTimerPeriod
UINT64 mTimerTicks = 0;

// Number of elapsed period since the last Timer interrupt
UINT64 mElapsedPeriod = 1;

// Cached copy of the Hardware Interrupt protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL *gInterrupt = NULL;

/**
  This function registers the handler NotifyFunction so it is called every time
  the timer interrupt fires.  It also passes the amount of time since the last
  handler call to the NotifyFunction.  If NotifyFunction is NULL, then the
  handler is unregistered.  If the handler is registered, then EFI_SUCCESS is
  returned.  If the CPU does not support registering a timer interrupt handler,
  then EFI_UNSUPPORTED is returned.  If an attempt is made to register a handler
  when a handler is already registered, then EFI_ALREADY_STARTED is returned.
  If an attempt is made to unregister a handler when a handler is not registered,
  then EFI_INVALID_PARAMETER is returned.  If an error occurs attempting to
  register the NotifyFunction with the timer interrupt, then EFI_DEVICE_ERROR
  is returned.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.
  @param  NotifyFunction   The function to call when a timer interrupt fires. This
                           function executes at TPL_HIGH_LEVEL. The DXE Core will
                           register a handler for the timer interrupt, so it can know
                           how much time has passed. This information is used to
                           signal timer based events. NULL will unregister the handler.
  @retval EFI_SUCCESS           The timer handler was registered.
  @retval EFI_UNSUPPORTED       The platform does not support timer interrupts.
  @retval EFI_ALREADY_STARTED   NotifyFunction is not NULL, and a handler is already
                                registered.
  @retval EFI_INVALID_PARAMETER NotifyFunction is NULL, and a handler was not
                                previously registered.
  @retval EFI_DEVICE_ERROR      The timer handler could not be registered.

**/
EFI_STATUS
EFIAPI
TimerDriverRegisterHandler (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN EFI_TIMER_NOTIFY         NotifyFunction
  )
{
  if ((NotifyFunction == NULL) && (mTimerNotifyFunction == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((NotifyFunction != NULL) && (mTimerNotifyFunction != NULL)) {
    return EFI_ALREADY_STARTED;
  }

  mTimerNotifyFunction = NotifyFunction;

  return EFI_SUCCESS;
}

/**
    Disable the timer
**/
VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (0 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));
}

/**

  This function adjusts the period of timer interrupts to the value specified
  by TimerPeriod.  If the timer period is updated, then the selected timer
  period is stored in EFI_TIMER.TimerPeriod, and EFI_SUCCESS is returned.  If
  the timer hardware is not programmable, then EFI_UNSUPPORTED is returned.
  If an error occurs while attempting to update the timer period, then the
  timer hardware will be put back in its state prior to this call, and
  EFI_DEVICE_ERROR is returned.  If TimerPeriod is 0, then the timer interrupt
  is disabled.  This is not the same as disabling the CPU's interrupts.
  Instead, it must either turn off the timer hardware, or it must adjust the
  interrupt controller so that a CPU interrupt is not generated when the timer
  interrupt fires.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.
  @param  TimerPeriod      The rate to program the timer interrupt in 100 nS units. If
                           the timer hardware is not programmable, then EFI_UNSUPPORTED is
                           returned. If the timer is programmable, then the timer period
                           will be rounded up to the nearest timer period that is supported
                           by the timer hardware. If TimerPeriod is set to 0, then the
                           timer interrupts will be disabled.


  @retval EFI_SUCCESS           The timer period was changed.
  @retval EFI_UNSUPPORTED       The platform cannot change the period of the timer interrupt.
  @retval EFI_DEVICE_ERROR      The timer period could not be changed due to a device error.

**/
EFI_STATUS
EFIAPI
TimerDriverSetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                   TimerPeriod
  )
{
  UINT32   CounterValue;
  UINT64   TimerTicks;
  EFI_TPL  OriginalTPL;

  // Always disable the timer
  MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (0 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));

  if (TimerPeriod != 0) {
    // mTimerTicks = TimerPeriod in 1ms unit x Frequency.10^-3
    //             = TimerPeriod.10^-4 x Frequency.10^-3
    //             = (TimerPeriod x Frequency) x 10^-7
    TimerTicks = MultU64x32 (TimerPeriod, ClockRate);
    TimerTicks = DivU64x32 (TimerTicks, 10000000U);

    // Raise TPL to update the mTimerTicks and mTimerPeriod to ensure these values
    // are coherent in the interrupt handler
    OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

    mTimerTicks    = TimerTicks;
    mTimerPeriod   = TimerPeriod;
    mElapsedPeriod = 1;

    gBS->RestoreTPL (OriginalTPL);

    // Get value of the current timer
    CounterValue = MmioRead32 (TEGRA_TIMER_USEC_CNTR);

    // Set the interrupt in Current Time + mTimerTick
    MmioWrite32 (TIMER_TMR1_TMR_PCR_0 & 0x1FFFFFFF, CounterValue + mTimerTicks);

    // Enable the timer
    MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (1 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));
  } else {
    // Save the new timer period
    mTimerPeriod = TimerPeriod;

    // Reset the elapsed period
    mElapsedPeriod = 1;
  }

  return EFI_SUCCESS;
}

/**
  This function retrieves the period of timer interrupts in 100 ns units,
  returns that value in TimerPeriod, and returns EFI_SUCCESS.  If TimerPeriod
  is NULL, then EFI_INVALID_PARAMETER is returned.  If a TimerPeriod of 0 is
  returned, then the timer is currently disabled.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.
  @param  TimerPeriod      A pointer to the timer period to retrieve in 100 ns units. If
                           0 is returned, then the timer is currently disabled.


  @retval EFI_SUCCESS           The timer period was returned in TimerPeriod.
  @retval EFI_INVALID_PARAMETER TimerPeriod is NULL.

**/
EFI_STATUS
EFIAPI
TimerDriverGetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  OUT UINT64                  *TimerPeriod
  )
{
  if (TimerPeriod == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *TimerPeriod = mTimerPeriod;
  return EFI_SUCCESS;
}

/**
  This function generates a soft timer interrupt. If the platform does not support soft
  timer interrupts, then EFI_UNSUPPORTED is returned. Otherwise, EFI_SUCCESS is returned.
  If a handler has been registered through the EFI_TIMER_ARCH_PROTOCOL.RegisterHandler()
  service, then a soft timer interrupt will be generated. If the timer interrupt is
  enabled when this service is called, then the registered handler will be invoked. The
  registered handler should not be able to distinguish a hardware-generated timer
  interrupt from a software-generated timer interrupt.

  @param  This             The EFI_TIMER_ARCH_PROTOCOL instance.

  @retval EFI_SUCCESS           The soft timer interrupt was generated.
  @retval EFI_UNSUPPORTED       The platform does not support the generation of soft timer interrupts.

**/
EFI_STATUS
EFIAPI
TimerDriverGenerateSoftInterrupt (
  IN EFI_TIMER_ARCH_PROTOCOL  *This
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Interface structure for the Timer Architectural Protocol.

  @par Protocol Description:
  This protocol provides the services to initialize a periodic timer
  interrupt, and to register a handler that is called each time the timer
  interrupt fires.  It may also provide a service to adjust the rate of the
  periodic timer interrupt.  When a timer interrupt occurs, the handler is
  passed the amount of time that has passed since the previous timer
  interrupt.

  @param RegisterHandler
  Registers a handler that will be called each time the
  timer interrupt fires.  TimerPeriod defines the minimum
  time between timer interrupts, so TimerPeriod will also
  be the minimum time between calls to the registered
  handler.

  @param SetTimerPeriod
  Sets the period of the timer interrupt in 100 nS units.
  This function is optional, and may return EFI_UNSUPPORTED.
  If this function is supported, then the timer period will
  be rounded up to the nearest supported timer period.


  @param GetTimerPeriod
  Retrieves the period of the timer interrupt in 100 nS units.

  @param GenerateSoftInterrupt
  Generates a soft timer interrupt that simulates the firing of
  the timer interrupt. This service can be used to invoke the   registered handler if the timer interrupt has been masked for
  a period of time.

**/
EFI_TIMER_ARCH_PROTOCOL gTegraTimer = {
  TimerDriverRegisterHandler,
  TimerDriverSetTimerPeriod,
  TimerDriverGetTimerPeriod,
  TimerDriverGenerateSoftInterrupt
};

/**

  C Interrupt Handler called in the interrupt context when Source interrupt is active.


  @param Source         Source of the interrupt. Hardware routing off a specific platform defines
                        what source means.

  @param SystemContext  Pointer to system register context. Mostly used by debuggers and will
                        update the system context after the return from the interrupt if
                        modified. Don't change these values unless you know what you are doing

**/
VOID
EFIAPI
TimerInterruptHandler (
  IN  HARDWARE_INTERRUPT_SOURCE  Source,
  IN  EFI_SYSTEM_CONTEXT         SystemContext
  )
{
  EFI_TPL  OriginalTPL;
  UINT32   CurrentValue;
  UINT64   CompareValue;

  //
  // DXE core uses this callback for the EFI timer tick. The DXE core uses locks
  // that raise to TPL_HIGH and then restore back to current level. Thus we need
  // to make sure TPL level is set to TPL_HIGH while we are handling the timer tick.
  //
  OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  // Signal end of interrupt early to help avoid losing subsequent ticks
  // from long duration handlers
  gInterrupt->EndOfInterrupt (gInterrupt, Source);

  // Check if the timer interrupt is active
  if (MmioRead32 (TIMER_TMR1_TMR_PTV_0) == (1 << 31)) {
    if (mTimerNotifyFunction != 0) {
      mTimerNotifyFunction (mTimerPeriod * mElapsedPeriod);
    }

    //
    // Reload the Timer
    //

    // Get current counter value
    CurrentValue = MmioRead32 (TEGRA_TIMER_USEC_CNTR);

    // Get the counter value to compare with
    CompareValue = MmioRead32 (TIMER_TMR1_TMR_PCR_0) & 0x1FFFFFFF;

    // This loop is needed in case we missed interrupts (eg: case when the interrupt handling
    // has taken longer than mTickPeriod).
    // Note: Physical Counter is counting up
    mElapsedPeriod = 0;
    do {
      CompareValue += mTimerTicks;
      mElapsedPeriod++;
    } while (CompareValue < CurrentValue);

    // Set next compare value
    MmioWrite32 (TIMER_TMR1_TMR_PCR_0 & 0x1FFFFFFF, CompareValue);
    MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (0 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));
    MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (1 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));
    ArmInstructionSynchronizationBarrier ();
  }

  gBS->RestoreTPL (OriginalTPL);
}

/**
  Initialize the state information for the Timer Architectural Protocol and
  the Timer Debug support protocol that allows the debugger to break into a
  running program.

  @param  ImageHandle   of the loaded driver
  @param  SystemTable   Pointer to the System Table

  @retval EFI_SUCCESS           Protocol registered
  @retval EFI_OUT_OF_RESOURCES  Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR      Hardware problems

**/
EFI_STATUS
EFIAPI
TimerInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HANDLE  Handle          = NULL;
  EFI_STATUS  Status          = EFI_SUCCESS;
  UINT32      TimerHypIntrNum = 0;
  UINT32      UsecConfig      = 0;
  UINT32      Value           = 0;

  // Find the interrupt controller protocol. ASSERT if not found.
  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&gInterrupt);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Locate GIC Protocol!\n"));
    ASSERT_EFI_ERROR (Status);
  }

  // Disable the timer
  MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (0 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));

  // Install secure and Non-secure interrupt handlers
  // Note: Because it is not possible to determine the security state of the
  // CPU dynamically, we just install interrupt handler for both sec and non-sec
  // timer PPI
  Status = gInterrupt->RegisterInterruptSource (gInterrupt, PcdGet32 (PcdTegraTimerVirtIntrNum), TimerInterruptHandler);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Register Virtual Timer Interrupt!\n"));
    ASSERT_EFI_ERROR (Status);
  }

  //
  // The hypervisor timer interrupt may be omitted by implementations that
  // execute under virtualization.
  //
  TimerHypIntrNum = PcdGet32 (PcdTegraTimerHypIntrNum);
  if (TimerHypIntrNum != 0) {
    Status = gInterrupt->RegisterInterruptSource (gInterrupt, TimerHypIntrNum, TimerInterruptHandler);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Failed to Register Hypervisor Timer Interrupt!\n"));
      ASSERT_EFI_ERROR (Status);
    }
  }

  Status = gInterrupt->RegisterInterruptSource (gInterrupt, PcdGet32 (PcdTegraTimerSecIntrNum), TimerInterruptHandler);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Register Secondary Timer Interrupt!\n"));
    ASSERT_EFI_ERROR (Status);
  }

  Status = gInterrupt->RegisterInterruptSource (gInterrupt, PcdGet32 (PcdTegraTimerIntrNum), TimerInterruptHandler);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Register Main Timer Interrupt!\n"));
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Configure microsecond timers to have 1MHz clock
  // Config register is 0xqqww, where qq is "dividend", ww is "divisor"
  // Uses n+1 scheme
  //
  ClockRate = GetClockRate(CLOCK_ID_CLK_M);
  switch (ClockRate) {
    case 12000000:
      UsecConfig = 0x000b; // (11+1) / (0+1)
      break;
    case 12800000:
      UsecConfig = 0x043f; // (63+1) / (4+1)
      break;
    case 13000000:
      UsecConfig = 0x000c; // (12+1) / (0+1)
      break;
    case 16800000:
      UsecConfig = 0x0453; // (83+1) / (4+1)
      break;
    case 19200000:
      UsecConfig = 0x045f; // (95+1) / (4+1)
      break;
    case 26000000:
      UsecConfig = 0x0019; // (25+1) / (0+1)
      break;
    case 38400000:
      UsecConfig = 0x04bf; // (191+1) / (4+1)
      break;
    case 48000000:
      UsecConfig = 0x002f; // (47+1) / (0+1)
      break;
    default:
      DEBUG ((EFI_D_ERROR, "Recieved Invalid Clock Rate: %d\n", ClockRate));
      ASSERT_EFI_ERROR (EFI_UNSUPPORTED);
  }

  DEBUG ((EFI_D_WARN, "Clock Rate: %d\n", ClockRate));

  // Install the Tegra Timer Protocol onto a new handle
  Status = gBS->InstallMultipleProtocolInterfaces (&Handle, &gEfiTimerArchProtocolGuid, &gTegraTimer, NULL);
  ASSERT_EFI_ERROR (Status);

  // Configure Timer
  Value = MmioRead32(TEGRA_OSC_CLK_ENB_L_SET);
	MmioWrite32(TEGRA_OSC_CLK_ENB_L_SET, Value | TEGRA_OSC_SET_CLK_ENB_TMR);
	MmioWrite32(TEGRA_TIMER_USEC_CFG, UsecConfig);

  // Everything is ready, Enable Timer
  MmioWrite32 (TIMER_TMR1_TMR_PTV_0, (1 << 31) | ((0x69420 - 1) & 0x1FFFFFFF));

  // Register for an ExitBootServicesEvent
  Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES, TPL_NOTIFY, ExitBootServicesEvent, NULL, &EfiExitBootServicesEvent);
  ASSERT_EFI_ERROR (Status);

  return Status;
}