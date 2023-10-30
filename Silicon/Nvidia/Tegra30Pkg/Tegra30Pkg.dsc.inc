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
  USE_TEGRA_TIMER    = 1

[PcdsFixedAtBuild.common]
  gNvidiaPkgTokenSpaceGuid.PcdTegraTimerLegacyIntrNum|13

  gArmTokenSpaceGuid.PcdGicDistributorBase|0x50041000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x50040100
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x50041000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000030
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x50041000

#if SOC_TYPE == 0
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Tegra (TM) 3 @ 1.50 GHz"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"T30"
#else if SOC_TYPE == 1
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Tegra (TM) 3 @ 1.30 GHz"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"T30L"
#else if SOC_TYPE == 2
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Tegra (R) 3 @ 1.70 GHz"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"T33"
#endif

  gArmPlatformTokenSpaceGuid.PcdCoreCount|4                     # Actually 5 Cores
  gArmPlatformTokenSpaceGuid.PcdClusterCount|1

[LibraryClasses.common]
  PlatformPeiLib|Tegra30Pkg/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|Tegra30Pkg/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  TegraClockLib|Tegra30Pkg/GPLLibrary/TegraClockLib/TegraClockLib.inf

[Components.common]
  Tegra30Pkg/Drivers/SmBiosTableDxe/SmBiosTableDxe.inf

!include NvidiaPkg/NvidiaPkg.dsc.inc