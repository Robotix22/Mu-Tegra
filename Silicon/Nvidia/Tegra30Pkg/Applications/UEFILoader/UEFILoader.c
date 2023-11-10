// Copyright (c) 2019 - 2020, Bingxing Wang and other project authors. All rights reserved.<BR>
// Copyright (c) 2021 - 2021, Leander Wollersberger. All rights reserved.<BR>

#include "UEFILoader.h"

#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/FileHandleLib.h>

#include <Guid/FileInfo.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleTextOut.h>

VOID
ExitApp()
{
  Print(L"Reset your Device Manually.\n");

  // Do CPU Death Loop
  CpuDeadLoop();
}

VOID
memcpy_usr(
  VOID       *dest,
  CONST VOID *src,
  UINTN       n)
{
  //cast src and dest to char*
  char *src_char = (char *)src;
  char *dest_char = (char *)dest;
  for (int i=0; i<n; i++) {
    dest_char[i] = src_char[i]; //copy contents byte by byte
  }
}

// adopted from kernel code. thanks Leander :)
struct parking_protocol_mailbox {
  UINT32 cpu_id;
  UINT32 reserved;
  UINT64 entry_point; // keep at 64Bit to keep cpu_mailbox_entry aligned
};

struct cpu_mailbox_entry {
  struct parking_protocol_mailbox *mailbox;
  UINT32 mailbox_addr;
  UINT8 version;
  UINT8 gic_cpu_id;
};

static struct cpu_mailbox_entry cpu_mailbox_entries[4];

STATIC
VOID
acpi_parking_protocol_cpu_init()
{
  cpu_mailbox_entries[0].gic_cpu_id = 0;
  cpu_mailbox_entries[0].version = 1;
  cpu_mailbox_entries[0].mailbox_addr = 0x82001000;
  cpu_mailbox_entries[0].mailbox = (struct parking_protocol_mailbox*)(0x82001000U);

  cpu_mailbox_entries[1].gic_cpu_id = 1;
  cpu_mailbox_entries[1].version = 1;
  cpu_mailbox_entries[1].mailbox_addr = 0x82002000;
  cpu_mailbox_entries[1].mailbox = (struct parking_protocol_mailbox*)(0x82002000U);

  cpu_mailbox_entries[2].gic_cpu_id = 2;
  cpu_mailbox_entries[2].version = 1;
  cpu_mailbox_entries[2].mailbox_addr = 0x82003000;
  cpu_mailbox_entries[2].mailbox = (struct parking_protocol_mailbox*)(0x82003000U);

  cpu_mailbox_entries[3].gic_cpu_id = 3;
  cpu_mailbox_entries[3].version = 1;
  cpu_mailbox_entries[3].mailbox_addr = 0x82004000;
  cpu_mailbox_entries[3].mailbox = (struct parking_protocol_mailbox*)(0x82004000U);
}

VOID
StartSecondaryCores(INT32 Cpu)
{
  UINT32 Value = 0;

  acpi_parking_protocol_cpu_init();

  // Let the secondary core use the payload loaded by UEFI
  MmioWrite32((UINT32)(&cpu_mailbox_entries[1].mailbox->entry_point), 0x93200000U);
  MmioWrite32((UINT32)(&cpu_mailbox_entries[2].mailbox->entry_point), 0x93300000U);
  MmioWrite32((UINT32)(&cpu_mailbox_entries[3].mailbox->entry_point), 0x93400000U);

  ArmDataMemoryBarrier();
  ArmDataSynchronizationBarrier();

  MmioWrite32((UINT32)(&cpu_mailbox_entries[Cpu].mailbox->cpu_id), Cpu);

  ArmDataMemoryBarrier();
  ArmDataSynchronizationBarrier();

  // Interrupt magic.
  // interrupt according to ACPI PP 0x00FE0000
  // reg: 0xF00
  // base: 0x50041000
  MmioWrite32(0x50041F00U, 0x00FE0000U); // send interrupt to trigger secondary core unpark
  MmioWrite32(0x50041F00U, 0x00FE0000U);
  MmioWrite32(0x50041F00U, 0x00FE0000U);

  // loop that waits for the mailbox to be cleared when secondary core starts
  while (MmioRead32((UINT32)(&cpu_mailbox_entries[Cpu].mailbox->entry_point)) != 0);

  Value = MmioRead32((UINT32)(&cpu_mailbox_entries[Cpu].mailbox->entry_point));
  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Entry: 0x%08x\r\n", __FUNCTION__, Value));

  Value = MmioRead32((UINT32)(&cpu_mailbox_entries[Cpu].mailbox->cpu_id));
  DEBUG ((EFI_D_WARN, "[DEBUG] %a: CpuID: 0x%08x\r\n", __FUNCTION__, Value));
}

