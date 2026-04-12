#ifndef AIBIOS_SECURITY_H
#define AIBIOS_SECURITY_H

#include <Uefi.h>

EFI_STATUS
PerformSecurityAudit (
  OUT BOOLEAN *AnomalyDetected
  );

#endif // AIBIOS_SECURITY_H
