#include "TestMain.h"
#include "../SettingsTuner/SettingsTuner.h"

EFI_STATUS RunSuite4 (VOID) {
  EFI_STATUS Status;

  // Test 4.1 & 4.4: All 5 profiles applied.
  // ApplyProfile uses SafeWriteMsr under the hood.
  Status = ApplyProfile(INTENT_GAMING);
  if (Status != EFI_SUCCESS) return EFI_DEVICE_ERROR;

  Status = ApplyProfile(INTENT_ECO);
  if (Status != EFI_SUCCESS) return EFI_DEVICE_ERROR;

  Status = ApplyProfile(INTENT_SILENT);
  if (Status != EFI_SUCCESS) return EFI_DEVICE_ERROR;
  
  Status = ApplyProfile(INTENT_VIDEO_EDIT);
  if (Status != EFI_SUCCESS) return EFI_DEVICE_ERROR;
  
  Status = ApplyProfile(INTENT_BATTERY);
  if (Status != EFI_SUCCESS) return EFI_DEVICE_ERROR;

  // Test 4.2: Inject out-of-range MSR
  // SafeWriteMsr 0x1AD with bad value
  Status = SafeWriteMsr(0x1AD, 0xFFFFFFFFFFFFFFFFULL);
  if (Status != EFI_SECURITY_VIOLATION) return EFI_DEVICE_ERROR;

  // Test 4.3: INTENT_UNKNOWN
  Status = ApplyProfile(INTENT_UNKNOWN);
  if (Status != EFI_UNSUPPORTED) return EFI_DEVICE_ERROR;

  Print(L"[aiBIOS] Suite 4 PASS\n");
  return EFI_SUCCESS;
}
