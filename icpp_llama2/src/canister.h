#pragma once

#include "ic_api.h"
#include "wasm_symbol.h"
#include <string>

extern std::string *p_owner_principal;
extern bool ready_for_inference;

bool is_owner(IC_API &ic_api, bool err_to_wire = true);

void canister_init() WASM_SYMBOL_EXPORTED("canister_init");
void health() WASM_SYMBOL_EXPORTED("canister_query health");
void ready() WASM_SYMBOL_EXPORTED("canister_query ready");