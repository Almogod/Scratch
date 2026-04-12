#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include "NlInterface.h"

typedef struct {
  CHAR16 *Keyword;
  CHAR16 *Explanation;
  CHAR16 *SettingName;
} KNOWLEDGE_ENTRY;

STATIC CONST KNOWLEDGE_ENTRY gKnowledgeBase[] = {
  { L"virtual machine", L"To run virtual machines, you must enable Virtualization Technology (VT-d for Intel, AMD-V for AMD). This allows the CPU to offload VM tasks to hardware.", L"VT-d / AMD-V" },
  { L"secure boot", L"Secure Boot ensures that the system only boots software that is trusted by the PC manufacturer. It prevents rootkits from loading during boot.", L"Secure Boot" },
  { L"fast boot", L"Fast Boot reduces boot time by skipping certain hardware initialization steps. Useful for SSD-based systems.", L"Fast Boot" },
  { L"tpm", L"The Trusted Platform Module (TPM) provides hardware-based security functions, such as key storage and platform integrity measurement.", L"TPM 2.0" }
};

EFI_STATUS
LookupKnowledge (
  IN  CONST CHAR16  *Query,
  OUT CONST CHAR16  **Explanation,
  OUT CONST CHAR16  **SettingName
  )
{
  UINTN i;

  if (Query == NULL || Explanation == NULL || SettingName == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (i = 0; i < sizeof(gKnowledgeBase)/sizeof(gKnowledgeBase[0]); i++) {
    if (StrStr(Query, gKnowledgeBase[i].Keyword) != NULL) {
      *Explanation = gKnowledgeBase[i].Explanation;
      *SettingName = gKnowledgeBase[i].SettingName;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
