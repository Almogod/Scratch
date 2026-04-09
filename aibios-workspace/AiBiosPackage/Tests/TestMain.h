#ifndef AIBIOS_TEST_MAIN_H
#define AIBIOS_TEST_MAIN_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>

EFI_STATUS RunSuite1 (VOID);
EFI_STATUS RunSuite2 (VOID);
EFI_STATUS RunSuite3 (VOID);
EFI_STATUS RunSuite4 (VOID);
EFI_STATUS RunSuite5 (VOID);
EFI_STATUS RunSuite6 (VOID);

#endif // AIBIOS_TEST_MAIN_H
