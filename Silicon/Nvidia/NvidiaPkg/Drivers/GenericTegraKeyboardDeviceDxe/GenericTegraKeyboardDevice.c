#include <PiDxe.h>

#include <Library/DebugLib.h>
#include <Library/TegraKeyboardDeviceImplLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/TegraKeyboardDevice.h>

#include <Configuration/BootDevices.h>

STATIC KEYBOARD_DEVICE_PROTOCOL mInternalKeyboardDevice = {
  TegraKeyboardDeviceImplReset,
  TegraKeyboardDeviceImplGetKeys,
};

EFI_STATUS
EFIAPI
TegraKeyboardDeviceDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  Status = gBS->InstallMultipleProtocolInterfaces(&ImageHandle, &gTegraKeyboardDeviceProtocolGuid, &mInternalKeyboardDevice, &gEfiDevicePathProtocolGuid, &TegraKeyboardDevicePath, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Install Keyboard Device Protocol!\n"));
    ASSERT_EFI_ERROR(Status);
  }

  return Status;
}