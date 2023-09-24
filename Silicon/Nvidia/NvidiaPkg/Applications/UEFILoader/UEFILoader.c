#include <Library/ArmLib.h>
#include <Library/EFIELF.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/FileHandleLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

#include "UEFILoader.h"

// Global Variables
EFI_PHYSICAL_ADDRESS UEFIPayloadEntryPoint;
EFI_FILE_PROTOCOL   *PayloadFileProtocol    = NULL;
EFI_FILE_INFO       *PayloadFileInformation = NULL;
VOID                *PayloadFileBuffer      = NULL;
BOOLEAN              ValidELFFile           = FALSE;
UINT64               UEFIPayloadLength      = 0;
UINTN                PayloadFileBufferSize  = 0;

VOID
ExecuteUEFIPayload(
  IN EFI_HANDLE ImageHandle)
{
  EFI_STATUS             Status;
  EFI_MEMORY_DESCRIPTOR *MemMap         = NULL;
  UINT32                 PayloadAddress = (UINT32) UEFIPayloadEntryPoint;
  UINT32                 PayloadLength  = (UINT32) UEFIPayloadLength;
  UINT32                 DesVersion     = 0;
  UINTN                  MemMapSize     = 0;
  UINTN                  MapKey         = 0;
  UINTN                  DesSize        = 0;

  Print(L"Executing UEFI Payload...\n");

  // Set Entry Function
  VOID(*entry)() = (VOID*) PayloadAddress;

  // Get Memory Map
  gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DesSize, &DesVersion);

  // Exit Boot Services
  Status = gBS->ExitBootServices(ImageHandle, MapKey);
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Exit Boot Services! Status = %r\n", Status);
    UEFILoaderFinish();
  }

  // Move LOAD Section to actual Location
  SetMem((VOID*)PayloadAddress, PayloadLength, 0xFF);
  CopyMem((VOID*)PayloadAddress, PayloadFileBuffer, PayloadLength);

  // De-Init Platform
  ArmDisableBranchPrediction();
  ArmDisableFiq();
  ArmDisableInterrupts();
  ArmDisableDataCache();
  ArmInvalidateInstructionCache();
  ArmDisableCachesAndMmu();
  ArmInvalidateTlb();

  // Disable Gic
  MmioWrite32(0, FixedPcdGet32(PcdGicDistributorBase));

  // Execute UEFI Payload
  entry();
}

STATIC
VOID
ValiadeELFFile(VOID)
{
  EFI_STATUS           Status;
  EFI_PHYSICAL_ADDRESS ELFEntryPoint;
  elf32_ehdr          *bl_elf_hdr    = PayloadFileBuffer;

  // Check if bl_elf_hdr is set
  if (bl_elf_hdr == NULL) {
    ValidELFFile = FALSE;
    return;
  }

  // Check ELF Header
  if (bl_elf_hdr->e_ident[EI_MAG0] != ELFMAG0 || bl_elf_hdr->e_ident[EI_MAG1] != ELFMAG1 || bl_elf_hdr->e_ident[EI_MAG2] != ELFMAG2 || bl_elf_hdr->e_ident[EI_MAG3] != ELFMAG3) {
    DEBUG((EFI_D_ERROR, "[DEBUG] %a: Invalid ELF Header!\n", __FUNCTION__));
    ValidELFFile = FALSE;
    return;
  }

  // Check if ELF is ARM
  if (bl_elf_hdr->e_machine != EM_ARM) {
    DEBUG((EFI_D_ERROR, "[DEBUG] %a: ELF File is not ARM!\n", __FUNCTION__));
    ValidELFFile = FALSE;
    return;
  }

  // Check if ELF is executeable
  if (bl_elf_hdr->e_type != ET_EXEC){
    DEBUG((EFI_D_ERROR, "[DEBUG] %a: ELF File isn't executeable!\n", __FUNCTION__));
    ValidELFFile = FALSE;
    return;
  }

  // Check if ELF Entry Point is valid
  ELFEntryPoint = bl_elf_hdr->e_entry;
  Status = gBS->AllocatePages(AllocateAddress, EfiLoaderCode, 1, &ELFEntryPoint);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_WARN, "[DEBUG] %a: Invalid Entry Point! Status = %r\n", __FUNCTION__, Status));
    ValidELFFile = FALSE;
    return;
  } else {
    // Free Memory
    gBS->FreePages(ELFEntryPoint, 1);
  }

  // Check if ELF has atleast one Program header entry
  if (bl_elf_hdr->e_phnum < 1) {
    DEBUG((EFI_D_ERROR, "[DEBUG] %a: No Program Header Entries found!\n", __FUNCTION__));
    ValidELFFile = FALSE;
    return;
  }

  // Check overlapping
  if (bl_elf_hdr->e_phoff < sizeof(PayloadFileBuffer)) {
    DEBUG((EFI_D_ERROR, "[DEBUG] %a: ELF Header has Overlapping!\n", __FUNCTION__));
    ValidELFFile = FALSE;
    return;
  }

  ValidELFFile = TRUE;
}

