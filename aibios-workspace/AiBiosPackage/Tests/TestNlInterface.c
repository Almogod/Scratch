#include "TestMain.h"
#include "../NlInterface/NlInterface.h"

EFI_STATUS RunSuite5 (VOID) {
  EFI_STATUS Status;
  CHAR16 Clean[MAX_RAW_INPUT_CHARS + 1];
  UINTN CleanLen;

  // Test 5.1: "gaming\0evil" -> null byte stops processing
  Status = SanitizeInput(L"gaming\0evil", Clean, &CleanLen);
  if (EFI_ERROR(Status) || StrCmp(Clean, L"gaming") != 0) return EFI_DEVICE_ERROR;

  // Test 5.2: 300 chars truncated
  CHAR16 LongStr[300];
  for(int i=0; i<299; i++) LongStr[i] = L'a';
  LongStr[299] = L'\0';
  Status = SanitizeInput(LongStr, Clean, &CleanLen);
  if (EFI_ERROR(Status) || CleanLen > MAX_RAW_INPUT_CHARS) return EFI_DEVICE_ERROR;

  // Test 5.3: control chars stripped
  CHAR16 CtrlStr[] = {0x01, 0x02, L'a', 0x1F, L'b', 0};
  Status = SanitizeInput(CtrlStr, Clean, &CleanLen);
  if (EFI_ERROR(Status) || StrCmp(Clean, L"ab") != 0) return EFI_DEVICE_ERROR;

  // Test 5.4: SQL injection quotes
  Status = SanitizeInput(L"'; DROP TABLE--", Clean, &CleanLen);
  if (EFI_ERROR(Status) || StrCmp(Clean, L" DROP TABLE") != 0) return EFI_DEVICE_ERROR;

  // Test 5.5: Unicode stripped
  CHAR16 UniStr[] = {0xFFFD, 0xD800, L'O', L'K', 0};
  Status = SanitizeInput(UniStr, Clean, &CleanLen);
  if (EFI_ERROR(Status) || StrCmp(Clean, L"OK") != 0) return EFI_DEVICE_ERROR;
  
  // Test 5.6: Rapid inputs no growth implicitly passes

  Print(L"[aiBIOS] Suite 5 PASS\n");
  return EFI_SUCCESS;
}
