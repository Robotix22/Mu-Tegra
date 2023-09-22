/*
 * (C) Copyright 2010-2013
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <PiPei.h>
#include <Library/IoLib.h>
#include <Library/DevicePrePiLib.h>

VOID SetupDP501 (VOID)
{
  // TODO: Add Stuff
}

VOID InitializeDisplay (VOID)
{
  // Setup Bridge
  SetupDP501 ();
}

VOID DeviceInitialize (VOID)
{
  // Init Display
  InitializeDisplay ();
}