EFI_STATUS
JumpToUEFI(EFI_HANDLE ImageHandle)
{
  EFI_STATUS             Status     = EFI_SUCCESS;
  EFI_MEMORY_DESCRIPTOR *MemMap     = 0;
  UINT32                 DesVersion = 0;
  UINTN                  MemMapSize = 0;
  UINTN                  MapKey     = 0;
  UINTN                  DesSize    = 0;

  // Get Memory Map
  gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DesSize, &DesVersion);

  // Exit Boot Services
  Status = gBS->ExitBootServices(ImageHandle, MapKey);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Exit Boot Services! Status = %r\n", __FUNCTION__, Status));
    goto exit;
  }

  // Deinit Original UEFI for Custom UEFI to takeover
  ArmDisableFiq();
  ArmDisableInterrupts();
  ArmDisableCachesAndMmu();
  ArmInvalidateInstructionCache();
  ArmInvalidateTlb();

  // Disable Gic
  MmioWrite32((UINT32)FixedPcdGet32(PcdGicRedistributorsBase), 0);

  // This should trigger an SMC, Jump to the payload and output stuff Hopefully.
  ArmCallSmcHelper(0, 0, 0, 0);

  // Something is Wrong.
  // The Code should not reach here.
  CpuDeadLoop();

exit:
  return Status;
}

EFI_STATUS
LoadUEFIIntoMemory(
  IN EFI_PHYSICAL_ADDRESS MemoryAddress,
  IN CHAR16              *FileName,
  OUT UINTN              *FileSize,
  IN EFI_HANDLE           ImageHandle)
{
  EFI_STATUS                       Status              = EFI_SUCCESS;
  EFI_GUID                         SFSPGuid            = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_GUID                         LoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
  EFI_LOADED_IMAGE                *LoadedImage         = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SFSProtocol         = NULL;
  EFI_FILE_PROTOCOL               *FileProtocol        = NULL;
  EFI_FILE_PROTOCOL               *File                = NULL;
  EFI_FILE_INFO                   *FileInfo            = NULL;
  EFI_PHYSICAL_ADDRESS             MemoryLocationAddr  = MemoryAddress;
  UINTN                            FileInfoSize        = 0;

  // Locate Loaded Image Handle Protocol
  Status = gBS->HandleProtocol(ImageHandle, &LoadedImageProtocol, (VOID *)&LoadedImage);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Locate Loaded Image Handle Protocol! Status = %r\n", __FUNCTION__, Status));
    goto exit;
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Located Loaded Image Handle Protocol.\n", __FUNCTION__));

  // Locate SFS Handle Protocol
  Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &SFSPGuid, (VOID *)&SFSProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Locate SFS Handle Protocol! Status = %r\n", __FUNCTION__, Status));
    goto exit;
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Located SFS Handle Protocol.\n", __FUNCTION__));

  // Open Current Volume (The Volume where the Application is)
  Status = SFSProtocol->OpenVolume(SFSProtocol, &FileProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Open Current Volume! Status = %r\n", __FUNCTION__, Status));
    goto exit;
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Opend Current Volume.\n", __FUNCTION__));

  // Open Payload File
  Status = FileProtocol->Open(FileProtocol, &File, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Open %s! Status = %r\n", __FUNCTION__, FileName, Status));
    goto exit;
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Opend %s.\n", __FUNCTION__, FileName));

  // Get the Infos about the Payload File
  Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, (VOID *)FileInfo);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    // Allocate Memory for the Payload
    Status = gBS->AllocatePool(EfiLoaderData, FileInfoSize, (VOID **)&FileInfo);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Allocate Memory! Status = %r\n", __FUNCTION__, Status));
      goto exit;
    }

    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Allocated Memory.\n", __FUNCTION__));

    SetMem((VOID *)FileInfo, FileInfoSize, 0xFF);

    // Get the Infos about the Payload File (Again)
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, (VOID *)FileInfo);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Get File Infos from %s! Status = %r\n", __FUNCTION__, FileName, Status));
      goto exit;
    }

    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Got File Infos from %s.\n", __FUNCTION__, FileName));
  } else if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Get File Infos from %s! Status = %r\n", __FUNCTION__, FileName, Status));
    goto exit;
  } else {
    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Got File Infos from %s.\n", __FUNCTION__, FileName));
  }

  // Check if Payload isn't too large
  if (FileInfo->FileSize > 0xFFFFFFFF) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: %s is too Large! Size: 0x%x", __FUNCTION__, FileName, FileInfo->FileSize));
    Status = EFI_OUT_OF_RESOURCES;
    goto exit;
  }

  Print(L"Size of %s: 0x%x\n", FileName, FileInfo->FileSize);

  // Allocate Memory for the Payload (Again)
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, EFI_SIZE_TO_PAGES(FileInfo->FileSize), &MemoryLocationAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Allocate Memory! Status = %r\n", __FUNCTION__, Status));
    goto exit;
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Allocated Memory.\n", __FUNCTION__));

  SetMem((EFI_PHYSICAL_ADDRESS *)MemoryLocationAddr, FileInfo->FileSize, 0xFF);

  // Read Payload into Memory
  Status = File->Read(File, (UINTN *)&FileInfo->FileSize, (EFI_PHYSICAL_ADDRESS *)MemoryLocationAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Read %s into Memory! Status = %r\n", __FUNCTION__, FileName, Status));
    goto exit;
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Read %s into Memory.\n", __FUNCTION__, FileName));

  *FileSize = FileInfo->FileSize;

  Print(L"%s in Memory at: 0x%x\n", FileName, MemoryLocationAddr);

exit:
  return Status;
}

