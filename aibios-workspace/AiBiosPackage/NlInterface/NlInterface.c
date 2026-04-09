#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "NlInterface.h"

// SECURITY: This is the attack surface — treat all user input as untrusted
EFI_STATUS
SanitizeInput (
  IN  CONST CHAR16  *RawInput,
  OUT CHAR16        *CleanInput,    // Caller provides MAX_RAW_INPUT_CHARS+1 buffer
  OUT UINTN         *CleanLen
  )
{
  UINTN  i, OutIdx;
  CHAR16 Ch;

  if (RawInput == NULL || CleanInput == NULL || CleanLen == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (CleanInput, (MAX_RAW_INPUT_CHARS + 1) * sizeof (CHAR16));
  OutIdx = 0;

  for (i = 0; RawInput[i] != L'\0' && i < MAX_RAW_INPUT_CHARS; i++) {
    Ch = RawInput[i];

    // Allow only printable ASCII + space + common punctuation
    // Reject: control characters, null bytes embedded in string,
    //         Unicode private-use area, surrogate pairs
    if (Ch < 0x0020 || Ch > 0x007E) {
      // Silently strip — do not return error (prevents timing oracle)
      continue;
    }
    
    // Test 5.4: Input "'; DROP TABLE--" -> sanitized to "DROP TABLE"
    if (Ch == L'\'' || Ch == L';' || Ch == L'-') {
      continue;
    }
    
    CleanInput[OutIdx++] = Ch;
  }

  CleanInput[OutIdx] = L'\0';
  *CleanLen = OutIdx;
  
  DEBUG ((DEBUG_INFO, "[aiBIOS] Sanitized input: %s (len %d)\n", CleanInput, *CleanLen));
  
  return EFI_SUCCESS;
}

EFI_STATUS
GetTextInput (
  OUT CHAR16        *Buffer,
  IN  UINTN         MaxLength
  )
{
  EFI_STATUS    Status;
  UINTN         Index;
  EFI_INPUT_KEY Key;
  CHAR16        Echo[2];

  if (Buffer == NULL || MaxLength == 0) return EFI_INVALID_PARAMETER;

  Index = 0;
  Echo[1] = L'\0';

  while (Index < MaxLength - 1) {
    UINTN EventIndex;
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (EFI_ERROR (Status)) continue;

    if (Key.UnicodeChar == L'\r' || Key.UnicodeChar == L'\n') {
      Print (L"\n");
      break;
    }

    if (Key.UnicodeChar == L'\b') {
      if (Index > 0) {
        Index--;
        // Visual backspace: go back, print space, go back
        Print (L"\b \b");
      }
      continue;
    }

    if (Key.UnicodeChar >= L' ') {
      Buffer[Index++] = Key.UnicodeChar;
      Echo[0] = Key.UnicodeChar;
      Print (L"%s", Echo);
    }
  }

  Buffer[Index] = L'\0';
  return EFI_SUCCESS;
}
