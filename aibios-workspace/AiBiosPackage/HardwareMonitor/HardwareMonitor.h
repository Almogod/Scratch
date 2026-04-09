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

#endif // AIBIOS_HARDWARE_MONITOR_H
