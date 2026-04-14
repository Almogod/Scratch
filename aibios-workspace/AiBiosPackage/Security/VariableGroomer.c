#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#define MAX_VAR_NAME_SIZE 1024

EFI_STATUS
ScanAndCleanVariables (
  OUT UINT32 *CleanedCount
  )
{
  EFI_STATUS  Status;
  UINTN       NameSize;
  CHAR16      *Name;
  EFI_GUID    Guid;
  UINT32      Count = 0;

  if (CleanedCount == NULL) return EFI_INVALID_PARAMETER;

  Name = AllocateZeroPool (MAX_VAR_NAME_SIZE);
  if (Name == NULL) return EFI_OUT_OF_RESOURCES;

  NameSize = MAX_VAR_NAME_SIZE;
  Name[0] = L'\0';

  DEBUG ((DEBUG_INFO, "[aiBIOS Security] Initiating full firmware variable scan...\n"));

  while (TRUE) {
    NameSize = MAX_VAR_NAME_SIZE;
    Status = gRT->GetNextVariableName (&NameSize, Name, &Guid);
    
    if (Status == EFI_NOT_FOUND) {
      break; 
    }

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "[aiBIOS Security] Error during variable iteration: %r\n", Status));
      break;
    }

    // Heuristic for "Orphaned" or "Suspicious" variables:
    // 1. Variables in 'Setup' GUID that are unusually sized (>4KB)
    // 2. Mock 'OldLogs' or 'TestVar' names
    
    // Check Size
    UINTN  DataSize = 0;
    Status = gRT->GetVariable(Name, &Guid, NULL, &DataSize, NULL);
    
    BOOLEAN ShouldClean = FALSE;
    if (StrStr (Name, L"OldLog") != NULL || StrStr (Name, L"TestVar") != NULL) {
      ShouldClean = TRUE;
    } else if (DataSize > 4096 && StrLen(Name) < 32) {
      // Unusually large data for a short variable name — possible overflow/tampering
      DEBUG ((DEBUG_WARN, "[aiBIOS Security] Suspicious variable size (%d bytes) for %s!\n", DataSize, Name));
      ShouldClean = TRUE;
    }

    if (ShouldClean) {
      DEBUG ((DEBUG_INFO, "[aiBIOS Security] Cleaning suspicious variable: %s...\n", Name));
      
      // Autonomous Cleanup
      Status = gRT->SetVariable (Name, &Guid, 0, 0, NULL);
      if (!EFI_ERROR (Status)) {
        Count++;
      } else {
        DEBUG ((DEBUG_WARN, "[aiBIOS Security] Failed to delete %s: %r\n", Name, Status));
      }
    }
  }

  FreePool (Name);
  *CleanedCount = Count;
  return EFI_SUCCESS;
}
