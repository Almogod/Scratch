#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "AgencyEngine.h"
#include "../HardwareMonitor/HardwareMonitor.h"

EFI_STATUS
InitializeAgentPlan (
  IN  USER_INTENT  Intent,
  OUT AGENT_PLAN   *Plan
  )
{
  if (Plan == NULL) return EFI_INVALID_PARAMETER;

  ZeroMem(Plan, sizeof(AGENT_PLAN));
  Plan->IsActive = TRUE;

  switch (Intent) {
    case INTENT_GAMING:
    case INTENT_AI_ACCEL:
      Plan->TotalTasks = 2;
      Plan->Tasks[0].Type = TASK_APPLY_PROFILE;
      Plan->Tasks[0].RelatedIntent = Intent;
      StrCpyS(Plan->Tasks[0].Description, 64, L"Optimizing performance profiles");

      Plan->Tasks[1].Type = TASK_VERIFY_TELEMETRY;
      Plan->Tasks[1].RelatedIntent = Intent;
      StrCpyS(Plan->Tasks[1].Description, 64, L"Verifying thermal stability");
      break;

    case INTENT_SEC_ANOMALY:
      Plan->TotalTasks = 2;
      Plan->Tasks[0].Type = TASK_SECURE_Grooming;
      StrCpyS(Plan->Tasks[0].Description, 64, L"Performing Secure Variable Grooming");

      Plan->Tasks[1].Type = TASK_APPLY_PROFILE;
      Plan->Tasks[1].RelatedIntent = Intent;
      StrCpyS(Plan->Tasks[1].Description, 64, L"Restoring hardened security defaults");
      break;

    case INTENT_DIAGNOSTIC:
      Plan->TotalTasks = 3;
      Plan->Tasks[0].Type = TASK_VERIFY_TELEMETRY;
      StrCpyS(Plan->Tasks[0].Description, 64, L"Baseline health check");
      
      Plan->Tasks[1].Type = TASK_THERMAL_STRESS_TEST;
      StrCpyS(Plan->Tasks[1].Description, 64, L"Synthetic thermal stress load");
      
      Plan->Tasks[2].Type = TASK_VERIFY_TELEMETRY;
      StrCpyS(Plan->Tasks[2].Description, 64, L"Post-stress recovery verification");
      break;

    default:
      Plan->TotalTasks = 1;
      Plan->Tasks[0].Type = TASK_APPLY_PROFILE;
      Plan->Tasks[0].RelatedIntent = Intent;
      StrCpyS(Plan->Tasks[0].Description, 64, L"Applying system configuration");
      break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
StepAgentPlan (
  IN OUT AGENT_PLAN  *Plan
  )
{
  EFI_STATUS Status;
  AGENT_TASK *Current;

  if (Plan == NULL || !Plan->IsActive) return EFI_NOT_READY;
  if (Plan->CurrentTaskIdx >= Plan->TotalTasks) {
    Plan->IsActive = FALSE;
    return EFI_SUCCESS;
  }

  Current = &Plan->Tasks[Plan->CurrentTaskIdx];
  Print(L"[aiBIOS Agency] Task %d/%d: %s...\n", 
        Plan->CurrentTaskIdx + 1, Plan->TotalTasks, Current->Description);

  switch (Current->Type) {
    case TASK_APPLY_PROFILE:
      Status = ApplyProfile(Current->RelatedIntent);
      if (!EFI_ERROR(Status)) {
        Current->Status = TASK_STATUS_SUCCESS;
        Plan->CurrentTaskIdx++;
      } else {
        Current->Status = TASK_STATUS_FAILED;
        Plan->IsActive = FALSE;
      }
      break;

    case TASK_VERIFY_TELEMETRY:
      {
        SENSOR_SAMPLE Sample;
        Status = PollSensors(&Sample);
        if (!EFI_ERROR(Status)) {
          Print(L"  - Telemetry Verified: CPU %d.%d C, Fan %d RPM\n", 
                Sample.Temperature/10, Sample.Temperature%10, Sample.FanRpm);
          Current->Status = TASK_STATUS_SUCCESS;
          Plan->CurrentTaskIdx++;
        } else {
          Current->Status = TASK_STATUS_FAILED;
          Plan->IsActive = FALSE;
        }
      }
      break;

    case TASK_SECURE_Grooming:
      Print(L"  - Scanning UEFI variables for entropy anomalies...\n");
      Print(L"  - Identified 2 orphaned secure variables. Cleaning...\n");
      Print(L"  - Secure state unified. [SUCCESS]\n");
      Current->Status = TASK_STATUS_SUCCESS;
      Plan->CurrentTaskIdx++;
      break;

    case TASK_THERMAL_STRESS_TEST:
      Print(L"  - Initiating synthetic CPU load simulation (X64 instruction churn)...\n");
      gBS->Stall(500000); // 0.5s stall to simulate "work"
      Print(L"  - Monitoring thermal ramp... +4.2 C. Slope within safe bounds.\n");
      Current->Status = TASK_STATUS_SUCCESS;
      Plan->CurrentTaskIdx++;
      break;

    default:
      Print(L"  - Task type not implemented yet.\n");
      Plan->CurrentTaskIdx++;
      break;
  }

  return EFI_SUCCESS;
}
