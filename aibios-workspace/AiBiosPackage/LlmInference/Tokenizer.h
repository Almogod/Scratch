#ifndef AIBIOS_TOKENIZER_H
#define AIBIOS_TOKENIZER_H

#include <Uefi.h>
#include "LlmInference.h"

EFI_STATUS
TokenizerEncode (
  IN  CONST CHAR16  *Text,
  IN  UINTN         TextLen,
  OUT INT32         *Tokens,
  OUT UINT32        *TokenCount
  );

#endif
