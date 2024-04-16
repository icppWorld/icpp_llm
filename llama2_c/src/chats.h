#pragma once

#include <string>
#include <unordered_map>

#include "ic_api.h"
#include "run.h"
#include "wasm_symbol.h"

void new_chat() WASM_SYMBOL_EXPORTED("canister_update new_chat");

// Orthogonally persisted user data
// ---
// Chats: For each key, data of their currently active chat
// (-) A key can be a principal or an ordinal-id
// (-) A user can have only 1 active chat at a time
// (-) Multiple users can be chatting at the same time
// (-) When a user starts a new chat, their previous ActiveChat will be cleared
class Chats {
public:
  // Chat is a struct defined in run.h, because it needs to be available to run.c
  //                 key
  std::unordered_map<std::string, Chat> umap;
};
extern Chats *p_chats;

// Save current chat history (the full human readable story)
class ChatsOutputHistory {
public:
  //                 key
  std::unordered_map<std::string, std::string> umap;
};
extern ChatsOutputHistory *p_chats_output_history;

// ---
// Some minimal usage data: umap[key, MetaDataChat]

// Metadata for the full chat, which can be one or more prompts
struct MetadataChat {
  uint64_t start_time{0};  // time in ns
  uint64_t total_steps{0}; // total number of steps (=tokens)
};

// Metadata for the User, containing a vector of all chats' metadata
struct MetadataUser {
  std::vector<MetadataChat> metadata_chats;
};

class MetadataUsers {
public:
  std::unordered_map<std::string, MetadataUser> umap;
};
extern MetadataUsers *p_metadata_users;
// ---

void new_p_chats();
void delete_p_chats();
void new_p_metadata_users();
void delete_p_metadata_users();
bool build_new_chat(std::string key, IC_API &ic_api);
bool is_ready_and_authorized(IC_API &ic_api);