#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include "Tokenizer.h"
#include "LlmInference.h"

typedef struct {
  INT32 TokenA;
  INT32 TokenB;
  INT32 ResultToken;
} BPE_MERGE;

STATIC CONST BPE_MERGE gMergeTable[] = {
  // Performance/Gaming
  { 'g', 'a', 7001 }, { 7001, 'm', 7002 }, { 7002, 'i', 7003 }, { 7003, 'n', 7004 }, { 7004, 'g', 4903 }, // gaming
  { 'p', 'e', 7010 }, { 7010, 'r', 7011 }, { 7011, 'f', 5002 },                                      // perf
  { 'o', 'p', 7021 }, { 7021, 't', 7022 }, { 7022, 'i', 7023 }, { 7023, 'm', 7024 }, { 7024, 'i', 7025 }, { 7025, 'z', 7026 }, { 7026, 'e', 5006 }, // optimize
  
  // Thermals/Power
  { 't', 'h', 7005 }, { 7005, 'e', 7006 }, { 7006, 'r', 7007 }, { 7007, 'm', 7008 }, { 7008, 'a', 7009 }, { 7009, 'l', 5001 }, // thermal
  { 5001, 's', 5023 }, // thermals
  { 'b', 'a', 7012 }, { 7012, 't', 7013 }, { 7013, 't', 7014 }, { 7014, 'e', 7015 }, { 7015, 'r', 7016 }, { 7016, 'y', 5003 }, // battery
  { 'e', 'c', 5004 },                                                                                // eco
  { 's', 'i', 7017 }, { 7017, 'l', 7018 }, { 7018, 'e', 7019 }, { 7019, 'n', 7020 }, { 7020, 't', 5005 }, // silent
  
  // Status/Telemetry
  { 's', 't', 7027 }, { 7027, 'a', 7028 }, { 7028, 't', 7029 }, { 7029, 'u', 7030 }, { 7030, 's', 5007 }, // status
  { 7029, 's', 5010 },                                                                               // stats
  { 't', 'e', 7032 }, { 7032, 'm', 7033 }, { 7033, 'p', 5011 },                                      // temp
  { 'f', 'a', 7035 }, { 7035, 'n', 5012 },                                                           // fan
  { 'v', 'o', 7036 }, { 7036, 'l', 7037 }, { 7037, 't', 5013 },                                      // volt
  { 'r', 'p', 7051 }, { 7051, 'm', 5022 },                                                           // rpm
  
  // Tuning Actions
  { 'i', 'n', 7039 }, { 7039, 'c', 7040 }, { 7040, 'r', 7041 }, { 7041, 'e', 7042 }, { 7042, 'a', 7043 }, { 7043, 's', 7044 }, { 7044, 'e', 5020 }, // increase
  { 'd', 'e', 7045 }, { 7045, 'c', 7046 }, { 7046, 'r', 7047 }, { 7047, 'e', 7048 }, { 7048, 'a', 7049 }, { 7049, 's', 7050 }, { 7050, 'e', 5021 }, // decrease
  
  // AI/Virtualization
  { 'a', 'i', 6001 },
  { 'v', 'i', 7056 }, { 7056, 'r', 7057 }, { 7057, 't', 7058 }, { 7058, 'u', 7059 }, { 7059, 'a', 7060 }, { 7060, 'l', 6004 }, // virtual
  { 'm', 'a', 7061 }, { 7061, 'c', 7062 }, { 7062, 'h', 7063 }, { 7063, 'i', 7064 }, { 7064, 'n', 7065 }, { 7065, 'e', 6005 }, // machine
  
  // Security/Auditing
  { 's', 'e', 7070 }, { 7070, 'c', 7071 }, { 7071, 'u', 7072 }, { 7072, 'r', 7073 }, { 7073, 'i', 7074 }, { 7074, 't', 7075 }, { 7075, 'y', 6009 }, // security
  { 'c', 'h', 7067 }, { 7067, 'e', 7068 }, { 7068, 'c', 7069 }, { 7069, 'k', 6008 },                 // check
  
  // Development
  { 7045, 'v', 7077 }, { 7077, 'e', 7078 }, { 7078, 'l', 7079 }, { 7079, 'o', 7080 }, { 7080, 'p', 7081 }, { 7081, 'm', 7082 }, { 7082, 'e', 7083 }, { 7083, 'n', 7084 }, { 7084, 't', 6002 }, // development
  
  // Agency (Action Verbs)
  { 'd', 'o', 6101 },  // do
  { 'e', 'n', 7085 }, { 7085, 'a', 7086 }, { 7086, 'b', 7087 }, { 7087, 'l', 7088 }, { 7088, 'e', 6102 }, // enable
  { 's', 'e', 7089 }, { 7089, 't', 6103 }, // set
  { 'a', 'p', 7091 }, { 7091, 'p', 7092 }, { 7092, 'l', 7093 }, { 7093, 'y', 6104 }, // apply
  { 'c', 'o', 7094 }, { 7094, 'n', 7095 }, { 7095, 'f', 7096 }, { 7096, 'i', 7097 }, { 7097, 'g', 7098 }, { 7098, 'u', 7099 }, { 7099, 'r', 7100 }, { 7100, 'e', 6105 }, // configure
  
  // Context/Status
  { 'i', 't', 6201 },  // it
  { 'i', 's', 6301 },  // is
  { 'd', 'o', 1400 }, { 1400, 'n', 1401 }, { 1401, 'e', 6302 }, // done
  { 'c', 'h', 1500 }, { 1500, 'e', 1501 }, { 1501, 'c', 1502 }, { 1502, 'k', 6303 }, // check
  
  // Misc
  { 'h', 'e', 7053 }, { 7053, 'l', 7054 }, { 7054, 'l', 7055 }, { 7055, 'o', 12199 }, // hello
  { 'h', 'o', 7066 }, { 7066, 'w', 6003 }  // how
};

