#include "SettingsTuner.h"

// Real example profiles based on published OC guides:
STATIC CONST TUNING_PROFILE gLocalProfiles[] = {
  // GAMING: Max single-core, XMP on, aggressive fan
  { INTENT_GAMING,     48, +5, 3200, 2, 125, TRUE  },
  // ECO: -200mV undervolt, 65W TDP, passive fan target
  { INTENT_ECO,        35, -20, 2133, 0, 65,  FALSE },
  // SILENT: Same clocks as eco, fan curve prioritizes quiet
  { INTENT_SILENT,     38, -10, 2400, 1, 95,  FALSE },
  // VIDEO_EDIT: All-core boost, high memory bandwidth
  { INTENT_VIDEO_EDIT, 45, 0,  3200, 3, 105, TRUE  },
  // BATTERY: Minimum viable, deep C-states enabled
  { INTENT_BATTERY,    28, -25, 1866, 0, 45,  FALSE },
};

EFI_STATUS
GetProfileByIntent (
  IN  USER_INTENT      Intent,
  OUT TUNING_PROFILE   **Profile
  )
{
  UINTN i;
  if (Profile == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  for (i = 0; i < ARRAY_SIZE(gLocalProfiles); i++) {
    if (gLocalProfiles[i].Intent == Intent) {
      *Profile = (TUNING_PROFILE*)&gLocalProfiles[i];
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

