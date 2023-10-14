## @file
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = oemk
  PLATFORM_GUID                  = b4dcb236-a5f5-411d-91f4-e446d4a8d2e8
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/oemkPkg
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = oemkPkg/oemk.fdf

  # 0 = T30
  # 1 = T30L
  # 2 = T33
  SOC_TYPE                       = 0

  # 0 = DDR3-L
  # 1 = LPDDR2-1066
  RAM_MODEL                      = 1

[BuildOptions.common]
  *_*_*_CC_FLAGS = -DSOC_TYPE=$(SOC_TYPE) -DRAM_MODEL=$(RAM_MODEL)

[LibraryClasses.common]
  PlatformMemoryMapLib|oemkPkg/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf

[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000                  # Starting address
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x80000000                  # 2GB Size

  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"No Maintainer"  # Device Maintainer

  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x88740000

  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x84200000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000               # 256K stack

  # SmBios
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemVendor|"Microsoft Corporation"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Surface RT"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"oemk"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"Surface_RT_oemk"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosBoardModel|"Surface RT"

  # Simple FrameBuffer
  gNvidiaPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1366
  gNvidiaPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|768
  gNvidiaPkgTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1366
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1366
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|170
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|40
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|170
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|40

!include Tegra30Pkg/Tegra30Pkg.dsc.inc