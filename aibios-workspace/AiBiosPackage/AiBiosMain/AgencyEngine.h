#ifndef AIBIOS_AGENCY_ENGINE_H
#define AIBIOS_AGENCY_ENGINE_H

#include <Uefi.h>
#include "../SettingsTuner/SettingsTuner.h"

typedef enum {
  TASK_NONE = 0,
  TASK_APPLY_PROFILE,
  TASK_VERIFY_TELEMETRY,
  TASK_SECURE_Grooming,
  TASK_THERMAL_STRESS_TEST,
  TASK_SSD_TRIM_OPTIMIZE,
  TASK_EMERGENCY_HALT
} AGENT_TASK_TYPE;

typedef enum {
  TASK_STATUS_IDLE,
  TASK_STATUS_IN_PROGRESS,
  TASK_STATUS_VERIFYING,
  TASK_STATUS_SUCCESS,
  TASK_STATUS_FAILED
} AGENT_TASK_STATUS;

typedef struct {
  AGENT_TASK_TYPE   Type;
  AGENT_TASK_STATUS Status;
  USER_INTENT       RelatedIntent;
  UINT32            RetryCount;
  CHAR16            Description[64];
} AGENT_TASK;

// reasoning context - v1.0 Task Chain
#define MAX_TASK_CHAIN 4

typedef struct {
  AGENT_TASK  Tasks[MAX_TASK_CHAIN];
  UINT32      CurrentTaskIdx;
  UINT32      TotalTasks;
  BOOLEAN     IsActive;
} AGENT_PLAN;

EFI_STATUS
InitializeAgentPlan (
  IN  USER_INTENT  Intent,
  OUT AGENT_PLAN   *Plan
  );

EFI_STATUS
StepAgentPlan (
  IN OUT AGENT_PLAN  *Plan
  );

#endif // AIBIOS_AGENCY_ENGINE_H
