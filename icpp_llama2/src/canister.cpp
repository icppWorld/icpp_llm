// canister_init, and health & ready endpoints
#include "canister.h"

#include <string>
#include <unordered_map>
#include <variant>

#include "chats.h"
#include "http.h"
#include "ic_api.h"
#include "nft_collection.h"

std::string *p_canister_owner_principal{nullptr};
std::string *p_canister_mode{nullptr};
bool ready_for_inference{false};

void set_canister_owner(IC_API &ic_api) {
  CandidTypePrincipal caller = ic_api.get_caller();

  if (p_canister_owner_principal == nullptr) {
    p_canister_owner_principal = new (std::nothrow) std::string();
    if (p_canister_owner_principal == nullptr) {
      IC_API::trap("Allocation of p_canister_owner_principal failed");
    }
  }
  *p_canister_owner_principal = caller.get_text();

  // IC_API::debug_print(std::string(__func__) +
  //                     ": canister_owner_principal = " + *p_canister_owner_principal);
}

// Checks if caller is owner of the canister
bool is_canister_owner(IC_API &ic_api, bool err_to_wire) {
  CandidTypePrincipal caller = ic_api.get_caller();
  if (caller.get_text() == *p_canister_owner_principal) return true;
  else {
    IC_API::debug_print(std::string(__func__) +
                        ": ERROR - caller is not the owner.");

    if (err_to_wire) {
      uint16_t status_code = Http::StatusCode::Unauthorized;
      ic_api.to_wire(CandidTypeVariant{"Err", CandidTypeNat16{status_code}});
    }
    return false;
  }
}

bool is_canister_mode_valid(std::string canister_mode) {
  return canister_mode == "chat-principal" || canister_mode == "nft-ordinal";
}
bool is_canister_mode_set() {
  return is_canister_mode_chat_principal() || is_canister_mode_nft_ordinal();
}
bool is_canister_mode_chat_principal() {
  return p_canister_mode && *p_canister_mode == "chat-principal";
}
bool is_canister_mode_nft_ordinal() {
  return p_canister_mode && *p_canister_mode == "nft-ordinal";
}

void set_canister_mode() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Only the canister owner is allowed to set the canister mode
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  // The canister mode is passed by argument
  std::string canister_mode;
  ic_api.from_wire(CandidTypeText(&canister_mode));

  if (!is_canister_mode_valid(canister_mode)) {
    IC_API::trap("ERROR: unknown canister_mode = " + canister_mode);
  }

  if (p_canister_mode == nullptr) {
    p_canister_mode = new (std::nothrow) std::string();
    if (p_canister_mode == nullptr) {
      IC_API::trap("Allocation of p_canister_mode failed");
    }
  }
  *p_canister_mode = canister_mode;

  IC_API::debug_print(std::string(__func__) +
                      ": p_canister_mode = " + *p_canister_mode);

  ic_api.to_wire();
}

void print_canister_metadata() {
  std::string msg = std::string(__func__) + ":";
  msg += "\n*p_canister_owner_principal     = " + *p_canister_owner_principal;
  msg += "\n*p_canister_mode                = " + *p_canister_mode;
  IC_API::debug_print(msg);
}

// --------------------------------------------------------------------------------------------------
// See: https://internetcomputer.org/docs/current/references/ic-interface-spec#system-api-init
// - This method will be called automatically during INITIAL deployment
void canister_init() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);
  set_canister_owner(ic_api);

  // Create a p_chats & p_chats_output_history instance
  new_p_chats();

  // Create a p_nft_collection instance
  new_p_nft_whitelist();
  new_p_nft_collection();

  // Create a p_metadata_users instance
  new_p_metadata_users();
}

// --------------------------------------------------------------------------------------------------
// health endpoint (liveness)
void health() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  IC_API::debug_print("llama2 is healthy!");
  ic_api.to_wire(CandidTypeBool{true});
}

// readiness endpoint (ready for inference & NFT Collection initialized
void ready() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  bool ready = ready_for_inference && p_nft_collection &&
               p_nft_collection->initialized && is_canister_mode_set();
  ic_api.to_wire(CandidTypeBool{ready});
}