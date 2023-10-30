/*
 * Tegra Keyboard input Library for the NVIDIA Tegra SoC internal matrix
 *
 * Copyright (c) 2009-2011, NVIDIA Corporation.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/TegraKeyboardDeviceImplLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>

#include <Protocol/TegraKeyboardDevice.h>

// KBC Registers
#define KBC_BASE_ADDR               0x7000E200
#define KBC_MAX_KPENT               8
#define KBC_KP_ENT0_0               0x30
#define KBC_CONTROL_FIFO_CNT_INT_EN (1 << 3)
#define KBC_INT_0                   0x4

// Scan Code Key Map
STATIC UINT16 KeyMapScanCode[16][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {SCAN_F2, SCAN_F1, SCAN_ESC, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, SCAN_F11, SCAN_F10, 0, 0, 0, 0, SCAN_DOWN},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {SCAN_F7, SCAN_F6, SCAN_F5, 0, 0, 0, 0, 0},
  {SCAN_F8, SCAN_F9, 0, 0, 0, SCAN_LEFT, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, SCAN_UP},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, SCAN_DELETE, SCAN_PAGE_UP, SCAN_INSERT, SCAN_F12, SCAN_PAGE_DOWN, SCAN_RIGHT}
};

// Unicode Char Key Map
STATIC CHAR16 KeyMapUnicodeChar[16][8] = {
  {0, CHAR_TAB, '`', '1', 'q', 'a', 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, '3', 'e', 'd', 'c', ' '},
  {0, 0, 0, '2', 'w', 's', 'x', 'z'},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {'g', 't', '5', '4', 'r', 'f', 'v', 'b'},
  {'h', 'y', '6', '7', 'u', 'j', 'm', 'n'},
  {0, 0, 0, '9', 'o', 'l', '.', 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, '8', 'i', 'k', ',', 0},
  {0, 0, '\\', 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {'-', 0, '0', 'p', 0, ';', '/', 0},
  {0, 0, '=', CHAR_BACKSPACE, 0, 0, 0, CHAR_CARRIAGE_RETURN},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

VOID
SetFifoInterrupt(BOOLEAN Enable)
{
  UINT32 Value;

  Value = MmioRead32(KBC_BASE_ADDR);

  if (Enable) {
    Value |= KBC_CONTROL_FIFO_CNT_INT_EN;
  } else {
    Value &= ~KBC_CONTROL_FIFO_CNT_INT_EN;
  }

  MmioWrite32(KBC_BASE_ADDR, Value);
}

RETURN_STATUS
EFIAPI
TegraKeyboardDeviceImplConstructor(VOID)
{
  return RETURN_SUCCESS;
}

EFI_STATUS
EFIAPI
TegraKeyboardDeviceImplReset(KEYBOARD_DEVICE_PROTOCOL *This)
{
  return EFI_SUCCESS;
}

EFI_STATUS
TegraKeyboardDeviceImplGetKeys(
  KEYBOARD_DEVICE_PROTOCOL *This,
  KEYBOARD_RETURN_API      *KeyboardReturnApi,
  UINT64                    Delta)
{
  UINT32 kp_ent;
  UINT32 Value;
  UINT32 i;
  UINT32 Col;
  UINT32 Row;

  Value = (MmioRead32(KBC_BASE_ADDR + KBC_INT_0) >> 4) & 0xf;

  if (Value) {
    for (i = 0; i < KBC_MAX_KPENT; i++) {
      // Get next word
      if ((i % 4) == 0) {
        kp_ent = MmioRead32 (KBC_BASE_ADDR + KBC_KP_ENT0_0 + i);
      }

      if (kp_ent & 0x80) {
        Col = kp_ent & 0x7;
        Row = (kp_ent >> 3) & 0xf;

        // Update Key Status
        if (KeyMapUnicodeChar[Row][Col] != 0) {
          EFI_KEY_DATA KeyPressed = {.Key = {.UnicodeChar = KeyMapUnicodeChar[Row][Col],}};
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &KeyPressed);
        } else if (KeyMapScanCode[Row][Col] != 0) {
          EFI_KEY_DATA KeyPressed = {.Key = {.ScanCode = KeyMapUnicodeChar[Row][Col],}};
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &KeyPressed);
        }
      }
    }

    // Shift to get next entry
    kp_ent >>= 8;
  } else {
    // Enable Keypress Interrupt
    SetFifoInterrupt(TRUE);
  }

  return EFI_SUCCESS;
}