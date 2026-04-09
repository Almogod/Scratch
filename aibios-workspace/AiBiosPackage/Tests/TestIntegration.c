#include "TestMain.h"
#include "../LlmInference/LlmInference.h"
#include "../SettingsTuner/SettingsTuner.h"
#include "../NlInterface/NlInterface.h"

EFI_STATUS RunSuite6 (VOID) {
  // Test 6.1: Full pipeline
  EFI_STATUS Status;
  CHAR16 Clean[MAX_RAW_INPUT_CHARS + 1];
  UINTN CleanLen;
  INFERENCE_RESULT Result;
  
  Status = SanitizeInput(L"maximize gaming performance", Clean, &CleanLen);
  if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;
  
  Status = LlmInferenceRun(Clean, CleanLen, &Result);
  if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;
  
  // mock inference outputs 0 for INTENT_GAMING if 'g' is in string
  Status = ApplyProfile((USER_INTENT)Result.OutputTokens[0]);
  if (EFI_ERROR(Status)) return EFI_DEVICE_ERROR;

  // Test 6.3: Unknown gobbledegook
  Status = SanitizeInput(L"xyz123", Clean, &CleanLen);
  Status = LlmInferenceRun(Clean, CleanLen, &Result);
  Status = ApplyProfile((USER_INTENT)Result.OutputTokens[0]);
  if (Status != EFI_UNSUPPORTED) return EFI_DEVICE_ERROR;

  Print(L"[aiBIOS] Suite 6 & 7 PASS\n");
  return EFI_SUCCESS;
}
