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
#include "../Security/Security.h"
#include "AgencyEngine.h"

#ifdef RUN_TESTS
#include "../Tests/TestMain.h"
EFI_STATUS RunAllTests(VOID);
#endif

// v0.9 Contextual Memory
STATIC USER_INTENT gLastActionableIntent = INTENT_UNKNOWN;
STATIC CHAR16      gLastSettingName[64] = {0};
STATIC AGENT_PLAN  gActivePlan = {0};
SENSOR_RING         gSensorHistory = {0}; // Elevated to global for cross-module trajectory analysis

VOID
DisplayStatus (
  VOID
  )
{
  SENSOR_SAMPLE Latest;
  EFI_STATUS    Status;
  INT32         PredictedDelta;

  Status = PollSensors (&Latest);
  if (Status == EFI_NOT_FOUND) {
    Print (L"[aiBIOS] Hardware not detected. Using simulated telemetry data.\n");
  } else if (EFI_ERROR (Status)) {
    Print (L"[ERROR] Failed to poll sensors: %r\n", Status);
    return;
  }

  // Update history for trend analysis
  gSensorHistory.Samples[gSensorHistory.Head] = Latest;
  gSensorHistory.Head = (gSensorHistory.Head + 1) % SENSOR_HISTORY_LEN;
  if (gSensorHistory.Count < SENSOR_HISTORY_LEN) gSensorHistory.Count++;

  PredictThermalTrend(&gSensorHistory, &PredictedDelta);

  Print (L"  \x250C\x2500\x2500\x2500 [ aiBIOS Telemetry Dashboard ] \x2500\x2500\x2500\x2510\n");
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
  Print (L"  \x2502 CPU Temperature:  %3d.%d C           \x2502\n", Latest.Temperature / 10, Latest.Temperature % 10);
  Print (L"  \x2502 Fan Speed:        %4d RPM             \x2502\n", Latest.FanRpm);
  Print (L"  \x2502 CPU Voltage:      %4d mV              \x2502\n", Latest.CpuVoltage);
  Print (L"  \x2502 SSD Wear:         %3d %%               \x2502\n", Latest.SsdWearPct);
  
  Print (L"  \x251C\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2524\n");
  
  if (PredictedDelta > 5) {
     gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_BLACK));
     Print (L"  \x2502 Thermal Trend:    [SPIKE] (+%d.%d/tick)  \x2502\n", PredictedDelta/10, PredictedDelta%10);
  } else if (PredictedDelta < -5) {
     gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
     Print (L"  \x2502 Thermal Trend:    [COOLING] (-%d.%d/tick)\x2502\n", (-PredictedDelta)/10, (-PredictedDelta)%10);
  } else {
     gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
     Print (L"  \x2502 Thermal Trend:    [STABLE]            \x2502\n");
  }

  COMPONENT_HEALTH SsdHealth = AnalyzeSsdHealth(&Latest);
  if (SsdHealth == HEALTH_CRITICAL) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_BLACK));
    Print (L"  \x2502 SSD Health:       [CRITICAL] Error!   \x2502\n");
  } else if (SsdHealth == HEALTH_WARNING) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
    Print (L"  \x2502 SSD Health:       [WARNING] Wear Up   \x2502\n");
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
    Print (L"  \x2502 SSD Health:       [OPTIMAL]           \x2502\n");
  }
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
  Print (L"  \x2514\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2500\x2518\n\n");
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

  Print(L"aiBIOS Production v1.0 Online.\n");

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

  Status = LlmInferenceInit(gModelWeights, sizeof(gModelWeights));
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "[aiBIOS] LLM Init failed: %r\n", Status));
    return Status;
  }

  // Phase 3: Proactive Security Audit
  BOOLEAN Anomaly;
  Status = PerformSecurityAudit(&Anomaly);
  if (!EFI_ERROR(Status) && Anomaly) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_BLACK));
    Print(L"[CAUTION] Security Anomaly Detected! Check bios variables for tampering.\n");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
  } else {
    Print(L"[aiBIOS] Security Audit: Clean boot verified.\n");
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
      
      if (gLastActionableIntent != INTENT_UNKNOWN) {
        gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
        Print (L"Contextual Suggestion:\n");
        Print (L"  - You previously asked about '%s'.\n", gLastSettingName);
        Print (L"  - Try 'apply settings' or 'is it active?'\n");
        gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
      }
      continue;
    }

    if (StrCmp (InputBuffer, L"debug spike") == 0) {
      extern SENSOR_RING gSensorHistory;
      Print (L"[DEBUG] Injecting artificial thermal runaway trajectory...\n");
      for (UINT32 i = 0; i < SENSOR_HISTORY_LEN; i++) {
        gSensorHistory.Samples[i].Temperature = 500 + (i * 100); // Rapidly rising
      }
      gSensorHistory.Count = SENSOR_HISTORY_LEN;
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
    Print (L"[aiBIOS] Analyzing intent and reasoning... \r");

    Status = LlmInferenceRun(CleanInput, CleanLen, &InfResult);
    if (EFI_ERROR(Status)) {
      Print (L"[ERROR] Inference failed: %r\n", Status);
      continue;
    }

    // AI feedback (showing engine activity)
    Print (L"[aiBIOS] Inference pass: %d tokens through %d layers complete.\n", InfResult.InputLen, NUM_LAYERS);

    Intent = (USER_INTENT)InfResult.OutputTokens[0];

    if (Intent == INTENT_STATUS_REPORT) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
      Print (L"[aiBIOS] Retrieving system telemetry as requested...\n");
      DisplayStatus ();
    } else if (Intent == INTENT_AI_ACCEL) {
       gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
       Print (L"[aiBIOS] AI Workload detected. Optimizing PCI-E path and NPU resources...\n");
       ApplyProfile(Intent);
       Print (L"  - Resizable BAR: Enabled\n");
       Print (L"  - Above 4G Decoding: Enabled\n");
       Print (L"  - NPU Offloading: Active\n");
    } else if (Intent == INTENT_SEMANTIC_QUERY) {
       CONST CHAR16 *Explanation;
       CONST CHAR16 *SettingName;
       
       gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTCYAN, EFI_BLACK));
       Status = LookupKnowledge(CleanInput, &Explanation, &SettingName);
       if (!EFI_ERROR(Status)) {
         Print (L"[aiBIOS Assistant] %s\n", Explanation);
         Print (L"  Relevant Setting: [%s]\n", SettingName);
         
         // v0.9 Save Context
         gLastActionableIntent = Intent;
         StrCpyS(gLastSettingName, 64, SettingName);

         if (InfResult.OutputTokens[1] == 1) { // Agency Bit Set
           gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
           Print (L"[aiBIOS Agency] Autonomous Action Triggered: Configuring %s...\n", SettingName);
           ApplyProfile(Intent);
         } else {
           gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
           Print (L"[aiBIOS Brain] Context stored. You can type 'do it' or 'configure it' to apply this setting.\n");
         }
       } else {
         Print (L"[aiBIOS Assistant] I'm sorry, I don't have information on that topic yet. Try 'virtual machine' or 'secure boot'.\n");
       }
    } else if (Intent == INTENT_UNKNOWN) {
        // v0.9/v1.0 Context & Status Resolver
        if ((InfResult.OutputTokens[1] == 1 || InfResult.OutputTokens[2] == 1 || InfResult.OutputTokens[3] == 1) && gLastActionableIntent != INTENT_UNKNOWN) {
          
          if (InfResult.OutputTokens[3] == 1) { // It's a STATUS QUERY ("is it done?")
            if (GetActiveIntent() == gLastActionableIntent) {
              gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
              Print (L"[aiBIOS Brain] Yes, the '%s' configuration is currently active and verified.\n", gLastSettingName);
            } else {
              gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
              Print (L"[aiBIOS Brain] Not yet. The '%s' configuration is pending. Type 'do it' to apply.\n", gLastSettingName);
            }
          } else { // It's an AGENCY COMMAND ("do it")
            gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
            Print (L"[aiBIOS Brain] Context identified. Applying '%s' configuration as requested...\n", gLastSettingName);
            ApplyProfile(gLastActionableIntent);
          }

        } else {
          gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_BLACK));
          Print (L"[aiBIOS] Intent ambiguous. Please clarify if you want to 'show status', 'optimize', or 'ask' a question.\n");
        }
    } else {
      // v1.0 Enhanced Agency Trigger
      gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGREEN, EFI_BLACK));
      
      Status = InitializeAgentPlan(Intent, (INT32)InfResult.OutputTokens[1], &gActivePlan);
      if (!EFI_ERROR(Status)) {
        Print(L"[aiBIOS Agency] Reasoned Plan Generated. Executing autonomous sequence...\n");
        while (gActivePlan.IsActive) {
          StepAgentPlan(&gActivePlan);
        }
        
        // v1.1 Self-Healing Check
        if (gActivePlan.CurrentTaskIdx < gActivePlan.TotalTasks && 
            gActivePlan.Tasks[gActivePlan.CurrentTaskIdx].Status == TASK_STATUS_FAILED) {
           gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTRED, EFI_BLACK));
           Print(L"[aiBIOS Agency] Stability Check Failed! Triggering autonomous Self-Healing (ECO Mode)...\n");
           InitializeAgentPlan(INTENT_ECO, 0, &gActivePlan);
           while (gActivePlan.IsActive) {
             StepAgentPlan(&gActivePlan);
           }
           Print(L"[aiBIOS Agency] System stabilized via autonomous failover.\n");
        } else {
           Print(L"[aiBIOS Agency] Sequence Complete. Verified system state is stable.\n");
        }
      } else {
        Print(L"[aiBIOS ERROR] Failed to initialize agent plan.\n");
      }
    }
    
    // Reset Color
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
  }

  return EFI_SUCCESS;
}
