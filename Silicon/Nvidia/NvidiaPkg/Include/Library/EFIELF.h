#ifndef _EFI_ELF_H_
#define _EFI_ELF_H_

/* ARM ELF Specific Values */
#define EM_ARM    40

typedef UINT32 Elf32_Addr;
typedef UINT16 Elf32_Half;
typedef UINT32 Elf32_Off;
typedef UINTN  Elf32_Sword;
typedef UINT32 Elf32_Word;

#define EI_NIDENT 16

typedef struct _elf32_ehdr {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;
  Elf32_Addr    e_entry;
  Elf32_Off     e_phoff;
  Elf32_Off     e_shoff;
  Elf32_Word    e_flags;
  Elf32_Half    e_ehsize;
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;
  Elf32_Half    e_shnum;
  Elf32_Half    e_shstrndx;
} elf32_ehdr;

typedef struct _elf32_phdr {
  Elf32_Word p_type;
  Elf32_Off  p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} elf32_phdr;

/* ELF Magic Headers */
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

/* ELF File Types */
#define ET_EXEC 2

#define PT_LOAD 1

#endif /* _EFI_ELF_H_ */
