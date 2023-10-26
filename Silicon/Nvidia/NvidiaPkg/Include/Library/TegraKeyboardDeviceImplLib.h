#ifndef __LIBRARY_TEGRA_KEYBOARD_DEVICE_IMPL_H__
#define __LIBRARY_TEGRA_KEYBOARD_DEVICE_IMPL_H__

#include <Protocol/TegraKeyboardDevice.h>

EFI_STATUS EFIAPI TegraKeyboardDeviceImplReset(KEYBOARD_DEVICE_PROTOCOL *This);
EFI_STATUS        TegraKeyboardDeviceImplGetKeys(
           KEYBOARD_DEVICE_PROTOCOL *This, KEYBOARD_RETURN_API *KeyboardReturnApi,
           UINT64 Delta);


typedef enum {
  KEYSTATE_RELEASED,
  KEYSTATE_PRESSED,
  KEYSTATE_LONGPRESS_RELEASE,
} KEY_STATE;

typedef struct {
  // keydata to be send to the driver
  EFI_KEY_DATA KeyData;

  // the time the current action has been running
  UINT64 Time;

  // the current state of the key
  KEY_STATE State;

  // the current key has to repeat sending events
  BOOLEAN Repeat;

  // the user did a longpress
  BOOLEAN Longpress;
} KEY_CONTEXT;

#endif