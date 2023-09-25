// Maintain one chat per user (principal) in Orthogonal Persistence

#include "chats.h"
#include "canister.h"
#include "http.h"
#include "ic_api.h"

// umap for Orthogonally Persisted chat data, per principal
Chats *p_chats{nullptr};
MetadataUsers *p_metadata_users{nullptr};

// Create a p_chats instance if not yet done
void new_p_chats() {
  if (p_chats == nullptr) {
    IC_API::debug_print(std::string(__func__) + ": Creating p_chats instance.");
    p_chats = new (std::nothrow) Chats();
    if (p_chats == nullptr) {
      IC_API::trap("Allocation of p_chats failed");
    }
  }
}

// Delete the p_chats instance
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
}

// Create a p_metadata_users instance if not yet done
void new_p_metadata_users() {
  if (p_metadata_users == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating p_metadata_users instance.");
    p_metadata_users = new (std::nothrow) MetadataUsers();
    if (p_metadata_users == nullptr) {
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

void build_new_chat(std::string principal) {
  // --------------------------------------------------------------------------
  // Create entries for this principal in the persisted memory, if not yet done
  if (p_chats && p_chats->umap.find(principal) == p_chats->umap.end()) {
    // Does not yet exist
    Chat chat;
    init_run_state(&chat.state);
    p_chats->umap[principal] = chat;
  }

  if (p_metadata_users &&
      p_metadata_users->umap.find(principal) == p_metadata_users->umap.end()) {
    // Does not yet exist
    MetadataUser metadata_user;
    p_metadata_users->umap[principal] = metadata_user;
  }

  // --------------------------------------------------------------------------
  // The New Chat data
  Chat *chat = &p_chats->umap[principal];
  free_run_state(&chat->state);
  malloc_run_state(&chat->state, &transformer.config);

  //initialize the next token predicted on pos 0 to the BOS token (1)
  chat->next = 1;
  chat->pos = 0;

  //icpp: initialize to add begin-of-sentence
  chat->bos = 1; // We no longer use this...
  chat->eos = 0;

  //icpp: initialize total_steps
  chat->total_steps = 0;

  // --------------------------------------------------------------------------
  // The New Chat metadata
  MetadataUser *metadata_user = &p_metadata_users->umap[principal];
  MetadataChat metadata_chat;
  metadata_chat.start_time = IC_API::time(); // time in ns
  metadata_user->metadata_chats.push_back(metadata_chat);
}

bool is_ready_and_authorized(IC_API ic_api) {

  if (!ready_for_inference) {
    ic_api.to_wire(CandidTypeVariant{
        "err", CandidTypeText{
                   "The Llama2 canister is not (yet) ready for inference."}});
    return false;
  }

  // Anonymous user is not allowed
  CandidTypePrincipal caller = ic_api.get_caller();
  if (caller.is_anonymous()) {
    ic_api.to_wire(CandidTypeVariant{
        "err",
        CandidTypeText{
            "The Llama2 canister does not allow calling with anonymous principal."}});
    return false;
  }

  return true;
}

// Endpoint to be called by user when starting a new chat
void new_chat() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_ready_and_authorized(ic_api)) return;

  CandidTypePrincipal caller = ic_api.get_caller();
  std::string principal = caller.get_text();

  build_new_chat(principal);

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}