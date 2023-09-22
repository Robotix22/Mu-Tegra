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

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|19200000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|17
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|18
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x50041000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x50040100
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x50041000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000030
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x50041000

  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Tegra (R) 3 @ 1.70 GHz"
!if $(SOC_TYPE) == 0
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"T30"
!elseif $(SOC_TYPE) == 1
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"T30L"
!elseif $(SOC_TYPE) == 2
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"T33"
!elseif $(SOC_TYPE) == 3
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"AP33"
!endif

  gArmPlatformTokenSpaceGuid.PcdCoreCount|4
  gArmPlatformTokenSpaceGuid.PcdClusterCount|1

[LibraryClasses.common]
  PlatformPeiLib|Tegra30Pkg/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|Tegra30Pkg/Library/PlatformPrePiLib/PlatformPrePiLib.inf

[Components.common]
  Tegra30Pkg/Drivers/SmBiosTableDxe/SmBiosTableDxe.inf

!include NvidiaPkg/NvidiaPkg.dsc.inc
