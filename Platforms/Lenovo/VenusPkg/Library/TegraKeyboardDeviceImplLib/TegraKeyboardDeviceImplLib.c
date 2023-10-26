/*
 * Tegra Keyboard input Library for the NVIDIA Tegra SoC internal matrix
 *
 * Copyright (c) 2009-2011, NVIDIA Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/BitmapLib.h>
#include <Library/TegraKeyboardDeviceImplLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#include <Protocol/TegraKeyboardDevice.h>

#define KBC_MAX_KPENT 8

typedef struct {
  KEY_CONTEXT EfiKeyContext;
  UINT8       Row;           // Row Number    (0 = First)
  UINT8       Col;           // Column Number (0 = First)
  UINT8       Valid;         // 1 If Valid, 0 If Invalid
} KEY_CONTEXT_PRIVATE;

UINTN gBitmapScanCodes[BITMAP_NUM_WORDS(0x18)]    = {0};
UINTN gBitmapUnicodeChars[BITMAP_NUM_WORDS(0x7f)] = {0};

#define MS2NS(ms) (((UINT64)(ms)) * 1000000ULL)

STATIC
inline
VOID
KeySetState(UINT16 ScanCode, CHAR16 UnicodeChar, BOOLEAN Value)
{
  if (ScanCode && ScanCode < 0x18) {
    if (Value) {
      BitmapSet(gBitmapScanCodes, ScanCode);
    } else {
      BitmapClear(gBitmapScanCodes, ScanCode);
    }
  }

  if (UnicodeChar && UnicodeChar < 0x7f) {
    if (Value) {
      BitmapSet(gBitmapUnicodeChars, ScanCode);
    } else {
      BitmapClear(gBitmapUnicodeChars, ScanCode);
    }
  }
}

STATIC
inline
VOID
LibKeyInitializeKeyContext(KEY_CONTEXT *Context)
{
  SetMem(&Context->KeyData, sizeof(Context->KeyData), 0);
  Context->Time      = 0;
  Context->State     = KEYSTATE_RELEASED;
  Context->Repeat    = FALSE;
  Context->Longpress = FALSE;
}

STATIC
inline
VOID
LibKeyUpdateKeyStatus(
  KEY_CONTEXT         *Context,
  KEYBOARD_RETURN_API *KeyboardReturnApi,
  BOOLEAN              IsPressed,
  UINT64               Delta)
{
  // keep track of the actual state
  KeySetState(Context->KeyData.Key.ScanCode, Context->KeyData.Key.UnicodeChar, IsPressed);

  // update key time
  Context->Time += Delta;

  switch (Context->State) {
    case KEYSTATE_RELEASED:
      if (IsPressed) {
        // change to pressed
        Context->Time  = 0;
        Context->State = KEYSTATE_PRESSED;
      }

      break;
  
    case KEYSTATE_PRESSED:
      if (IsPressed) {
        // keyrepeat
        if (Context->Repeat && Context->Time >= MS2NS(100)) {
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &Context->KeyData);
          Context->Time   = 0;
          Context->Repeat = TRUE;
        } else if (!Context->Longpress && Context->Time >= MS2NS(500)) {
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &Context->KeyData);
          Context->Time   = 0;
          Context->Repeat = TRUE;
        }

        Context->Longpress = TRUE;
      } else {
        if (!Context->Longpress) {
          // we supressed down, so report it now
          KeyboardReturnApi->PushEfikeyBufTail(KeyboardReturnApi, &Context->KeyData);
          Context->State = KEYSTATE_LONGPRESS_RELEASE;
        } else if (Context->Time >= MS2NS(10)) {
          // we reported another key already
          Context->Time      = 0;
          Context->Repeat    = FALSE;
          Context->Longpress = FALSE;
          Context->State     = KEYSTATE_RELEASED;
        }
      }

      break;

    case KEYSTATE_LONGPRESS_RELEASE:
      // change to released
      Context->Time      = 0;
      Context->Repeat    = FALSE;
      Context->Longpress = FALSE;
      Context->State     = KEYSTATE_RELEASED;
      break;

    default:
      ASSERT(FALSE);
      break;
  }
}

STATIC KEY_CONTEXT_PRIVATE KeyContextTAB;
STATIC KEY_CONTEXT_PRIVATE *KeyList[] = { &KeyContextTAB };

STATIC
VOID
KeyboardInitializeKeyContextPrivate(KEY_CONTEXT_PRIVATE *Context)
{
  Context->Valid = 0;
  Context->Row   = 0;
  Context->Col   = 0;
}

STATIC
KEY_CONTEXT_PRIVATE *KeyboardKeyCodeToKeyContext(UINT32 KeyCode)
{
  if (KeyCode == 114) {
    return &KeyContextTAB;
  }

  return NULL;
}

RETURN_STATUS
EFIAPI
TegraKeyboardDeviceImplConstructor(VOID)
{
  UINTN                Index;
  KEY_CONTEXT_PRIVATE *StaticContext;

  // Reset all keys
  for (Index = 0; Index < (sizeof(KeyList) / sizeof(KeyList[0])); Index++) {
    KeyboardInitializeKeyContextPrivate(KeyList[Index]);
  }

  // Configure keys
  /// TAB Key
  StaticContext              = KeyboardKeyCodeToKeyContext(114);
  StaticContext->Row         = 0x0;
  StaticContext->Col         = 0x1;

  return RETURN_SUCCESS;
}

EFI_STATUS
EFIAPI
TegraKeyboardDeviceImplReset(KEYBOARD_DEVICE_PROTOCOL *This)
{
  LibKeyInitializeKeyContext(&KeyContextESC.EfiKeyContext);
  KeyContextESC.EfiKeyContext.KeyData.Key.UnicodeChar = 0xd;  // Enter

  return EFI_SUCCESS;
}

EFI_STATUS
TegraKeyboardDeviceImplGetKeys(
  KEYBOARD_DEVICE_PROTOCOL *This,
  KEYBOARD_RETURN_API      *KeyboardReturnApi,
  UINT64                    Delta)
{
  BOOLEAN IsPressed;
  UINTN   Index;

  for (Index = 0; Index < (sizeof(KeyList) / sizeof(KeyList[0])); Index++) {
    KEY_CONTEXT_PRIVATE *Context = KeyList[Index];

    IsPressed = FALSE;

    // TODO: Add Stuff
  
    LibKeyUpdateKeyStatus(&Context->EfiKeyContext, KeyboardReturnApi, IsPressed, Delta);
  }

  return EFI_SUCCESS;
}