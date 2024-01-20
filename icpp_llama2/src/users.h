#pragma once

#include "wasm_symbol.h"
#include <string>

// user functions
void whoami() WASM_SYMBOL_EXPORTED("canister_query whoami");

// admin functions
void get_user_count() WASM_SYMBOL_EXPORTED("canister_query get_user_count");
void get_user_ids() WASM_SYMBOL_EXPORTED("canister_query get_user_ids");
void get_user_metadata()
    WASM_SYMBOL_EXPORTED("canister_query get_user_metadata");