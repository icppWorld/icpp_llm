#pragma once

#include "wasm_symbol.h"
#include <string>

void inference() WASM_SYMBOL_EXPORTED("canister_query inference");
void inference_update()
    WASM_SYMBOL_EXPORTED("canister_update inference_update");