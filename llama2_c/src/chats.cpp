// Maintain one chat per user (principal) in Orthogonal Persistence

#include <fstream>
#include <string>
#include <cstring>
#include <iostream>

#include "chats.h"
#include "canister.h"
#include "http.h"
#include "ic_api.h"

// Orthogonally Persisted data
Chats *p_chats{nullptr};
RunState *p_runstate{nullptr}; // Just one run state that we read back each time
ChatsOutputHistory *p_chats_output_history{nullptr};
MetadataUsers *p_metadata_users{nullptr};

// Create a p_chats & p_chats_output_history instance if not yet done
void new_p_chats() {
  if (p_chats == nullptr) {
    IC_API::debug_print(std::string(__func__) + ": Creating p_chats instance.");
    p_chats = new (std::nothrow) Chats();
    if (p_chats == nullptr) {
      // called from canister_init, so trap is correct!
      IC_API::trap("Allocation of p_chats failed");
    }
  }

  if (p_runstate == nullptr) {
    IC_API::debug_print(std::string(__func__) + ": Creating p_runstate instance.");
    p_runstate = new (std::nothrow) RunState();
    if (p_runstate == nullptr) {
      // called from canister_init, so trap is correct!
      IC_API::trap("Allocation of p_runstate failed");
    }
    init_run_state(p_runstate);
  }

  if (p_chats_output_history == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating p_chats_output_history instance.");
    p_chats_output_history = new (std::nothrow) ChatsOutputHistory();
    if (p_chats_output_history == nullptr) {
      // called from canister_init, so trap is correct!
      IC_API::trap("Allocation of p_chats_output_history failed");
    }
  }
}

// Delete the p_chats & p_chats_output_history instance
void delete_p_chats() {
  if (p_chats) {
    // for (auto &pair : p_chats->umap) {
    //   const std::string &principal = pair.first;
    //   Chat &chat = pair.second;
    //   free_run_state(&chat.state);
    // }
    delete p_chats;
    p_chats = nullptr;
  }

  if (p_runstate) {
    free_run_state(p_runstate);
    delete p_runstate;
    p_runstate = nullptr;
  }

  if (p_chats_output_history) {
    delete p_chats_output_history;
    p_chats_output_history = nullptr;
  }
}

// Create a p_metadata_users instance if not yet done
void new_p_metadata_users() {
  if (p_metadata_users == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating p_metadata_users instance.");
    p_metadata_users = new (std::nothrow) MetadataUsers();
    if (p_metadata_users == nullptr) {
      // called from canister_init, so trap is correct!
      IC_API::trap("Allocation of p_metadata_users failed");
    }
  }
}

// Delete the p_metadata_users instance
void delete_p_metadata_users() {
  if (p_metadata_users) {
    delete p_metadata_users;
    p_metadata_users = nullptr;
  }
}

void init_run_state(RunState *s) {
  s->x = nullptr;
  s->xb = nullptr;
  s->xb2 = nullptr;
  s->hb = nullptr;
  s->hb2 = nullptr;
  s->q = nullptr;
  s->k = nullptr;
  s->v = nullptr;
  s->att = nullptr;
  s->logits = nullptr;
  s->key_cache = nullptr;
  s->value_cache = nullptr;
}

// key = principal or ordinal-id
bool build_new_chat(std::string key, IC_API &ic_api) {
  // --------------------------------------------------------------------------
  // Create entries for this key in the persisted memory, if not yet done
  if (p_chats && p_chats->umap.find(key) == p_chats->umap.end()) {
    // Does not yet exist
    Chat chat;
    // init_run_state(&chat.state);  // moved to new_p_chats. not per user anymore.
    p_chats->umap[key] = chat;
  }

  if (p_chats_output_history && p_chats_output_history->umap.find(key) ==
                                    p_chats_output_history->umap.end()) {
    // Does not yet exist
    std::string output_history;
    p_chats_output_history->umap[key] = output_history;
  }

  if (p_metadata_users &&
      p_metadata_users->umap.find(key) == p_metadata_users->umap.end()) {
    // Does not yet exist
    MetadataUser metadata_user;
    p_metadata_users->umap[key] = metadata_user;
  }

  // --------------------------------------------------------------------------
  // Reset the Chat data
  Chat *chat = &p_chats->umap[key];
  // free_run_state(&chat->state);
  // if (!malloc_run_state(&chat->state, &transformer.config)) {
  //   std::string error_msg = "malloc_run_state failed";
  //   ic_api.to_wire(CandidTypeVariant{
  //       "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
  //   return false;
  // }

  // Reset the output data
  std::string *output_history = &p_chats_output_history->umap[key];
  output_history->clear();

  //initialize the next token predicted on pos 0 to the BOS token (1)
  chat->next = 1;
  chat->pos = 0;

  //icpp: initialize to add begin-of-sentence
  chat->bos = 1; // We no longer use this...
  chat->eos = 0;

  //icpp: initialize total_steps
  chat->total_steps = 0; // total story length, across multiple inference calls
  chat->inference_steps = 0; // per inference

  // --------------------------------------------------------------------------
  // The New Chat metadata
  MetadataUser *metadata_user = &p_metadata_users->umap[key];
  MetadataChat metadata_chat;
  metadata_chat.start_time = IC_API::time(); // time in ns
  metadata_user->metadata_chats.push_back(metadata_chat);

  return true;
}

