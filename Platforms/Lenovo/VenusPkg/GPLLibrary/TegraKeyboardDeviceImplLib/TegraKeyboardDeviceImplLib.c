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
#include <Library/TimerLib.h>

#include <Protocol/TegraKeyboardDevice.h>

// KBC Registers
#define KBC_BASE_ADDR               0x7000E200
#define KBC_MAX_KPENT               8
#define KBC_RPT_DLY_0               0x2C
#define KBC_INIT_DLY_0              0x28
#define KBC_KP_ENT0_0               0x30
#define KBC_KP_ENT1_0               0x34                 
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

/*
// Unicode Char Key Map (Shift State)
STATIC CHAR16 KeyMapUnicodeCharShift[16][8] = {
  {0, CHAR_TAB, '~', '!', 'Q', 'A', 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, '#', 'E', 'D', 'C', ' '},
  {0, 0, 0, '@', 'W', 'S', 'X', 'Z'},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {'G', 'T', '%', '$', 'R', 'F', 'V', 'B'},
  {'H', 'Y', '^', '&', 'U', 'J', 'M', 'N'},
  {0, 0, 0, '(', 'O', 'L', '>', 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, '*', 'I', 'K', '<', 0},
  {0, 0, '|', 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {'_', 0, ')', 'P', 0, ':', '?', 0},
  {0, 0, '+', CHAR_BACKSPACE, 0, 0, '"', CHAR_CARRIAGE_RETURN},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
*/

RETURN_STATUS
EFIAPI
TegraKeyboardDeviceImplConstructor(VOID)
{
  // Set Hardware Matrix Delay
  MmioWrite32(KBC_BASE_ADDR + KBC_RPT_DLY_0, 1200);
  MmioWrite32(KBC_BASE_ADDR + KBC_INIT_DLY_0, 1200);

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
  UINT32 kp_ent0  = 0;
  UINT32 kp_ent1  = 0;
  UINT32 fifo_cnt = 0;
  UINT32 i        = 0;
  UINT32 Col      = 0;
  UINT32 Row      = 0;

  fifo_cnt = (MmioRead32(KBC_BASE_ADDR + KBC_INT_0) >> 4) & 0xf;

  if (fifo_cnt) {
    for (i = 0; i < KBC_MAX_KPENT; i++) {
      // Get next word
      if ((i % 4) == 0) {
        kp_ent0 = MmioRead32 (KBC_BASE_ADDR + KBC_KP_ENT0_0 + i);
        kp_ent1 = MmioRead32 (KBC_BASE_ADDR + KBC_KP_ENT1_0);        // This Read Emptys Fifo Head
      }

      if (kp_ent0 & 0x80) {
        Col = kp_ent0 & 0x7;
        Row = (kp_ent0 >> 3) & 0xf;

        // Wait a Bit before Processing Key
        MicroSecondDelay(80000);

        // TODO: Add Shift and Caps Lock Check Here.

        // Update Key Status
        if (KeyMapUnicodeChar[Row][Col] != 0) {
          EFI_KEY_DATA KeyPressed = {.Key = {.UnicodeChar = KeyMapUnicodeChar[Row][Col],}};
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &KeyPressed);
        } else if (KeyMapScanCode[Row][Col] != 0) {
          EFI_KEY_DATA KeyPressed = {.Key = {.ScanCode = KeyMapScanCode[Row][Col],}};
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &KeyPressed);
        }
      }

      // Shift to get next entry
      kp_ent0 >>= 8;
    }
  } else {
    // TODO: Add Gpio Side Buttons here.
  }

  return EFI_SUCCESS;
}