#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

#include <Library/MemoryMapHelperLib.h>
#include <Library/PlatformPrePiLib.h>
#include <Library/DevicePrePiLib.h>

#include "InitializationUtils.h"

VOID InitializeSharedUartBuffers(VOID)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX DisplayMemoryRegion;
  LocateMemoryMapAreaByName("Display Reserved", &DisplayMemoryRegion);

  // Clear Screen
  ZeroMem((VOID *)DisplayMemoryRegion.Address, DisplayMemoryRegion.Length);

  // Setup Position counter
  INTN *pFbConPosition =
      (INTN
           *)(DisplayMemoryRegion.Address + (FixedPcdGet32(PcdMipiFrameBufferWidth) * FixedPcdGet32(PcdMipiFrameBufferHeight) * FixedPcdGet32(PcdMipiFrameBufferPixelBpp) / 8));

  *(pFbConPosition + 0) = 0;
  *(pFbConPosition + 1) = 0;
}

VOID UartInit(VOID)
{
  SerialPortInitialize();
  InitializeSharedUartBuffers();

  DEBUG((EFI_D_INFO, "\nProject Mu on %a (Arm)\n", (VOID *)FixedPcdGetPtr(PcdSmbiosSystemModel)));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

VOID EarlyInitialization(VOID)
{
  // Do platform specific initialization here
  PlatformInitialize();

  // Do Device specific initialization here
  DeviceInitialize();

  // Initialize UART Serial
  UartInit();
}
