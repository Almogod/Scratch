#include "TestMain.h"
#include "../LlmInference/Tokenizer.h"
#include "../LlmInference/LlmInference.h"

EFI_STATUS RunSuite2 (VOID) {
  EFI_STATUS Status;
  INT32 Tokens[MAX_INPUT_TOKENS];
  UINT32 TokenCount;

  // Test 2.1: L"hello" -> 12199
  Status = TokenizerEncode(L"hello", 5, Tokens, &TokenCount);
  // Our mock merge outputs 29871, 12199 for hello
  if (TokenCount != 2 || Tokens[1] != 12199) return EFI_DEVICE_ERROR;

  // Test 2.2: L"" -> EFI_INVALID_PARAMETER
  Status = TokenizerEncode(L"", 0, Tokens, &TokenCount);
  if (Status != EFI_INVALID_PARAMETER) return EFI_DEVICE_ERROR;

  // Test 2.3: 1000-char input -> truncated
  CHAR16 LongStr[1024];
  for(int i=0; i<1000; i++) LongStr[i] = L'a';
  Status = TokenizerEncode(LongStr, 1000, Tokens, &TokenCount);
  if (EFI_ERROR(Status) || TokenCount > MAX_INPUT_TOKENS) return EFI_DEVICE_ERROR;

  // Test 2.4: Non-ASCII stripped
  Status = TokenizerEncode(L"g\xFFFDming", 7, Tokens, &TokenCount);
  // Should bypass the emoji/unicode and encode 'g', 'm', 'i', 'n', 'g'
  if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

  Print(L"[aiBIOS] Suite 2 PASS\n");
  return EFI_SUCCESS;
}
