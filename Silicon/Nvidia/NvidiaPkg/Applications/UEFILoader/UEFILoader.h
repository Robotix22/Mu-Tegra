#ifndef _UEFI_LOADER_H_
#define _UEFI_LOADER_H_

#ifdef FIX_CONSOLE_OUTPUT
VOID
ConsoleOutputFixup(VOID);
#endif

#ifdef TEGRA30_TRUSTZONE_EXPLOIT
EFI_STATUS
SecureWorldExploit();
#endif

VOID
UEFILoaderFinish(VOID);

#endif /* _UEFI_LOADER_H_ */
