#ifndef AIBIOS_LLM_INFERENCE_H
#define AIBIOS_LLM_INFERENCE_H

#include <Uefi.h>

#define MAX_INPUT_TOKENS   128
#define MAX_OUTPUT_TOKENS  64
#define EMBEDDING_DIM      2048
#define NUM_LAYERS         22
#define VOCAB_SIZE         32000
#define KV_CACHE_SIZE      (MAX_INPUT_TOKENS + MAX_OUTPUT_TOKENS)

typedef struct {
  INT8    Data[EMBEDDING_DIM];   // INT8 activations
  UINT32  Scale;                 // Q8 scale factor (fixed-point)
} QUANTIZED_VECTOR;

typedef struct {
  INT32   InputTokens[MAX_INPUT_TOKENS];
  UINT32  InputLen;
  INT32   OutputTokens[MAX_OUTPUT_TOKENS];
  UINT32  OutputLen;
  BOOLEAN Truncated;             // Set if input exceeded MAX_INPUT_TOKENS
} INFERENCE_RESULT;

// O(n*L) time, O(D*L) space — no heap allocation
EFI_STATUS
LlmInferenceRun (
  IN  CONST CHAR16        *Prompt,
  IN  UINTN               PromptLen,
  OUT INFERENCE_RESULT    *Result
  );

EFI_STATUS
LlmInferenceInit (
  IN  CONST UINT8   *ModelWeights,
  IN  UINTN         ModelSize
  );

#endif // AIBIOS_LLM_INFERENCE_H
