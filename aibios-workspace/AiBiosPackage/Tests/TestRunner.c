#include "TestMain.h"
#include "../LlmInference/LlmInference.h"
#include "../LlmInference/Tokenizer.h"
#include "../HardwareMonitor/HardwareMonitor.h"
#include "../SettingsTuner/SettingsTuner.h"
#include "../NlInterface/NlInterface.h"

EFI_STATUS RunAllTests (VOID) {
  UINTN Passes = 0, Fails = 0;
  EFI_STATUS Status;

  Print(L"\n[aiBIOS] Starting Test Loop...\n");
  
  Status = RunSuite1();
  if (!EFI_ERROR(Status)) Passes++; else Fails++;
  
  Status = RunSuite2();
  if (!EFI_ERROR(Status)) Passes++; else Fails++;
  
  Status = RunSuite3();
  if (!EFI_ERROR(Status)) Passes++; else Fails++;
  
  Status = RunSuite4();
  if (!EFI_ERROR(Status)) Passes++; else Fails++;
  
  Status = RunSuite5();
  if (!EFI_ERROR(Status)) Passes++; else Fails++;
  
  Status = RunSuite6();
  if (!EFI_ERROR(Status)) Passes++; else Fails++;
  
  Print(L"Tests Complete: %d Suites Passed, %d Failed.\n", Passes, Fails);
  
  if (Fails == 0) {
    Print(L"ALL TESTS PASSED.\n");
  } else {
    Print(L"SOME TESTS FAILED.\n");
  }
  
  return (Fails == 0) ? EFI_SUCCESS : EFI_ABORTED;
}
