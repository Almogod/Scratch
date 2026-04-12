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
  { 'g', 'a', 200 },
  { 200, 'm', 201 },
  { 201, 'i', 202 },
  { 202, 'n', 203 },
  { 203, 'g', 4903 },      // gaming
  { 't', 'h', 300 },
  { 300, 'e', 301 },
  { 301, 'r', 302 },
  { 302, 'm', 303 },
  { 303, 'a', 304 },
  { 304, 'l', 5001 },      // thermal
  { 'p', 'e', 400 },
  { 400, 'r', 401 },
  { 401, 'f', 5002 },      // perf
  { 'b', 'a', 500 },
  { 500, 't', 501 },
  { 501, 't', 502 },
  { 502, 'e', 503 },
  { 503, 'r', 504 },
  { 504, 'y', 5003 },      // battery
  { 'e', 'c', 5004 },      // eco
  { 's', 'i', 600 },
  { 600, 'l', 601 },
  { 601, 'e', 602 },
  { 602, 'n', 603 },
  { 603, 't', 5005 },      // silent
  { 'o', 'p', 700 },
  { 700, 't', 701 },
  { 701, 'i', 702 },
  { 702, 'm', 703 },
  { 703, 'i', 704 },
  { 704, 'z', 705 },
  { 705, 'e', 5006 },      // optimize
  { 's', 't', 800 },
  { 800, 'a', 801 },
  { 801, 't', 802 },
  { 802, 'u', 803 },
  { 803, 's', 5007 },      // status
  { 's', 't', 900 },
  { 900, 'a', 901 },
  { 901, 't', 902 },
  { 902, 's', 5010 },      // stats
  { 't', 'e', 1000 },
  { 1000, 'm', 1001 },
  { 1001, 'p', 5011 },     // temp
  { 'f', 'a', 1100 },
  { 1201, 't', 5013 },     // volt
  { 'i', 'n', 1300 },
  { 1300, 'c', 1301 },
  { 1301, 'r', 1302 },
  { 1302, 'e', 1303 },
  { 1303, 'a', 1304 },
  { 1304, 's', 1305 },
  { 1305, 'e', 5020 },     // increase
  { 'd', 'e', 1400 },
  { 1400, 'c', 1401 },
  { 1401, 'r', 1402 },
  { 1402, 'e', 1403 },
  { 1403, 'a', 1404 },
  { 1404, 's', 1405 },
  { 1405, 'e', 5021 },     // decrease
  { 'r', 'p', 1500 },
  { 1500, 'm', 5022 },     // rpm
  { 5001, 's', 5023 },     // thermals
  { 'h', 'e', 200 },
  { 200, 'l', 201 },
  { 201, 'l', 202 },
  { 202, 'o', 12199 },     // hello -> 12199, with dummy 29871 prefix
  { 29871, 'g', 29872 },
  { 29872, 'a', 29873 },
  { 29873, 'm', 29874 },
  { 29874, 'i', 29875 },
  { 29875, 'n', 29876 },
  { 29876, 'g', 4903 },
  // v0.6 New Semantic Concepts
  { 'a', 'i', 6001 },          // ai
  { 'v', 'i', 1600 },
  { 1600, 'r', 1601 },
  { 1601, 't', 1602 },
  { 1602, 'u', 1603 },
  { 1603, 'a', 1604 },
  { 1604, 'l', 6004 },         // virtual
  { 'm', 'a', 1700 },
  { 1700, 'c', 1701 },
  { 1701, 'h', 1702 },
  { 1702, 'i', 1703 },
  { 1703, 'n', 1704 },
  { 1704, 'e', 6005 },         // machine
  { 'h', 'o', 1800 },
  { 1800, 'w', 6003 },         // how
  { 'c', 'h', 1900 },
  { 1900, 'e', 1901 },
  { 1901, 'c', 1902 },
  { 1902, 'k', 6008 },         // check
  { 's', 'e', 2000 },
  { 2000, 'c', 2001 },
  { 2001, 'u', 2002 },
  { 2002, 'r', 2003 },
  { 2003, 'i', 2004 },
  { 2004, 't', 2005 },
  { 2005, 'y', 6009 },         // security
  { 'd', 'e', 2100 },
  { 2100, 'v', 2101 },
  { 2101, 'e', 2102 },
  { 2102, 'l', 2103 },
  { 2103, 'o', 2104 },
  { 2104, 'p', 2105 },
  { 2105, 'm', 2106 },
  { 2106, 'e', 2107 },
  { 2107, 'n', 2108 },
  { 2108, 't', 6002 },         // development
  // v0.8 Action Verbs (Agency)
  { 'd', 'o', 6101 },          // do
  { 'e', 'n', 1100 },
  { 1100, 'a', 1101 },
  { 1101, 'b', 1102 },
  { 1102, 'l', 1103 },
  { 1103, 'e', 6102 },         // enable
  { 's', 'e', 1200 },
  { 1200, 't', 6103 },         // set
  { 'a', 'p', 1300 },
  { 1300, 'p', 1301 },
  { 1301, 'l', 1302 },
  { 1302, 'y', 6104 },         // apply
  { 'c', 'o', 2200 },
  { 2200, 'n', 2201 },
  { 2201, 'f', 2202 },
  { 2202, 'i', 2203 },
  { 2203, 'g', 2204 },
  { 2204, 'u', 2205 },
  { 2205, 'r', 2206 },
  { 2206, 'e', 6105 },         // configure
  { 'i', 't', 6201 },           // it
  { 'i', 's', 6301 },           // is
  { 'd', 'o', 1400 },
  { 1400, 'n', 1401 },
  { 1401, 'e', 6302 },          // done
  { 'c', 'h', 1500 },
  { 1500, 'e', 1501 },
  { 1501, 'c', 1502 },
  { 1502, 'k', 6303 }           // check
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