EFI_STATUS
EFIAPI
UEFILoaderMain(
  EFI_HANDLE        ImageHandle,
  EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status   = EFI_SUCCESS;
  INT32      i        = 0;
  UINTN      FileSize = 0;

  UINT8 SmcHandlerString[5];

  SmcHandlerString[0] = 0x02;
  SmcHandlerString[1] = 0x00;
  SmcHandlerString[2] = 0x50;
  SmcHandlerString[3] = 0xe3;
  SmcHandlerString[4] = 0xc8;

  // Fix Display Output
  Status = FixConsoleOutput();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "[DEBUG] %a: Failed to Fix Console Output! Status = %r\n", __FUNCTION__, Status));
    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Continue without Display.\n", __FUNCTION__));
  }

  // Print App Info
  Print(L"UEFI Loader Ver. %s\nBuilt on %a at %a\n\n", (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__);

  // Disable Watch Dog Timer
  gBS->SetWatchdogTimer(0, 0, 0, NULL);

  // Unprotect TZ
  Print(L"Unprotect Trust Zone...\n");

  Status = UnprotectTZ();
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Unprotect Trust Zone! Status = %r\n", Status);
    goto exit;
  }

  Print(L"Trust Zone has been Successfully Unprotected.\n");
  Print(L"Load Exploit Payload into Memory...\n");

  // Load Exploit Payload into Memory
  Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93000000, L"ExploitSecureWorld.bin", &FileSize, ImageHandle);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Load Exploit Payload into Memory! Status = %r\n", Status);
    goto exit;
  }

  Print(L"Exploit Payload has been Successfully Read into Memory.\n");
  Print(L"Load Secondary Cores Payload into Memory...\n");

  // Load Secondary Cores Payload into Memory
  Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93110000, L"SecondaryCores.bin", 0, ImageHandle);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Load Secondary Cores Payload into Memory! Status = %r\n", Status);
    goto exit;
  } else {
    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Loaded SecondaryCores.bin at 0x93110000.\n", __FUNCTION__));

    Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93120000, L"SecondaryCores.bin", 0, ImageHandle);
    if (EFI_ERROR (Status)) {
      Print(L"Failed to Load Secondary Cores Payload into Memory! Status = %r\n", Status);
      goto exit;
    } else {
      DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Loaded SecondaryCores.bin at 0x93120000.\n", __FUNCTION__));

      Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93130000, L"SecondaryCores.bin", 0, ImageHandle);
      if (EFI_ERROR (Status)) {
        Print(L"Failed to Load Secondary Cores Payload into Memory! Status = %r\n", Status);
        goto exit;
      }
    }
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Loaded SecondaryCores.bin at 0x93130000.\n", __FUNCTION__));

  Print(L"Secondary Cores Payload has been Successfully Read into Memory.\n");
  Print(L"Load Smc Payload into Memory...\n");

  // Load Smc Payload into Memory
  Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93200000, L"Smc.bin", 0, ImageHandle);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Load Smc Payload into Memory! Status = %r\n", Status);
    goto exit;
  } else {
    DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Loaded Smc.bin at 0x93200000.\n", __FUNCTION__));
  
    Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93300000, L"Smc.bin", 0, ImageHandle);
    if (EFI_ERROR (Status)) {
      Print(L"Failed to Load Smc Payload into Memory! Status = %r\n", Status);
      goto exit;
    } else {
      DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Loaded Smc.bin at 0x93300000.\n", __FUNCTION__));

      Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x93400000, L"Smc.bin", 0, ImageHandle);
      if (EFI_ERROR (Status)) {
        Print(L"Failed to Load Smc Payload into Memory! Status = %r\n", Status);
        goto exit;
      }
    }
  }

  DEBUG ((EFI_D_WARN, "[DEBUG] %a: Successfully Loaded Smc.bin at 0x93400000.\n", __FUNCTION__));

  Print(L"Smc Payload has been Successfully Read into Memory.\n");
  Print(L"Load Project Mu UEFI Payload into Memory...\n");

  // Load UEFI Payload into Memory
  Status = LoadUEFIIntoMemory((EFI_PHYSICAL_ADDRESS)0x84000000, L"ProjectMu.bin", 0, ImageHandle);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Load UEFI Payload into Memory! Status = %r\n", Status);
    goto exit;
  }

  Print(L"Project Mu UEFI Payload has been Successfully Read into Memory.\n");
  Print(L"Trying to Write Trust Zone...\n");

  // Disable MMU to get access to Trustzone memory by disarming the Translation Table / Page Table
  // NO UEFI FROM HERE: Print(), ..., and so on
  ArmDisableFiq();
  ArmDisableInterrupts();
  ArmDisableCachesAndMmu();

  // Copy payload into Trustzone memory.
  // The memory needs to be marked as secure, as you can only execute secure memory in secure mode so
  // we copy the payload to TZ memory
  VOID *mTZSmcHandlerString = memmem((VOID *)0x80000000, 0x200000, SmcHandlerString, sizeof(SmcHandlerString));
  memcpy_usr((VOID *)(mTZSmcHandlerString), (CONST VOID *)0x93000000, (UINTN)FileSize);

  MmioWrite32(0x82002880U, 0x93110000U);
  MmioWrite32(0x82003880U, 0x93120000U);
  MmioWrite32(0x82004880U, 0x93130000U);

  ArmDataSynchronizationBarrier();

  // Payload is now in place. Enable MMU to use UEFI one last time
  ArmEnableMmu();
  ArmEnableDataCache();
  ArmEnableInstructionCache();
  ArmEnableFiq();
  ArmEnableInterrupts();

  // Print UEFI Smc Address
  DEBUG ((EFI_D_WARN, "[DEBUG] %a: UEFI SMC Address: 0x%08x\n", __FUNCTION__, mTZSmcHandlerString));

  Print(L"Successfully Wrote Trust Zone.\n");

  // Start Secondary Core
  Print(L"Starting Secondary Cores...\n");
  for (i = 1; i < 4; i++) {
    StartSecondaryCores(i);
    Print(L"Secondary Core %d Started.\n", i);
  }

  // Jump to UEFI Payload
  Print(L"Jumping to Project Mu UEFI Payload...\n");

  Status = JumpToUEFI(ImageHandle);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Jump to Project Mu UEFI! Status = %r\n", Status);
    goto exit;
  }

exit:
  ExitApp();

  return Status;
}