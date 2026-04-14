#ifndef AIBIOS_HARDWARE_MONITOR_H
#define AIBIOS_HARDWARE_MONITOR_H

#include <Uefi.h>

// Ring buffer for sensor history — O(1) insert, O(n) trend scan
#define SENSOR_HISTORY_LEN  64

typedef enum {
  HEALTH_OK       = 0,
  HEALTH_WARNING  = 1,
  HEALTH_CRITICAL = 2
} COMPONENT_HEALTH;


typedef struct {
  UINT16  Temperature;    // Celsius * 10 (e.g., 725 = 72.5°C)
  UINT16  FanRpm;
  UINT32  CpuVoltage;     // mV
  UINT32  SsdWearPct;     // 0–100
  UINT64  Timestamp;      // UEFI GetTime() ticks
} SENSOR_SAMPLE;

typedef struct {
  SENSOR_SAMPLE  Samples[SENSOR_HISTORY_LEN];
  UINT32         Head;    // Next write position (ring buffer)
  UINT32         Count;   // Valid samples filled so far
} SENSOR_RING;

EFI_STATUS
PollSensors (
  OUT SENSOR_SAMPLE  *Latest
  );

COMPONENT_HEALTH
AnalyzeFanHealth (
  IN CONST SENSOR_RING  *History
  );

COMPONENT_HEALTH
AnalyzeSsdHealth (
  IN CONST SENSOR_SAMPLE *Latest
  );

EFI_STATUS
PredictThermalTrend (
  IN  CONST SENSOR_RING  *History,
  OUT INT32              *PredictedDelta
  );

EFI_STATUS
SetFanSpeed (
  IN UINT16  Rpm
  );

EFI_STATUS
VerifyHardwareState (
  IN UINT32  TaskId
  );

BOOLEAN
IsThermalCritical (
  IN  CONST SENSOR_RING  *History,
  OUT INT16              *Confidence
  );

#endif // AIBIOS_HARDWARE_MONITOR_H
