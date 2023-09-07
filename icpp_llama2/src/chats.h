#pragma once

#include <string>
#include <unordered_map>

#include "ic_api.h"
#include "run.h"
#include "wasm_symbol.h"

void new_chat() WASM_SYMBOL_EXPORTED("canister_update new_chat");

// Orthogonally persisted user data
// For each registered user, data of their currently active chat
// (-) A user can have only 1 active chat at a time
// (-) Multiple users can be chatting at the same time
// (-) When a user starts a new chat, their previous ActiveChat will be cleared
class Chats {
public:
  std::unordered_map<std::string, Chat> umap;
};
extern Chats *p_chats;

void print_my_chat(std::string calling_function, std::string principal);
void new_p_chats();
void delete_p_chats();
void build_new_chat(std::string principal);
bool is_ready_and_authorized(IC_API ic_api);