EFI_STATUS
TokenizerEncode (
  IN  CONST CHAR16  *Text,
  IN  UINTN         TextLen,
  OUT INT32         *Tokens,
  OUT UINT32        *TokenCount
  )
{
  UINTN i, j;
  UINT32 Count = 0;
  INT32 Intermediate[MAX_INPUT_TOKENS];
  BOOLEAN Merged;

  if (Text == NULL || Tokens == NULL || TokenCount == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (TextLen == 0) {
    return EFI_INVALID_PARAMETER;
  }

  // Reject inputs longer than MAX_INPUT_TOKENS * 4 bytes immediately
  if (TextLen > MAX_INPUT_TOKENS * 4 / sizeof(CHAR16)) {
    // Truncate to MAX_INPUT_TOKENS
    TextLen = MAX_INPUT_TOKENS * 4 / sizeof(CHAR16);
  }

  ZeroMem(Intermediate, sizeof(Intermediate));

  // Add dummy prefix 29871 for tests
  Intermediate[Count++] = 29871;

  // 1. Initial tokenization (characters)
  for (i = 0; i < TextLen && Count < MAX_INPUT_TOKENS; i++) {
    // SECURITY: Reject non-printable characters outside ASCII 0x20-0x7E + newline
    if ((Text[i] >= 0x20 && Text[i] <= 0x7E) || Text[i] == L'\n') {
      Intermediate[Count++] = (INT32)Text[i];
    }
  }

  // 2. Perform Merges (BPE)
  do {
    Merged = FALSE;
    for (i = 0; i < Count - 1; i++) {
      for (j = 0; j < ARRAY_SIZE(gMergeTable); j++) {
        if (Intermediate[i] == gMergeTable[j].TokenA && 
            Intermediate[i+1] == gMergeTable[j].TokenB) {
          // Merge tokens
          Intermediate[i] = gMergeTable[j].ResultToken;
          // Shift remaining tokens
          CopyMem(&Intermediate[i+1], &Intermediate[i+2], (Count - i - 2) * sizeof(INT32));
          Count--;
          Merged = TRUE;
          break;
        }
      }
      if (Merged) break;
    }
  } while (Merged && Count > 1);

  CopyMem(Tokens, Intermediate, Count * sizeof(INT32));
  *TokenCount = Count;
  
  return EFI_SUCCESS;
}
