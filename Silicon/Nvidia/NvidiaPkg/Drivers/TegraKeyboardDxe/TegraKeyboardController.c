/** @file
  Routines that talk to the Keyboard Device protocol.

  Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license may
  be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "TegraKeyboard.h"

/**
  Display error message.

  @param ConsoleIn Pointer to instance of KEYBOARD_CONSOLE_IN_DEV
  @param ErrMsg    Unicode string of error message
**/
VOID
KeyboardError(
  IN KEYBOARD_CONSOLE_IN_DEV *ConsoleIn,
  IN CHAR16                *ErrMsg)
{
  ConsoleIn->KeyboardErr = TRUE;
}

/**
  Timer event handler: read a series of scancodes from 8042
  and put them into memory scancode buffer.
  it read as much scancodes to either fill
  the memory buffer or empty the keyboard buffer.
  It is registered as running under TPL_NOTIFY

  @param Event       The timer event
  @param Context     A KEYBOARD_CONSOLE_IN_DEV pointer
**/
VOID
EFIAPI
KeyboardTimerHandler(
  IN EFI_EVENT Event,
  IN VOID     *Context)
{
  EFI_TPL                  OldTpl;
  KEYBOARD_CONSOLE_IN_DEV *ConsoleIn;

  ConsoleIn = (KEYBOARD_CONSOLE_IN_DEV *)Context;

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL(TPL_NOTIFY);

  if (((KEYBOARD_CONSOLE_IN_DEV *)Context)->KeyboardErr) {
    //
    // Leave critical section and return
    //
    gBS->RestoreTPL(OldTpl);
    return;
  }

  UINT64 CurrentCounterValue = GetPerformanceCounter();
  UINT64 DeltaCounter        = CurrentCounterValue - ConsoleIn->Last;
  ConsoleIn->Last            = CurrentCounterValue;

  ConsoleIn->KeyboardDevice->GetKeys(ConsoleIn->KeyboardDevice, &ConsoleIn->KeyboardReturnApi, GetTimeInNanoSecond(DeltaCounter));

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL(OldTpl);
}

/**
  Perform 8042 controller and keyboard Initialization.
  If ExtendedVerification is TRUE, do additional test for
  the keyboard interface

  @param ConsoleIn            - KEYBOARD_CONSOLE_IN_DEV instance pointer
  @param ExtendedVerification - indicates a thorough initialization

  @retval EFI_DEVICE_ERROR      Fail to init keyboard
  @retval EFI_SUCCESS           Success to init keyboard
**/
EFI_STATUS
InitKeyboard(
  IN OUT KEYBOARD_CONSOLE_IN_DEV *ConsoleIn,
  IN BOOLEAN                      ExtendedVerification)
{
  EFI_STATUS Status = EFI_SUCCESS;

  ConsoleIn->KeyboardDevice->Reset(ConsoleIn->KeyboardDevice);

  //
  // Clear Memory Scancode Buffer
  //
  ConsoleIn->EfiKeyQueue.Head          = 0;
  ConsoleIn->EfiKeyQueue.Tail          = 0;
  ConsoleIn->EfiKeyQueueForNotify.Head = 0;
  ConsoleIn->EfiKeyQueueForNotify.Tail = 0;

  //
  // Reset the status indicators
  //
  ConsoleIn->CapsLock   = FALSE;
  ConsoleIn->NumLock    = FALSE;
  ConsoleIn->ScrollLock = FALSE;
  ConsoleIn->LeftCtrl   = FALSE;
  ConsoleIn->RightCtrl  = FALSE;
  ConsoleIn->LeftAlt    = FALSE;
  ConsoleIn->RightAlt   = FALSE;
  ConsoleIn->LeftShift  = FALSE;
  ConsoleIn->RightShift = FALSE;
  ConsoleIn->LeftLogo   = FALSE;
  ConsoleIn->RightLogo  = FALSE;
  ConsoleIn->Menu       = FALSE;
  ConsoleIn->SysReq     = FALSE;

  ConsoleIn->IsSupportPartialKey = FALSE;

  return Status;
}