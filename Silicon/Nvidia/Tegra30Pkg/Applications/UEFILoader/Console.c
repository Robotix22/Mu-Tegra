// Copyright (c) 2019 - 2020, Bingxing Wang and other project authors. All rights reserved.<BR>
// Copyright (c) 2021 - 2021, Leander Wollersberger. All rights reserved.<BR>

#include "UEFILoader.h"

#include <Library/UefiRuntimeServicesTableLib.h>

EFI_STATUS
FixConsoleOutput()
{
  EFI_STATUS                       Status                   = EFI_SUCCESS;
  UINTN                            NumOutputProtocolHandles = 0;
  EFI_HANDLE                      *mOutputHandles;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *mSimpleTextOutputProtocol;

  // Locate Handle Buffers of STO Protocol
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleTextOutProtocolGuid, NULL, &NumOutputProtocolHandles, &mOutputHandles);

  // Some sanity check here.
  if (!EFI_ERROR(Status) && NumOutputProtocolHandles >= 3) {
    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Located Handle Buffers of STO Protocol.\n", __FUNCTION__));

    // Take the last handle as ConOut
    Status = gBS->HandleProtocol(mOutputHandles[NumOutputProtocolHandles - 1], &gEfiSimpleTextOutProtocolGuid, (VOID **)&mSimpleTextOutputProtocol);

    if (!EFI_ERROR(Status)) {
      DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Located ConOut Handle Protocol.\n", __FUNCTION__));

      // Hack: Force ConOut to use Display
      gST->ConOut           = mSimpleTextOutputProtocol;
      gST->ConsoleOutHandle = mOutputHandles[NumOutputProtocolHandles - 1];
      
      // Clear Screen
      gST->ConOut->ClearScreen (gST->ConOut);
    } else {
      DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Locate ConOut Handle Protocol! Status = %r\n", __FUNCTION__, Status));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Locate Handle Buffer of STO Protocol! Status = %r\n", __FUNCTION__, Status));
  }

  return Status;
}