#ifndef __PROTOCOL_TEGRA_KEYBOARD_DEVICE_H__
#define __PROTOCOL_TEGRA_KEYBOARD_DEVICE_H__

#include <Protocol/SimpleTextInEx.h>

#define KEYBOARD_DEVICE_PROTOCOL_GUID                                          \
  {                                                                            \
    0x657b03d1, 0x866c, 0x4f2f,                                                \
    {                                                                          \
      0x82, 0xe5, 0xb0, 0x22, 0x51, 0xc2, 0x94, 0xb2                           \
    }                                                                          \
  }

typedef struct _KEYBOARD_DEVICE_PROTOCOL KEYBOARD_DEVICE_PROTOCOL;
typedef struct _KEYBOARD_RETURN_API      KEYBOARD_RETURN_API;

typedef VOID(EFIAPI *PUSH_EFI_KEY_BUF_TAIL)(
    KEYBOARD_RETURN_API *This, EFI_KEY_DATA *KeyData);

struct _KEYBOARD_RETURN_API {
  PUSH_EFI_KEY_BUF_TAIL PushEfikeyBufTail;
};

typedef EFI_STATUS(EFIAPI *KEYBOARD_RESET)(KEYBOARD_DEVICE_PROTOCOL *This);
typedef EFI_STATUS(EFIAPI *KEYBOARD_GET_KEYS)(
    KEYBOARD_DEVICE_PROTOCOL *This, KEYBOARD_RETURN_API *KeyboardReturnApi,
    UINT64 Delta);

struct _KEYBOARD_DEVICE_PROTOCOL {
  KEYBOARD_RESET    Reset;
  KEYBOARD_GET_KEYS GetKeys;
};

extern EFI_GUID gTegraKeyboardDeviceProtocolGuid;

#endif