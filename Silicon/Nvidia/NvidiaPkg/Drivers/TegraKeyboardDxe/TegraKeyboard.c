/** @file
  Keyboard driver. Routines that interacts with callers, conforming to EFI driver model.

  Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license may
  be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#include "TegraKeyboard.h"

#include <Configuration/BootDevices.h>

//
// Function prototypes
//
/**
  Test controller is a keyboard Controller.

  @param This                 Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller           driver's controller
  @param RemainingDevicePath  children device path

  @retval EFI_UNSUPPORTED     controller is not floppy disk
  @retval EFI_SUCCESS         controller is floppy disk
**/
EFI_STATUS
EFIAPI
KeyboardControllerDriverSupported(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

/**
  Create KEYBOARD_CONSOLE_IN_DEV instance on controller.

  @param This                Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller          driver controller handle
  @param RemainingDevicePath Children's device path

  @retval                    whether success to create floppy control instance.
**/
EFI_STATUS
EFIAPI
KeyboardControllerDriverStart(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device
**/
EFI_STATUS
EFIAPI
KeyboardControllerDriverStop(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                  *ChildHandleBuffer
  );

/**
  Free the waiting key notify list.

  @param ListHead  Pointer to list head

  @retval EFI_INVALID_PARAMETER  ListHead is NULL
  @retval EFI_SUCCESS            Sucess to free NotifyList
**/
EFI_STATUS
KbdFreeNotifyList(IN OUT LIST_ENTRY *ListHead);

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gKeyboardControllerDriver = {
  KeyboardControllerDriverSupported,
  KeyboardControllerDriverStart,
  KeyboardControllerDriverStop,
  0xa,
  NULL,
  NULL
};

/**
  Test controller is a keyboard Controller.

  @param This                 Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller           driver's controller
  @param RemainingDevicePath  children device path

  @retval EFI_UNSUPPORTED     controller is not floppy disk
  @retval EFI_SUCCESS         controller is floppy disk
**/
EFI_STATUS
EFIAPI
KeyboardControllerDriverSupported(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath)
{
  EFI_STATUS                Status;
  KEYBOARD_DEVICE_PROTOCOL *KeyboardDevice;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol(Controller, &gTegraKeyboardDeviceProtocolGuid, (VOID **)&KeyboardDevice, This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol(Controller, &gTegraKeyboardDeviceProtocolGuid, This->DriverBindingHandle, Controller);

  return Status;
}

STATIC
EFIAPI
VOID
KeyboardReturnApiPushEfikeyBufTail(
  KEYBOARD_RETURN_API *This,
  EFI_KEY_DATA        *KeyData)
{
  KEYBOARD_CONSOLE_IN_DEV       *ConsoleIn;
  LIST_ENTRY                  *Link;
  KEYBOARD_CONSOLE_IN_EX_NOTIFY *CurrentNotify;

  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_KEYBOARD_RETURN_API(This);

  //
  // If the key can not be converted then just return.
  //
  if (KeyData->Key.ScanCode == SCAN_NULL && KeyData->Key.UnicodeChar == CHAR_NULL) {
    if (!ConsoleIn->IsSupportPartialKey) {
      DEBUG ((EFI_D_WARN, "Cannot convert this Keycode.\n"));
      return;
    }
  }

  //
  // Signal KeyNotify process event if this key pressed matches any key
  // registered.
  //
  for (Link = GetFirstNode(&ConsoleIn->NotifyList); !IsNull(&ConsoleIn->NotifyList, Link); Link = GetNextNode(&ConsoleIn->NotifyList, Link)) {
    CurrentNotify = CR(Link, KEYBOARD_CONSOLE_IN_EX_NOTIFY, NotifyEntry, KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE);
    if (IsKeyRegistered(&CurrentNotify->KeyData, KeyData)) {
      //
      // The key notification function needs to run at TPL_CALLBACK
      // while current TPL is TPL_NOTIFY. It will be invoked in
      // KeyNotifyProcessHandler() which runs at TPL_CALLBACK.
      //
      PushEfikeyBufTail(&ConsoleIn->EfiKeyQueueForNotify, KeyData);
      gBS->SignalEvent(ConsoleIn->KeyNotifyProcessEvent);
    }
  }

  PushEfikeyBufTail(&ConsoleIn->EfiKeyQueue, KeyData);
}

/**
  Create KEYBOARD_CONSOLE_IN_DEV instance on controller.

  @param This                Pointer of EFI_DRIVER_BINDING_PROTOCOL
  @param Controller          driver controller handle
  @param RemainingDevicePath Children's device path

  @retval                    whether success to create floppy control instance.
**/
EFI_STATUS
EFIAPI
KeyboardControllerDriverStart(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath)
{
  EFI_STATUS                Status;
  KEYBOARD_DEVICE_PROTOCOL *KeyboardDevice;
  KEYBOARD_CONSOLE_IN_DEV  *ConsoleIn;

  //
  // Get the Keyboard Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol(Controller, &gTegraKeyboardDeviceProtocolGuid, (VOID **)&KeyboardDevice, This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Open Keyboard Protocol! Status = %r\n", Status));
    return Status;
  }

  //
  // Allocate private data
  //
  ConsoleIn = AllocateZeroPool(sizeof(KEYBOARD_CONSOLE_IN_DEV));
  if (ConsoleIn == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Failed to Allocate Private Data! Status = %r\n", Status));
    goto exit;
  }

  //
  // Setup the device instance
  //
  ConsoleIn->Signature                           = KEYBOARD_CONSOLE_IN_DEV_SIGNATURE;
  ConsoleIn->Handle                              = Controller;
  (ConsoleIn->ConIn).Reset                       = KeyboardEfiReset;
  (ConsoleIn->ConIn).ReadKeyStroke               = KeyboardReadKeyStroke;
  ConsoleIn->KeyboardDevice                      = KeyboardDevice;
  ConsoleIn->KeyboardReturnApi.PushEfikeyBufTail = KeyboardReturnApiPushEfikeyBufTail;
  ConsoleIn->Last                                = (UINT64)-1;
  ConsoleIn->ConInEx.Reset                       = KeyboardEfiResetEx;
  ConsoleIn->ConInEx.ReadKeyStrokeEx             = KeyboardReadKeyStrokeEx;
  ConsoleIn->ConInEx.SetState                    = KeyboardSetState;
  ConsoleIn->ConInEx.RegisterKeyNotify           = KeyboardRegisterKeyNotify;
  ConsoleIn->ConInEx.UnregisterKeyNotify         = KeyboardUnregisterKeyNotify;

  InitializeListHead(&ConsoleIn->NotifyList);

  //
  // Fix for random hangs in System waiting for the Key if no KBC is present in
  // BIOS. When KBC decode (IO port 0x60/0x64 decode) is not enabled, Keyboard Read
  // will read back as 0xFF and return status is EFI_SUCCESS. So instead we read
  // status register to detect after read if KBC decode is enabled.
  //

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent(EVT_NOTIFY_WAIT, TPL_NOTIFY, KeyboardWaitForKey, ConsoleIn, &((ConsoleIn->ConIn).WaitForKey));
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Failed to Create WaitForKey Event! Status = %r\n", Status));
    goto exit;
  }

  //
  // Setup the WaitForKeyEx event
  //
  Status = gBS->CreateEvent(EVT_NOTIFY_WAIT, TPL_NOTIFY, KeyboardWaitForKeyEx, ConsoleIn, &(ConsoleIn->ConInEx.WaitForKeyEx));
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Failed to Create WaitForKeyEx Event! Status = %r\n", Status));
    goto exit;
  }

  //
  // Setup a periodic timer, used for reading keystrokes at a fixed interval
  //
  Status = gBS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, KeyboardTimerHandler, ConsoleIn, &ConsoleIn->TimerEvent);
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Failed to Create Periodic Timer Event! Status = %r\n", Status));
    goto exit;
  }

  Status = gBS->SetTimer(ConsoleIn->TimerEvent, TimerPeriodic, KEYBOARD_TIMER_INTERVAL);
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Failed to Set Timer State! Status = %r\n", Status));
    goto exit;
  }

  Status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, KeyNotifyProcessHandler, ConsoleIn, &ConsoleIn->KeyNotifyProcessEvent);
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "Failed to Create KeyNotify Event! Status = %r\n", Status));
    goto exit;
  }

  //
  // Reset the keyboard device
  //
  Status = ConsoleIn->ConInEx.Reset(&ConsoleIn->ConInEx, FALSE);
  if (EFI_ERROR(Status)) {
    Status = EFI_DEVICE_ERROR;
    DEBUG ((EFI_D_ERROR, "Failed to Reset Keyboard Device! Status = %r\n", Status));
    goto exit;
  }

  ConsoleIn->ControllerNameTable = NULL;
  AddUnicodeString2("eng", gKeyboardComponentName.SupportedLanguages, &ConsoleIn->ControllerNameTable, L"Keyboard Device", TRUE);
  AddUnicodeString2("en", gKeyboardComponentName2.SupportedLanguages, &ConsoleIn->ControllerNameTable, L"Keyboard Device", FALSE);

  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces(&Controller, &gEfiSimpleTextInProtocolGuid, &ConsoleIn->ConIn, &gEfiSimpleTextInputExProtocolGuid, &ConsoleIn->ConInEx, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Install Keyboard Protocol! Status = %r\n", Status));
    goto exit;
  }

  return Status;