// read runstate from file
// key = principal or ordinal-id
bool load_runstate(std::string key, IC_API &ic_api) {
  if (p_chats && p_chats->umap.find(key) == p_chats->umap.end()) {
    // Does not yet exist
    std::string error_msg = "load_runstate failed because key " + key + " does not exist";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }

  // read the run state from file into OP memory
  if (!read_run_state(key, *p_runstate, transformer.config)){
    // If nothing there, just continue with the empty run state
  }
  
  return true;
}

// write the run state to file
// key = principal or ordinal-id
bool save_runstate(std::string key, IC_API &ic_api) {
  if (p_chats && p_chats->umap.find(key) == p_chats->umap.end()) {
    // Does not yet exist
    std::string error_msg = "save_runstate failed because key " + key + " does not exist";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }

  // write the run state from OP memory to a file
  Chat *chat = &p_chats->umap[key];
  if (!write_run_state(key, *p_runstate, transformer.config)){
    std::string error_msg = "write_run_state failed for key " + key;
    std::cout << error_msg << std::endl;
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }

  // free the run state in OP memory
  // free_run_state(&chat->state);
  
  return true;
}

bool is_ready_and_authorized(IC_API &ic_api) {

  if (!ready_for_inference) {
    uint16_t status_code = Http::StatusCode::InternalServerError;
    ic_api.to_wire(CandidTypeVariant{"Err", CandidTypeNat16{status_code}});
    return false;
  }

  // Anonymous user is not allowed
  CandidTypePrincipal caller = ic_api.get_caller();
  if (caller.is_anonymous()) {
    ic_api.to_wire(CandidTypeVariant{
        "Err",
        CandidTypeText{
            "The Llama2 canister does not allow calling with anonymous principal."}});
    return false;
  }

  return true;
}

// Endpoint to be called by user when starting a new chat
void new_chat() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_mode_chat_principal()) {
    std::string error_msg =
        "Access Denied: canister_mode is not set to 'principal'.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }
  if (!is_ready_and_authorized(ic_api)) return;

  CandidTypePrincipal caller = ic_api.get_caller();
  std::string principal = caller.get_text();

  if (!build_new_chat(principal, ic_api)) return;

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

// Function to write RunState to a file
bool write_run_state(const std::string& key, const RunState& state, const Config& config) {
    std::string filename = key + ".runstate";
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cout << "Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }

    // Serialize RunState
    auto write_array = [&](const void* data, size_t count, size_t size) {
        out.write(static_cast<const char*>(data), count * size);
        return out.good();
    };

    if (!write_array(state.x, config.dim, sizeof(float)) ||
        !write_array(state.xb, config.dim, sizeof(float)) ||
        !write_array(state.xb2, config.dim, sizeof(float)) ||
        !write_array(state.hb, config.hidden_dim, sizeof(float)) ||
        !write_array(state.hb2, config.hidden_dim, sizeof(float)) ||
        !write_array(state.q, config.dim, sizeof(float)) ||
        !write_array(state.k, (config.dim * config.n_kv_heads) / config.n_heads, sizeof(float)) ||
        !write_array(state.v, (config.dim * config.n_kv_heads) / config.n_heads, sizeof(float)) ||
        !write_array(state.att, config.n_heads * config.seq_len, sizeof(float)) ||
        !write_array(state.logits, config.vocab_size, sizeof(float)) ||
        !write_array(state.key_cache, config.n_layers * config.seq_len * ((config.dim * config.n_kv_heads) / config.n_heads), sizeof(float)) ||
        !write_array(state.value_cache, config.n_layers * config.seq_len * ((config.dim * config.n_kv_heads) / config.n_heads), sizeof(float))) {
        std::cerr << "Error: Failed to write to file: " << filename << std::endl;
        return false;
    }

    return true;
}

// Function to read RunState from a file
bool read_run_state(const std::string& key, RunState& state, const Config& config) {
    std::string filename = key + ".runstate";
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cout << "INFO: Could not open file for reading: " << filename << std::endl;
        return false;
    }

    // Deserialize RunState
    auto read_array = [&](void* data, size_t count, size_t size) {
        in.read(static_cast<char*>(data), count * size);
        return in.good();
    };

    if (!read_array(state.x, config.dim, sizeof(float)) ||
        !read_array(state.xb, config.dim, sizeof(float)) ||
        !read_array(state.xb2, config.dim, sizeof(float)) ||
        !read_array(state.hb, config.hidden_dim, sizeof(float)) ||
        !read_array(state.hb2, config.hidden_dim, sizeof(float)) ||
        !read_array(state.q, config.dim, sizeof(float)) ||
        !read_array(state.k, (config.dim * config.n_kv_heads) / config.n_heads, sizeof(float)) ||
        !read_array(state.v, (config.dim * config.n_kv_heads) / config.n_heads, sizeof(float)) ||
        !read_array(state.att, config.n_heads * config.seq_len, sizeof(float)) ||
        !read_array(state.logits, config.vocab_size, sizeof(float)) ||
        !read_array(state.key_cache, config.n_layers * config.seq_len * ((config.dim * config.n_kv_heads) / config.n_heads), sizeof(float)) ||
        !read_array(state.value_cache, config.n_layers * config.seq_len * ((config.dim * config.n_kv_heads) / config.n_heads), sizeof(float))) {
        std::cerr << "Error: Failed to read from file: " << filename << std::endl;
        return false;
    }

    return true;
}
