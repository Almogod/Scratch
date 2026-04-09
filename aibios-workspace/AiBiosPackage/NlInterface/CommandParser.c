#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include "SettingsTuner.h"

USER_INTENT
ParseIntent (
  IN CONST CHAR16 *LlmOutput
  )
{
  if (LlmOutput == NULL) {
    return INTENT_UNKNOWN;
  }

  // Simple string matching for the classification token.
  // The LLM is prompted to output ONLY the token (e.g., "GAMING").
  if (StrCmp (LlmOutput, L"GAMING") == 0) {
    return INTENT_GAMING;
  }
  if (StrCmp (LlmOutput, L"ECO") == 0) {
    return INTENT_ECO;
  }
  if (StrCmp (LlmOutput, L"SILENT") == 0) {
    return INTENT_SILENT;
  }
  if (StrCmp (LlmOutput, L"VIDEO_EDIT") == 0) {
    return INTENT_VIDEO_EDIT;
  }
  if (StrCmp (LlmOutput, L"BATTERY") == 0) {
    return INTENT_BATTERY;
  }
  if (StrCmp (LlmOutput, L"DIAGNOSTIC") == 0) {
    return INTENT_DIAGNOSTIC;
  }

  return INTENT_UNKNOWN;
}
