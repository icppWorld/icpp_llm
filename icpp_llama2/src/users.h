#pragma once

#include "wasm_symbol.h"
#include <string>

// user functions
void whoami() WASM_SYMBOL_EXPORTED("canister_query whoami");

// admin functions
void get_users() WASM_SYMBOL_EXPORTED("canister_query get_users");
void get_user_metadata()
    WASM_SYMBOL_EXPORTED("canister_query get_user_metadata");