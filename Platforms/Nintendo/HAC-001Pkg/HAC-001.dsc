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
  PLATFORM_NAME                  = HAC-001
  PLATFORM_GUID                  = 2fa9729e-6bed-4f30-acdd-8eb3c1b4aca5
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/HAC-001Pkg
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = RELEASE|DEBUG
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = HAC-001Pkg/HAC-001.fdf
  
  #
  # 0 = Tegra X1
  # 1 = Tegra X1+
  #
  SOC_TYPE                       = 1

[LibraryClasses.common]
  DeviceMemoryMapLib|HAC-001Pkg/Library/DeviceMemoryMapLib/DeviceMemoryMapLib.inf
  KeypadDeviceLib|HAC-001Pkg/Library/KeypadDeviceLib/KeypadDeviceLib.inf

[PcdsFixedAtBuild.common]
  # Device Specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000             # Starting Address
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x100000000            # 4GB Size

  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Robotix22" # Device Maintainer

  #gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x0               # TODO: Fix me!

  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x83E00000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000          # 256K stack

  # SmBios
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemVendor|"Nintendo K.K."
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Switch"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"HAC-001"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"Switch_HAC-001"
  gNvidiaPkgTokenSpaceGuid.PcdSmbiosBoardModel|"Switch"

  # Simple FrameBuffer
  gNvidiaPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|720
  gNvidiaPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|1280
  gNvidiaPkgTokenSpaceGuid.PcdMipiFrameBufferColorDepth|32

[Components.common]
  # Keypad
  NvidiaPkg/Drivers/KeypadDxe/KeypadDxe.inf
  NvidiaPkg/Drivers/KeypadDeviceDxe/KeypadDeviceDxe.inf

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|720
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1280
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|720
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1280
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|90
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|67
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|90
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|67

!include Tegra210Pkg/Tegra210Pkg.dsc.inc
