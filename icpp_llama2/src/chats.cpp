// Maintain one chat per user (principal) in Orthogonal Persistence

#include "chats.h"
#include "canister.h"
#include "http.h"
#include "ic_api.h"

// Orthogonally Persisted data
Chats *p_chats{nullptr};
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
    for (auto &pair : p_chats->umap) {
      const std::string &principal = pair.first;
      Chat &chat = pair.second;
      free_run_state(&chat.state);
    }
    delete p_chats;
    p_chats = nullptr;
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
    init_run_state(&chat.state);
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
  free_run_state(&chat->state);
  if (!malloc_run_state(&chat->state, &transformer.config)) {
    std::string error_msg = "malloc_run_state failed";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }

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