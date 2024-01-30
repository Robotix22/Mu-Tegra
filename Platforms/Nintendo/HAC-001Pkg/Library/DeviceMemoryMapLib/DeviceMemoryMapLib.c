#include <Library/BaseLib.h>
#include <Library/DeviceMemoryMapLib.h>

STATIC
ARM_MEMORY_REGION_DESCRIPTOR_EX
gDeviceMemoryDescriptorEx[] = {
  // Name, Address, Length, HobOption,
  // ResourceAttribute, ArmAttributes,
  // ResourceType, MemoryType

  // DDR Regions
  {"DXE Heap", 0x80000000, 0x03C00000, AddMem,
   EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
   EfiConventionalMemory, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

  {"UEFI FD", 0x83C00000, 0x00200000, AddMem,
   EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
   EfiBootServicesData, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},
  
  {"UEFI Stack", 0x83E00000, 0x00040000, AddMem,
   EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
   EfiBootServicesData, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

  {"Display Reserved", 0xF5A00000, 0x00384000, AddMem,
   EFI_RESOURCE_MEMORY_RESERVED, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
   EfiReservedMemoryType, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH},
  
  // NOTE: Added to bypass MMU Loop
  {"RAM Partition", 0xF5D84000, 0x0A27C000, AddMem,
   EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
   EfiConventionalMemory, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

  // Register regions
  {"Gic Distributor", 0x50041000, 0x00001000, AddDev,
   EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
   EfiMemoryMappedIO, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE},
  
  {"Gic Redistributors", 0x50042000, 0x00006000, AddDev,
   EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
   EfiMemoryMappedIO, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE},

  {"GPIO Controller", 0x6000D000, 0x00001000, AddDev,
   EFI_RESOURCE_MEMORY_MAPPED_IO, EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE,
   EfiMemoryMappedIO, ARM_MEMORY_REGION_ATTRIBUTE_DEVICE},

  // Terminator for MMU
  {"Terminator", 0, 0, 0, 0, 0, 0, 0}
};

ARM_MEMORY_REGION_DESCRIPTOR_EX*
GetDeviceMemoryMap()
{
  return gDeviceMemoryDescriptorEx;
}