exit:
  if ((ConsoleIn != NULL) && (ConsoleIn->ConIn.WaitForKey != NULL)) {
    gBS->CloseEvent(ConsoleIn->ConIn.WaitForKey);
  } else if ((ConsoleIn != NULL) && (ConsoleIn->TimerEvent != NULL)) {
    gBS->CloseEvent(ConsoleIn->TimerEvent);
  } else if ((ConsoleIn != NULL) && (ConsoleIn->ConInEx.WaitForKeyEx != NULL)) {
    gBS->CloseEvent(ConsoleIn->ConInEx.WaitForKeyEx);
  } else if ((ConsoleIn != NULL) && (ConsoleIn->KeyNotifyProcessEvent != NULL)) {
    gBS->CloseEvent(ConsoleIn->KeyNotifyProcessEvent);
  }

  KbdFreeNotifyList(&ConsoleIn->NotifyList);

  if ((ConsoleIn != NULL) && (ConsoleIn->ControllerNameTable != NULL)) {
    FreeUnicodeStringTable(ConsoleIn->ControllerNameTable);
  } else if (ConsoleIn != NULL) {
    gBS->FreePool(ConsoleIn);
  }

  gBS->CloseProtocol(Controller, &gTegraKeyboardDeviceProtocolGuid, This->DriverBindingHandle, Controller);

  return Status;
}

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device
**/
EFI_STATUS
EFIAPI
KeyboardControllerDriverStop(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                  *ChildHandleBuffer)
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
  KEYBOARD_CONSOLE_IN_DEV        *ConsoleIn;

  //
  // Disable Keyboard
  //
  Status = gBS->OpenProtocol(Controller, &gEfiSimpleTextInProtocolGuid, (VOID **)&ConIn, This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Open STI Protocol! Status = %r\n", Status));
    return Status;
  }

  Status = gBS->OpenProtocol(Controller, &gEfiSimpleTextInputExProtocolGuid, NULL, This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to COpen STIEx Protocol! Status = %r\n", Status));
    return Status;
  }

  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_THIS(ConIn);

  if (ConsoleIn->TimerEvent != NULL) {
    gBS->CloseEvent(ConsoleIn->TimerEvent);
    ConsoleIn->TimerEvent = NULL;
  }

  //
  // Uninstall the SimpleTextIn and SimpleTextInEx protocols
  //
  Status = gBS->UninstallMultipleProtocolInterfaces(Controller, &gEfiSimpleTextInProtocolGuid, &ConsoleIn->ConIn, &gEfiSimpleTextInputExProtocolGuid, &ConsoleIn->ConInEx, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Remove Keyboard Protocol! Status = %r\n", Status));
    return Status;
  }

  gBS->CloseProtocol(Controller, &gTegraKeyboardDeviceProtocolGuid, This->DriverBindingHandle, Controller);

  //
  // Free other resources
  //
  if ((ConsoleIn->ConIn).WaitForKey != NULL) {
    gBS->CloseEvent((ConsoleIn->ConIn).WaitForKey);
    (ConsoleIn->ConIn).WaitForKey = NULL;
  } else if (ConsoleIn->ConInEx.WaitForKeyEx != NULL) {
    gBS->CloseEvent(ConsoleIn->ConInEx.WaitForKeyEx);
    ConsoleIn->ConInEx.WaitForKeyEx = NULL;
  } else if (ConsoleIn->KeyNotifyProcessEvent != NULL) {
    gBS->CloseEvent(ConsoleIn->KeyNotifyProcessEvent);
    ConsoleIn->KeyNotifyProcessEvent = NULL;
  }

  KbdFreeNotifyList(&ConsoleIn->NotifyList);
  FreeUnicodeStringTable(ConsoleIn->ControllerNameTable);
  gBS->FreePool(ConsoleIn);

  return EFI_SUCCESS;
}

