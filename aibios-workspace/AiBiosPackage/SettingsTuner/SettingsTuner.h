#ifndef AIBIOS_SETTINGS_TUNER_H
#define AIBIOS_SETTINGS_TUNER_H

#include <Uefi.h>

typedef enum {
  INTENT_GAMING       = 0,
  INTENT_ECO          = 1,
  INTENT_SILENT       = 2,
  INTENT_VIDEO_EDIT   = 3,
  INTENT_BATTERY      = 4,
  INTENT_DIAGNOSTIC   = 5,
  INTENT_FAN_TUNING   = 6,
  INTENT_UNKNOWN      = 7,
  INTENT_STATUS_REPORT = 8,
  INTENT_SEMANTIC_QUERY = 9,
  INTENT_AI_ACCEL      = 10,
  INTENT_SEC_ANOMALY   = 11,
  INTENT_PREDICTIVE_COOLING = 12
} USER_INTENT;

typedef struct {
  USER_INTENT   Intent;
  UINT8         CpuMaxMultiplier;
  UINT8         CpuVoltageOffset;
  UINT16        MemoryClockMHz;
  UINT8         FanCurveId;
  UINT32        TdpWatts;
  BOOLEAN       XmpEnabled;
  BOOLEAN       VirtualizationEnabled; // v0.8
} TUNING_PROFILE;

EFI_STATUS
ApplyProfile (
  IN USER_INTENT Intent
  );

USER_INTENT
EFIAPI
GetActiveIntent (
  VOID
  );


EFI_STATUS
SafeWriteMsr (
  IN UINT32  MsrAddress,
  IN UINT64  Value
  );

EFI_STATUS
TuneHardwareParameter (
  IN UINT32  Type,
  IN INT32   Value
  );

INT32
GetSimulatedFanSpeed (
  VOID
  );

#endif // AIBIOS_SETTINGS_TUNER_H
