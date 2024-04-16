#pragma once

#include "wasm_symbol.h"
#include <cstdint>
#include <vector>

// The uploaded bytes of the trained model (eg. models/stories15Mtok4096.bin)
class ModelBytes {
public:
  std::vector<uint8_t> vec;
};
extern ModelBytes *p_model_bytes;

// The uploaded bytes of the tokenizer (eg. tokenizers/tok4096.bin)
class TokenizerBytes {
public:
  std::vector<uint8_t> vec;
};
extern TokenizerBytes *p_tokenizer_bytes;

void reset_model() WASM_SYMBOL_EXPORTED("canister_update reset_model");
void reset_tokenizer() WASM_SYMBOL_EXPORTED("canister_update reset_tokenizer");

void upload_model_bytes_chunk()
    WASM_SYMBOL_EXPORTED("canister_update upload_model_bytes_chunk");
void upload_tokenizer_bytes_chunk()
    WASM_SYMBOL_EXPORTED("canister_update upload_tokenizer_bytes_chunk");