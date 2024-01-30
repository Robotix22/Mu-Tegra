## @file
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

# SoC Defines
[Defines]
  USE_PHYSICAL_TIMER = 1

[BuildOptions.common]
  *_*_AARCH64_CC_FLAGS = -march=armv8-a+crypto+rcpc

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|19200000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|13
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|14
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|11
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|10
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x50041000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x50042000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x50041000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000210
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x50041000

  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Tegra (TM) X1 @ 2.00 GHz"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"Tegra210"

  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2

[LibraryClasses.common]
  PlatformPeiLib|Tegra210Pkg/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|Tegra210Pkg/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  HwResetSystemLib|ArmPkg/Library/ArmSmcPsciResetSystemLib/ArmSmcPsciResetSystemLib.inf
  ResetSystemLib|ArmPkg/Library/ArmSmcPsciResetSystemLib/ArmSmcPsciResetSystemLib.inf
  SoCPlatformLib|Tegra210Pkg/Library/SoCPlatformLib/SoCPlatformLib.inf

[Components.common]
  Tegra210Pkg/Drivers/SmBiosTableDxe/SmBiosTableDxe.inf

!include NvidiaPkg/NvidiaPkg.dsc.inc