EFI_STATUS
LoadElfFile()
{
  EFI_STATUS                       Status                     = EFI_SUCCESS;
  EFI_HANDLE                      *SfsHandles                 = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *EfiSfsProtocol             = NULL;
  EFI_FILE_PROTOCOL               *FileProtocol               = NULL;
  CHAR16                          *PayloadFileName            = L"UEFI.elf";
  UINTN                            PayloadFileInformationSize = 0;
  UINTN                            NumHandles                 = 0;

  // Locate Handle Buffer of SFS
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &NumHandles, &SfsHandles);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to get Loaded Image Handle Protocol! Status = %r\n", __FUNCTION__, Status));
    goto exit;
  } else {
    DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Located SFS Handle\n", __FUNCTION__));
  }

  for (UINTN index = 0; index < NumHandles; index++) {
    // Invoke SFS Handle Protocol
    Status = gBS->HandleProtocol(SfsHandles[index], &gEfiSimpleFileSystemProtocolGuid, (VOID **) &EfiSfsProtocol);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to get SFS Handle Protocol! Status = %r\n", __FUNCTION__, Status));
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Located SFS Handle Protocol.\n", __FUNCTION__));
    }

    // Get File Protocol Handle
    Status = EfiSfsProtocol->OpenVolume(EfiSfsProtocol, &FileProtocol);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to Open Currect Volume! Status = %r\n", __FUNCTION__, Status));
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Opend the current Volume.\n", __FUNCTION__));
    }

    // Open the UEFI Payload File
    Status = FileProtocol->Open(FileProtocol, &PayloadFileProtocol, PayloadFileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to Open UEFI Payload File! Status = %r\n", __FUNCTION__, Status));
      continue;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Opend UEFI Payload File.\n", __FUNCTION__));
    }

    // Get File Infos from UEFI Payload
    Status = PayloadFileProtocol->GetInfo(PayloadFileProtocol, &gEfiFileInfoGuid, &PayloadFileInformationSize, (VOID *) PayloadFileInformation);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      // Allocate Memory
      Status = gBS->AllocatePool(EfiLoaderData, PayloadFileInformationSize, (VOID **)&PayloadFileInformation);
      if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to Allocate Memory! Status = %r\n", __FUNCTION__, Status));
        goto exit;
      } else {
        DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Allocated Memory.\n", __FUNCTION__));
      }

      SetMem((VOID *) PayloadFileInformation, PayloadFileInformationSize, 0xFF);

      // Get File Infos from UEFI Payload Again
      Status = PayloadFileProtocol->GetInfo(PayloadFileProtocol, &gEfiFileInfoGuid, &PayloadFileInformationSize, (VOID *)PayloadFileInformation);
      if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to Get UEFI Payload File Info! Status = %r\n", __FUNCTION__, Status));
        goto exit;
      } else {
        DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Recived Payload Information.\n", __FUNCTION__));
      }
    } else if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to Get UEFI Payload File Info! Status = %r\n", __FUNCTION__, Status));
      goto exit;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Recived Payload Information.\n", __FUNCTION__));
    }

    // Check if UEFI Payload isn't to large
    if (PayloadFileInformation->FileSize > 0xFFFFFFFF) {
      Print (L"UEFI Payload File is to Large!\n");
      Status = EFI_OUT_OF_RESOURCES;
      goto exit;
    }

    PayloadFileBufferSize = (UINTN) PayloadFileInformation->FileSize;

    // Allocate more Memory
    Status = gBS->AllocatePool(EfiLoaderData, PayloadFileBufferSize, &PayloadFileBuffer);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to allocate more Memory! Status = %r\n", __FUNCTION__, Status));
      goto exit;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Allocated Memory.\n", __FUNCTION__));
    }

    SetMem(PayloadFileBuffer, PayloadFileBufferSize,0xFF);

    // Read UEFI Payload into Memory
    Status = PayloadFileProtocol->Read(PayloadFileProtocol, &PayloadFileBufferSize, PayloadFileBuffer);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Failed to Read UEFI Payload into Memory! Status = %r\n", __FUNCTION__, Status));
      goto exit;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Successfully Read UEFI Payload into Memory.\n", __FUNCTION__));
    }
  }

  // If the UEFI Payload was not Found exit
  if (EFI_ERROR (Status)) {
    Print(L"No UEFI Payload Found!\n");
    goto exit;
  }

  // Check if ELF File is Valid
  ValiadeELFFile();
  if (ValidELFFile == FALSE) {
    Print(L"The UEFI Payload Entry is Invaild!\n");
    Status = EFI_UNSUPPORTED;
    goto exit;
  }

  // Print UEFI Payload File Infos
  Print(L"UEFI Payload in Memory at: 0x%x\n", PayloadFileBuffer);
  Print(L"UEFI Payload Size: 0x%x\n", PayloadFileBufferSize);

