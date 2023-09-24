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
  PLATFORM_NAME                  = Venus
  PLATFORM_GUID                  = 72ef84c9-ca18-4aa4-956f-f4b07e34da39
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/VenusPkg
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = VenusPkg/Venus.fdf

  # 0 = T30
  # 1 = T30L
  # 2 = T33
  # 3 = AP33
  SOC_TYPE                       = 0

[BuildOptions.common]
  *_*_*_CC_FLAGS = -DSOC_TYPE=$(SOC_TYPE) -DBOOT_METHOD=$(BOOT_METHOD) -DTEGRA30_TRUSTZONE_EXPLOIT

[LibraryClasses.common]
  PlatformMemoryMapLib|VenusPkg/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf

[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # Starting address
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x80000000         # 2GB Size

  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Robotix22"  # Device Maintainer

  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x94840000

  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x90300000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack

  # SmBios
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemVendor|"Lenovo Ltd"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemModel|"IdeaPad Yoga 11"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"Venus"                # NOTE: This is wrong
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"IdeaPad_Yoga_11_Venus"  # NOTE: This is wrong
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosBoardModel|"IdeaPad Yoga 11"

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
