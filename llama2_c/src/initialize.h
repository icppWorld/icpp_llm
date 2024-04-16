#pragma once

#include "wasm_symbol.h"
#include <string>

#include "run.h"

void initialize() WASM_SYMBOL_EXPORTED("canister_update initialize");
void get_model_config() WASM_SYMBOL_EXPORTED("canister_query get_model_config");