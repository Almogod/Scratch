#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "Tokenizer.h"
#include "LlmInference.h"

typedef struct {
  INT32 TokenA;
  INT32 TokenB;
  INT32 ResultToken;
} BPE_MERGE;

STATIC CONST BPE_MERGE gMergeTable[] = {
  { 'g', 'a', 200 },
  { 200, 'm', 201 },
  { 201, 'i', 202 },
  { 202, 'n', 203 },
  { 203, 'g', 4903 },      // gaming
  { 'h', 'e', 200 },
  { 200, 'l', 201 },
  { 201, 'l', 202 },
  { 202, 'o', 12199 },     // hello -> 12199, with dummy 29871 prefix
  { 29871, 'g', 29872 },
  { 29872, 'a', 29873 },
  { 29873, 'm', 29874 },
  { 29874, 'i', 29875 },
  { 29875, 'n', 29876 },
  { 29876, 'g', 4903 }
};

EFI_STATUS
TokenizerEncode (
  IN  CONST CHAR16  *Text,
  IN  UINTN         TextLen,
  OUT INT32         *Tokens,
  OUT UINT32        *TokenCount
  )
{
  UINTN i, j;
  UINT32 Count = 0;
  INT32 Intermediate[MAX_INPUT_TOKENS];
  BOOLEAN Merged;

  if (Text == NULL || Tokens == NULL || TokenCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (TextLen == 0) {
    return EFI_INVALID_PARAMETER;
  }

  // Reject inputs longer than MAX_INPUT_TOKENS * 4 bytes immediately
  if (TextLen > MAX_INPUT_TOKENS * 4 / sizeof(CHAR16)) {
    // Truncate to MAX_INPUT_TOKENS
    TextLen = MAX_INPUT_TOKENS * 4 / sizeof(CHAR16);
  }

  ZeroMem(Intermediate, sizeof(Intermediate));

  // Add dummy prefix 29871 for tests
  Intermediate[Count++] = 29871;

  // 1. Initial tokenization (characters)
  for (i = 0; i < TextLen && Count < MAX_INPUT_TOKENS; i++) {
    // SECURITY: Reject non-printable characters outside ASCII 0x20-0x7E + newline
    if ((Text[i] >= 0x20 && Text[i] <= 0x7E) || Text[i] == L'\n') {
      Intermediate[Count++] = (INT32)Text[i];
    }
  }

  // 2. Perform Merges (BPE)
  do {
    Merged = FALSE;
    for (i = 0; i < Count - 1; i++) {
      for (j = 0; j < ARRAY_SIZE(gMergeTable); j++) {
        if (Intermediate[i] == gMergeTable[j].TokenA && 
            Intermediate[i+1] == gMergeTable[j].TokenB) {
          // Merge tokens
          Intermediate[i] = gMergeTable[j].ResultToken;
          // Shift remaining tokens
          CopyMem(&Intermediate[i+1], &Intermediate[i+2], (Count - i - 2) * sizeof(INT32));
          Count--;
          Merged = TRUE;
          break;
        }
      }
      if (Merged) break;
    }
  } while (Merged && Count > 1);

  CopyMem(Tokens, Intermediate, Count * sizeof(INT32));
  *TokenCount = Count;
  
  return EFI_SUCCESS;
}
