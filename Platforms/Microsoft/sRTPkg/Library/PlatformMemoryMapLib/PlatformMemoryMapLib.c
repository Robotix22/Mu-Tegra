#include <Library/BaseLib.h>
#include <Library/PlatformMemoryMapLib.h>

static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    //
    // Name, Address, Length, HobOption,
    // ResourceAttribute, ArmAttributes,
    // ResourceType, MemoryType
    //

    // DDR Regions
    {"UEFI FD", 0x84000000, 0x00200000, AddMem,
     EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     EfiBootServicesData, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

    {"UEFI Stack", 0x84200000, 0x00040000, AddMem,
     EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     EfiBootServicesData, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

    {"DXE Heap", 0x84240000, 0x04500000, AddMem,
     EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     EfiConventionalMemory, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

    {"CPU Vectors", 0x88740000, 0x00001000, AddMem,
     EFI_RESOURCE_SYSTEM_MEMORY, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     EfiBootServicesData, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK},

    {"Display Reserved", 0xFC400000, 0x01000000, AddMem,
     EFI_RESOURCE_MEMORY_RESERVED, SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES,
     EfiReservedMemoryType, ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH},

    // RAM Partition Regions
    // TODO: Add RAM Partition Region

    // Register Regions
    // TODO: Add Register Regions

    // Terminator for MMU
    {"Terminator", 0, 0, 0, 0, 0, 0, 0}};

ARM_MEMORY_REGION_DESCRIPTOR_EX *GetPlatformMemoryMap()
{
    return gDeviceMemoryDescriptorEx;
}