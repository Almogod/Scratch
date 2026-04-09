#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "AiBiosMain.h"
#include "../LlmInference/LlmInference.h"
#include "../LlmInference/ModelWeights.h"

#include "../HardwareMonitor/HardwareMonitor.h"
#include "../SettingsTuner/SettingsTuner.h"
#include "../NlInterface/NlInterface.h"

#ifdef RUN_TESTS
#include "../Tests/TestMain.h"
EFI_STATUS RunAllTests(VOID);
#endif

VOID
DisplayStatus (
  VOID
  )
{
  SENSOR_SAMPLE Latest;
  EFI_STATUS    Status;

  Status = PollSensors (&Latest);
  if (EFI_ERROR (Status)) {
    Print (L"[ERROR] Failed to poll sensors: %r\n", Status);
    return;
  }

  Print (L"\n--- [aiBIOS Telemetry Dashboard] ---\n");
  Print (L"  CPU Temperature: %d.%d C\n", Latest.Temperature / 10, Latest.Temperature % 10);
  Print (L"  Fan Speed:       %d RPM\n", Latest.FanRpm);
  Print (L"  CPU Voltage:     %d mV\n", Latest.CpuVoltage);
  Print (L"  SSD Wear:        %d %%\n", Latest.SsdWearPct);
  Print (L"------------------------------------\n\n");
}

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

  Print(L"aiBIOS Prototype v0.2 Online.\n");

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

  Print(L"Type 'help' for commands, 'exit' to quit.\n\n");

  while (TRUE) {
    // Set Prompt Color (Cyan)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
    Print(L"aiBIOS> ");
    // Set Input Color (White)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));

    Status = GetTextInput (InputBuffer, MAX_RAW_INPUT_CHARS);
    if (EFI_ERROR (Status)) continue;

    if (StrCmp (InputBuffer, L"exit") == 0 || StrCmp (InputBuffer, L"quit") == 0) {
      break;
    }

    if (StrCmp (InputBuffer, L"help") == 0) {
      Print (L"Commands:\n");
      Print (L"  status - Show hardware telemetry\n");
      Print (L"  help   - Show this help\n");
      Print (L"  exit   - Exit aiBIOS\n");
      Print (L"  <text> - Ask AI to optimize your system\n");
      continue;
    }

    if (StrCmp (InputBuffer, L"status") == 0) {
      DisplayStatus ();
      continue;
    }

    if (StrLen (InputBuffer) == 0) continue;

    // AI Processing
    Status = SanitizeInput(InputBuffer, CleanInput, &CleanLen);
    if (EFI_ERROR(Status)) continue;

    // Show "thinking" color (Yellow)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
    Print (L"[aiBIOS] Thinking...\r");

    Status = LlmInferenceRun(CleanInput, CleanLen, &InfResult);
    if (EFI_ERROR(Status)) {
      Print (L"[ERROR] Inference failed: %r\n", Status);
      continue;
    }

    Intent = (USER_INTENT)InfResult.OutputTokens[0];

    if (Intent == INTENT_STATUS_REPORT) {
      // Info Response Color (Cyan)
      gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
      Print (L"[aiBIOS] Retrieving system telemetry as requested...\n");
      DisplayStatus ();
    } else if (Intent == INTENT_UNKNOWN) {
       gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
       Print (L"[aiBIOS] Intent ambiguous. Please clarify if you want to 'show status' or 'optimize'.\n");
    } else {
      // AI Response Color (Green)
      gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));

      Status = ApplyProfile(Intent);
      if (EFI_ERROR(Status)) {
        Print(L"[aiBIOS] Failed to apply profile: %r\n", Status);
      } else {
        if (Intent == INTENT_GAMING) {
          Print(L"[aiBIOS] Performance boost applied. Thermal limits increased.\n");
        } else if (Intent == INTENT_BATTERY) {
          Print(L"[aiBIOS] Power saving mode active. Undervolting applied.\n");
        } else {
          Print(L"[aiBIOS] Profile updated successfully.\n");
        }
      }
    }
    
    // Reset Color
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
  }

  return EFI_SUCCESS;
}
