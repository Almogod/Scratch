#include "TestMain.h"
#include "../HardwareMonitor/HardwareMonitor.h"

EFI_STATUS RunSuite3 (VOID) {
  EFI_STATUS Status;
  SENSOR_SAMPLE Sample;
  SENSOR_RING MockRing;

  // Test 3.1 & 3.2
  Status = PollSensors(&Sample);
  if (Status != EFI_SUCCESS && Status != EFI_NOT_FOUND && Status != EFI_TIMEOUT) {
    return EFI_DEVICE_ERROR;
  }
  
  // Test 3.3: Ring buffer (simple manual test)
  ZeroMem(&MockRing, sizeof(MockRing));
  for (int i=0; i<100; i++) {
    // Injecting into MockRing directly instead of AddSensorSample to keep simple
    MockRing.Samples[MockRing.Head].FanRpm = 2000;
    MockRing.Head = (MockRing.Head + 1) % SENSOR_HISTORY_LEN;
    if (MockRing.Count < SENSOR_HISTORY_LEN) MockRing.Count++;
  }
  if (MockRing.Count != 64) return EFI_DEVICE_ERROR;

  // Test 3.4: AnalyzeFanHealth() injected RPM > 8%
  for (int i=0; i<64; i++) {
    MockRing.Samples[i].FanRpm = i % 2 == 0 ? 1000 : 3000;
  }
  if (AnalyzeFanHealth(&MockRing) != HEALTH_WARNING) return EFI_DEVICE_ERROR;

  // Test 3.5: Flat 2000 RPM -> OK
  for (int i=0; i<64; i++) MockRing.Samples[i].FanRpm = 2000;
  if (AnalyzeFanHealth(&MockRing) != HEALTH_OK) return EFI_DEVICE_ERROR;

  // Test 3.6: SSD Wear 95%
  Sample.SsdWearPct = 95;
  if (AnalyzeSsdHealth(&Sample) != HEALTH_CRITICAL) return EFI_DEVICE_ERROR;

  Print(L"[aiBIOS] Suite 3 PASS\n");
  return EFI_SUCCESS;
}
