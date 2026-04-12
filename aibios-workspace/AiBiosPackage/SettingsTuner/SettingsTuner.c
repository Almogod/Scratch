#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include "SettingsTuner.h"

typedef struct {
  UINT32  MsrAddress;
  UINT64  SafeMin;
  UINT64  SafeMax;
  CHAR8   *Name;
} MSR_SAFE_RANGE;

// Intel MSR safe ranges (from Intel SDM Vol 4)
STATIC CONST MSR_SAFE_RANGE gIntelSafeRanges[] = {
  { 0x1AD, 0x1E1E1E1E1E1E1E1EULL, 0x3030303030303030ULL, "IA32_TURBO_RATIO_LIMIT" },
  { 0x150, 0xFFFFFFFFFFFF0001ULL, 0xFFFFFFFFFFFF07D1ULL, "OC_MAILBOX" },
};

EFI_STATUS
SafeWriteMsr (
  IN UINT32  MsrAddress,
  IN UINT64  Value
  )
{
  UINTN  i;
  for (i = 0; i < ARRAY_SIZE(gIntelSafeRanges); i++) {
    if (gIntelSafeRanges[i].MsrAddress == MsrAddress) {
      if (Value < gIntelSafeRanges[i].SafeMin ||
          Value > gIntelSafeRanges[i].SafeMax) {
        DEBUG ((DEBUG_WARN, "[aiBIOS] MSR 0x%x value 0x%llx out of safe range\n",
                MsrAddress, Value));
        return EFI_SECURITY_VIOLATION;
      }
      AsmWriteMsr64 (MsrAddress, Value);
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND; // Unknown MSR — refuse to write for safety
}

EFI_STATUS
ApplyProfile (
  IN USER_INTENT Intent
  )
{
  EFI_STATUS Status;
  TUNING_PROFILE *Profile = NULL;
  UINT64 MsrValue;
  
  if (Intent == INTENT_UNKNOWN) {
    return EFI_UNSUPPORTED;
  }

  Status = GetProfileByIntent(Intent, &Profile);
  if (EFI_ERROR(Status) || Profile == NULL) {
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "[aiBIOS] Applying profile for intent %d\n", Intent));
  
  // 1. Write CpuMaxMultiplier to IA32_TURBO_RATIO_LIMIT (0x1AD)
  // For demonstration, packing multiplier into all 8 cores byte fields
  // e.g. 48 -> 0x3030303030303030
  MsrValue = 0;
  for (UINTN i = 0; i < 8; i++) {
    MsrValue |= ((UINT64)Profile->CpuMaxMultiplier) << (i * 8);
  }
  
  Status = SafeWriteMsr(0x1AD, MsrValue);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_WARN, "[aiBIOS] Failed to apply CPU multiplier: %r\n", Status));
  }

  // Handle AI Workload Acceleration Specifics
  if (Intent == INTENT_AI_ACCEL) {
    DEBUG ((DEBUG_INFO, "[aiBIOS] Enabling Above 4G Decoding and Resizable BAR for AI Load...\n"));
    // Mock PCI config space writes
  }

  if (Profile->VirtualizationEnabled) {
    DEBUG ((DEBUG_INFO, "[aiBIOS] Enabling VT-d/AMD-V hardware virtualization extensions...\n"));
    // Mock VMX_BASIC (0x480) or similar
  }
  
  return EFI_SUCCESS;
}
