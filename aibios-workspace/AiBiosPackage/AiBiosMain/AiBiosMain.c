#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "AiBiosMain.h"
#include "../LlmInference/LlmInference.h"
#include "../LlmInference/ModelWeights.h"

#include "../SettingsTuner/SettingsTuner.h"
#include "../NlInterface/NlInterface.h"

#ifdef RUN_TESTS
#include "../Tests/TestMain.h"
EFI_STATUS RunAllTests(VOID);
#endif

EFI_STATUS
EFIAPI
AiBiosMainEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  CHAR16      InputBuffer[MAX_RAW_INPUT_CHARS + 1];
  CHAR16      CleanInput[MAX_RAW_INPUT_CHARS + 1];
  UINTN       CleanLen;
  INFERENCE_RESULT InfResult;
  USER_INTENT Intent;

  Print(L"aiBIOS Prototype v0.1 Online.\n");

#ifdef RUN_TESTS
  Print(L"[aiBIOS] Test Mode Enabled. Running Suites...\n");
  Status = RunAllTests();
  if (EFI_ERROR(Status)) {
    return Status;
  }
#endif

  // Verify model weights are present
  if (gModelWeights == NULL) {
    DEBUG ((DEBUG_ERROR, "[aiBIOS] Model weights missing!\n"));
    return EFI_NOT_FOUND;
  }

  // Initialize LLM Engine
  Status = LlmInferenceInit(gModelWeights, sizeof(gModelWeights));
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "[aiBIOS] LLM Init failed: %r\n", Status));
    return Status;
  }

  Print(L"NL Interface ready. Enter command: ");

  // 1. Read Input (Mock for DXE driver entry — in real BIOS, this would be a prompt)
  // For prototype, we'll demonstrate a single pass:
  StrCpyS(InputBuffer, MAX_RAW_INPUT_CHARS, L"Maximize gaming performance");
  Print(L"%s\n", InputBuffer);

  // 2. Sanitize
  Status = SanitizeInput(InputBuffer, CleanInput, &CleanLen);
  if (EFI_ERROR(Status)) return Status;

  // 3. Inference
  Status = LlmInferenceRun(CleanInput, CleanLen, &InfResult);
  if (EFI_ERROR(Status)) return Status;

  // 4. Parse Intent & Apply
  // (In the stub, LlmInferenceRun sets OutputTokens[0] to 0 for GAMING)
  Intent = (USER_INTENT)InfResult.OutputTokens[0];
  Status = ApplyProfile(Intent);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to apply profile: %r\n", Status);
  } else {
    Print(L"Gaming profile applied. Boost enabled.\n");
  }

  return EFI_SUCCESS;
}