/**
  Free the waiting key notify list.

  @param ListHead                Pointer to list head

  @retval EFI_INVALID_PARAMETER  ListHead is NULL
  @retval EFI_SUCCESS            Sucess to free NotifyList
**/
EFI_STATUS
KbdFreeNotifyList(IN OUT LIST_ENTRY *ListHead)
{
  KEYBOARD_CONSOLE_IN_EX_NOTIFY *NotifyNode;

  if (ListHead == NULL) {
    DEBUG ((EFI_D_ERROR, "ListHead is NULL!\n"));
    return EFI_INVALID_PARAMETER;
  }

  while (!IsListEmpty(ListHead)) {
    NotifyNode = CR(ListHead->ForwardLink, KEYBOARD_CONSOLE_IN_EX_NOTIFY, NotifyEntry, KEYBOARD_CONSOLE_IN_EX_NOTIFY_SIGNATURE);
    RemoveEntryList(ListHead->ForwardLink);
    gBS->FreePool(NotifyNode);
  }

  return EFI_SUCCESS;
}

/**
  The module Entry Point for module Keyboard.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
InitializeKeyboard(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2(ImageHandle, SystemTable, &gKeyboardControllerDriver, ImageHandle, &gKeyboardComponentName, &gKeyboardComponentName2);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Install Driver Model Protocols! Status = %r\n", Status));
    ASSERT_EFI_ERROR(Status);
  }

  return Status;
}