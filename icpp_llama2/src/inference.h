#pragma once

#include "wasm_symbol.h"
#include <string>
#include "ic_api.h"
#include "chats.h"
#include "prompt.h"

void inference() WASM_SYMBOL_EXPORTED("canister_update inference");
void inference_mo() WASM_SYMBOL_EXPORTED("canister_update inference_mo");

void inference_(bool from_motoko);
std::string do_inference(IC_API &ic_api, Prompt wire_prompt, Chat *chat,
                         std::string *output_history,
                         MetadataUser *metadata_user, bool *error);
