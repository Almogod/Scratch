#ifndef AIBIOS_NL_INTERFACE_H
#define AIBIOS_NL_INTERFACE_H

#include <Uefi.h>

#define MAX_RAW_INPUT_CHARS   256

EFI_STATUS
SanitizeInput (
  IN  CONST CHAR16  *RawInput,
  OUT CHAR16        *CleanInput,
  OUT UINTN         *CleanLen
  );

EFI_STATUS
GetTextInput (
  OUT CHAR16        *Buffer,
  IN  UINTN         MaxLength
  );

EFI_STATUS
LookupKnowledge (
  IN  CONST CHAR16  *Query,
  OUT CONST CHAR16  **Explanation,
  OUT CONST CHAR16  **SettingName
  );

#endif // AIBIOS_NL_INTERFACE_H
