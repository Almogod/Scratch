#include <Library/BaseMemoryLib.h>
#include "HardwareMonitor.h"

STATIC SENSOR_RING gSensorHistory;

VOID
AddSensorSample (
  IN SENSOR_SAMPLE *Sample
  )
{
  if (Sample == NULL) {
    return;
  }
  
  // Copy sample into ring buffer
  CopyMem(&gSensorHistory.Samples[gSensorHistory.Head], Sample, sizeof(SENSOR_SAMPLE));
  
  // Update head and count
  gSensorHistory.Head = (gSensorHistory.Head + 1) % SENSOR_HISTORY_LEN;
  if (gSensorHistory.Count < SENSOR_HISTORY_LEN) {
    gSensorHistory.Count++;
  }
}

// Example trend analysis logic (linear regression placeholder)
// O(n) time as per brief
BOOLEAN
IsTemperatureRising (
  VOID
  )
{
  if (gSensorHistory.Count < 2) return FALSE;
  
  // Simplified check: compare last two samples
  UINT32 LastIdx = (gSensorHistory.Head == 0) ? (SENSOR_HISTORY_LEN - 1) : (gSensorHistory.Head - 1);
  UINT32 PrevIdx = (LastIdx == 0) ? (SENSOR_HISTORY_LEN - 1) : (LastIdx - 1);
  
  return gSensorHistory.Samples[LastIdx].Temperature > gSensorHistory.Samples[PrevIdx].Temperature;
}
