#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "AgencyEngine.h"
#include "../HardwareMonitor/HardwareMonitor.h"
#include "../Security/Security.h"

EFI_STATUS ScanAndCleanVariables (OUT UINT32 *CleanedCount); // Forward decl

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

  // Proactive Trajectory Check before every task
  INT16 Confidence;
  extern SENSOR_RING gSensorHistory; 
  if (IsThermalCritical(&gSensorHistory, &Confidence)) {
    Print(L"  [CRITICAL] Thermal Trajectory Alert (Confidence %d%%)!\n", Confidence);
    Print(L"  - Forcing Emergency System Halt for hardware protection...\n");
    gBS->Stall(500000); 
    CpuDeadLoop(); // Emergency Halt
  }

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
      {
        UINT32 Cleaned;
        Print(L"  - Executing real-time firmware variable scan...\n");
        Status = ScanAndCleanVariables(&Cleaned);
        if (!EFI_ERROR(Status)) {
          Print(L"  - Grooming Complete: Removed %d orphaned/suspicious variables.\n", Cleaned);
          Current->Status = TASK_STATUS_SUCCESS;
          Plan->CurrentTaskIdx++;
        } else {
          Print(L"  - Grooming Failed: %r\n", Status);
          Current->Status = TASK_STATUS_FAILED;
          Plan->IsActive = FALSE;
        }
      }
      break;

    case TASK_THERMAL_STRESS_TEST:
      {
        SENSOR_SAMPLE Pre, Post;
        UINT32 Junk = 0x12345678;
        
        Print(L"  - Taking baseline thermal reading...\n");
        PollSensors(&Pre);
        
        Print(L"  - Running IA32 computational stress pass (500ms)...\n");
        
        // Use MicroSecondDelay for predictable duration across different CPUs
        for (UINT32 Step = 0; Step < 5; Step++) {
           Print(L"    [Stress %d%%] Churning arithmetic units...\r", (Step+1)*20);
           // Real CPU churn: 1M iterations of fixed-point arithmetic per step
           for (UINTN i = 0; i < 1000000; i++) {
             Junk = (Junk * 1103515245 + 12345) & 0x7FFFFFFF;
           }
           gBS->Stall(100000); // 100ms delay per step (100,000 us)
        }
        Print(L"\n");
        
        PollSensors(&Post);
        INT32 Delta = (INT32)Post.Temperature - (INT32)Pre.Temperature;
        
        Print(L"  - Thermal Delta: +%d.%d C\n", Delta/10, Delta%10);
        
        if (Delta < 100) { // Safety Threshold: 10.0 C
          Print(L"  - Verification Success: Delta within stability limits.\n");
          Current->Status = TASK_STATUS_SUCCESS;
          Plan->CurrentTaskIdx++;
        } else {
          Print(L"  - Verification Failed: Thermal delta too high (%d.%d C)!\n", Delta/10, Delta%10);
          Current->Status = TASK_STATUS_FAILED;
          Plan->IsActive = FALSE;
        }
      }
      break;

    default:
      Print(L"  - Task type not implemented yet.\n");
      Plan->CurrentTaskIdx++;
      break;
  }

  return EFI_SUCCESS;
}
