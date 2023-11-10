// Copyright (c) 2019 - 2020, Bingxing Wang and other project authors. All rights reserved.<BR>
// Copyright (c) 2021 - 2021, Leander Wollersberger. All rights reserved.<BR>

#include <Uefi.h>

#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
UnprotectTZ(VOID);

EFI_STATUS
FixConsoleOutput(VOID);

UINT32
ArmCallSmcHelper(
  UINT32 R0,
  UINT32 R1,
  UINT32 R2,
  UINT32 R3
  );

void *memmem(const void *h0, UINTN k, const void *n0, UINTN l);
