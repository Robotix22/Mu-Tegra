// Taken from https://github.com/NekomimiRouter/yahallo/blob/main/TegraFirmwareDump/Console.c
#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "UEFILoader.h"

VOID
ConsoleOutputFixup(VOID)
{
  EFI_STATUS                       Status                   = EFI_SUCCESS;
  UINTN                            NumOutputProtocolHandles = 0;
  EFI_HANDLE                      *pOutputHandles           = NULL;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *pScreenEfiOutputProtocol = NULL;

  // Locate Simple Text Out Handle
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleTextOutProtocolGuid, NULL, &NumOutputProtocolHandles, &pOutputHandles);
  if (!EFI_ERROR(Status) && NumOutputProtocolHandles >= 3) {
    // Get Hadle Protocol
    Status = gBS->HandleProtocol(pOutputHandles[NumOutputProtocolHandles - 1], &gEfiSimpleTextOutProtocolGuid, (VOID **)&pScreenEfiOutputProtocol);

    // Hack: Force the Screen to Use ConOut
    if (!EFI_ERROR(Status)) {
      gST->ConOut           = pScreenEfiOutputProtocol;
      gST->ConsoleOutHandle = pOutputHandles[NumOutputProtocolHandles - 1];
    } else {
      DEBUG ((EFI_D_ERROR, "Failed to Get and Set Simple Text Out Handle!\n"));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "Failed to Locate Simple Text Out Handle!\n"));
  }
}
