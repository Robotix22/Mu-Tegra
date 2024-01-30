#ifndef _DEVICE_MEMORY_MAP_LIB_H_
#define _DEVICE_MEMORY_MAP_LIB_H_

#include <PiPei.h>

#include <Library/ArmLib.h>

#define MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT 128

// Below flag is used for system memory
#define SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES                           \
  EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED |    \
  EFI_RESOURCE_ATTRIBUTE_TESTED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |     \
  EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |                               \
  EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |                         \
  EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |                            \
  EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE |                           \
  EFI_RESOURCE_ATTRIBUTE_READ_PROTECTABLE |                                \
  EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTABLE

typedef enum {
  NoHob,
  AddMem,
  AddDev,
  HobOnlyNoCacheSetting,
  AddDynamicMem,
  AllocOnly,
  MaxMem
} DeviceMemoryAddHob;

#define MEMORY_REGION_NAME_MAX_LENGTH 32

typedef struct {
  CHAR8                        Name[MEMORY_REGION_NAME_MAX_LENGTH];
  EFI_PHYSICAL_ADDRESS         Address;
  UINT64                       Length;
  DeviceMemoryAddHob           HobOption;
  EFI_RESOURCE_TYPE            ResourceType;
  EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute;
  EFI_MEMORY_TYPE              MemoryType;
  ARM_MEMORY_REGION_ATTRIBUTES ArmAttributes;
} ARM_MEMORY_REGION_DESCRIPTOR_EX, *PARM_MEMORY_REGION_DESCRIPTOR_EX;

ARM_MEMORY_REGION_DESCRIPTOR_EX* GetDeviceMemoryMap();

#endif /* _DEVICE_MEMORY_MAP_LIB_H_ */
