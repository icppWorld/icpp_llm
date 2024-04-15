#pragma once

#include "ic_api.h"
#include "wasm_symbol.h"
#include <string>

extern std::string *p_canister_owner_principal;
extern std::string *p_canister_mode;
extern bool ready_for_inference;

bool is_canister_owner(IC_API &ic_api, bool err_to_wire = true);
bool is_canister_mode_valid(std::string canister_mode);
bool is_canister_mode_set();
bool is_canister_mode_chat_principal();
bool is_canister_mode_nft_ordinal();

void canister_init() WASM_SYMBOL_EXPORTED("canister_init");
void set_canister_mode()
    WASM_SYMBOL_EXPORTED("canister_update set_canister_mode");
void health() WASM_SYMBOL_EXPORTED("canister_query health");
void ready() WASM_SYMBOL_EXPORTED("canister_query ready");