exit:
  return Status;
}

EFI_STATUS
PrepareElfFile()
{
  EFI_STATUS           Status               = EFI_SUCCESS;
  elf32_ehdr          *bl_elf_hdr           = PayloadFileBuffer;
  elf32_phdr          *bl_elf_phdr          = (VOID*)(((UINTN)PayloadFileBuffer) + bl_elf_hdr->e_phoff);
  UINT64               PayloadSectionOffset = 0;

  UEFIPayloadEntryPoint = bl_elf_hdr->e_entry;

  // Determine LOAD section
  for (UINTN ph_idx = 0; ph_idx < bl_elf_hdr->e_phnum; ph_idx++) {
    bl_elf_phdr = (VOID*)(((UINTN)bl_elf_phdr) + (ph_idx * sizeof(elf32_phdr)));

    // Check if it is LOAD section
    if (bl_elf_phdr->p_type != PT_LOAD) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: Section %d skipped because it is not LOAD, it is 0x%x\n", __FUNCTION__, ph_idx, bl_elf_phdr->p_type));
      continue;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: UEFI Payload has LOAD Section.\n", __FUNCTION__));
    }

    // Sanity check: PA = VA, PA = entry_point, memory size = file size
    if (bl_elf_phdr->p_paddr != bl_elf_phdr->p_vaddr) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: LOAD section %d skipped due to identity mapping vioaltion\n", __FUNCTION__, ph_idx));
      continue;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: Sanity Check Passed.\n", __FUNCTION__));
    }

    if (bl_elf_phdr->p_filesz != bl_elf_phdr->p_memsz) {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: %ELOAD section %d size inconsistent; use with caution%N\n", __FUNCTION__, ph_idx));
    }

    if (bl_elf_phdr->p_paddr != UEFIPayloadEntryPoint) {
      DEBUG((EFI_D_ERROR, "[DEBUG] %a: LOAD section %d skipped due to entry point violation\n", __FUNCTION__, ph_idx));
      continue;
    } else {
      DEBUG((EFI_D_WARN, "[DEBUG] %a: No UEFI Entry Point Violations.\n", __FUNCTION__));
    }

    PayloadSectionOffset = bl_elf_phdr->p_offset;
    UEFIPayloadLength = bl_elf_phdr->p_memsz;

    // Exit on the first result
    break;
  }

  // Check if Payload is invalid
  if (PayloadSectionOffset == 0 || UEFIPayloadLength == 0) {
    Print(L"UEFI Payload has no Suitable LOAD Section!\n");
    Status = EFI_UNSUPPORTED;
    goto exit;
  }

  // Check if Entry Point isn't located to hight
  if (UEFIPayloadEntryPoint > 0xFFFFFFFF) {
    Print(L"UEFI Payload Entry Point is Located too High!\n");
    Status = EFI_UNSUPPORTED;
    goto exit;
  } else {
    DEBUG((EFI_D_WARN, "[DEBUG] %a: UEFI Payload Entry isn't located to high.\n", __FUNCTION__));
  }

  // Print Payload Infos
  Print(L"UEFI Payload Entry Point = 0x%x\n", UEFIPayloadEntryPoint);
  Print(L"UEFI Payload Offset = 0x%x\n", PayloadSectionOffset);
  Print(L"UEFI Payload Length = 0x%x\n", UEFIPayloadLength);

exit:
  return Status;
}

EFI_STATUS
EFIAPI
LoadMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status = EFI_SUCCESS;

#ifdef FIX_CONSOLE_OUTPUT
  // Fix Console Output
  ConsoleOutputFixup();
#endif

#ifdef TEGRA30_TRUSTZONE_EXPLOIT
  // Exploit TrustZone
  Status = SecureWorldExploit();
  if (EFI_ERROR (Status)) {
    Print(L"Secure World Exploit Failed! Status = %r\n", Status);
    UEFILoaderFinish();
  }
#endif

  // Load ELF File into Memory
  Status = LoadElfFile();
  if (EFI_ERROR (Status)) {
    Print(L"Failed to put UEFI Payload into Memory! Status = %r\n", Status);
    UEFILoaderFinish();
  }

  // Prepare ELF File for Memory Boot
  Status = PrepareElfFile();
  if (EFI_ERROR (Status)) {
    Print(L"Failed to Prepare UEFI Payload for Boot! Status = %r\n", Status);
    UEFILoaderFinish();
  }

  // Jump to UEFI Payload
  ExecuteUEFIPayload(ImageHandle);

  return Status;
}

VOID
UEFILoaderFinish(VOID)
{
  Print(L"Reset your Device Manually!\n");
  CpuDeadLoop();
}
