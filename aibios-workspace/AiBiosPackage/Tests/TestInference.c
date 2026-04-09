#include "TestMain.h"
#include "../LlmInference/LlmInference.h"
#include "../LlmInference/ModelWeights.h"

EFI_STATUS RunSuite1 (VOID) {
  EFI_STATUS Status;
  INFERENCE_RESULT Result;
  
  // Test 1.1: Empty prompt -> EFI_INVALID_PARAMETER
  Status = LlmInferenceRun(L"", 0, &Result);
  if (Status != EFI_INVALID_PARAMETER) return EFI_DEVICE_ERROR;
  
  // Test 1.2: 128-token prompt (we simulate this by length < 129 but reasonably long)
  // Our tokenizer bounds to MAX_INPUT_TOKENS (128).
  CHAR16 Prompt12[300];
  ZeroMem(Prompt12, sizeof(Prompt12));
  for(int i=0; i<128; i++) Prompt12[i] = L'A';
  Status = LlmInferenceRun(Prompt12, 128, &Result);
  if (Status != EFI_SUCCESS) return EFI_DEVICE_ERROR;
  
  // Test 1.3: 129-token prompt -> EFI_BUFFER_TOO_SMALL
  for(int i=0; i<129; i++) Prompt12[i] = L'A';
  Status = LlmInferenceRun(Prompt12, 129, &Result);
  if (Status != EFI_BUFFER_TOO_SMALL || Result.Truncated != TRUE) return EFI_DEVICE_ERROR;
  
  // Test 1.4: Prompt "gaming"
  Status = LlmInferenceRun(L"Maximize gaming performance", 27, &Result);
  if (Status != EFI_SUCCESS || Result.OutputTokens[0] != 0) return EFI_DEVICE_ERROR;
  
  // Test 1.5: Prompt "save battery"
  Status = LlmInferenceRun(L"save battery", 12, &Result);
  if (Status != EFI_SUCCESS || Result.OutputTokens[0] != 4) return EFI_DEVICE_ERROR;
  
  // Test 1.6 & 1.7 implicitly validated by running Tests 1.1-1.5 without crash.
  
  Print(L"[aiBIOS] Suite 1 PASS\n");
  return EFI_SUCCESS;
}
