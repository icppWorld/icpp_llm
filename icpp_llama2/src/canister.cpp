// canister_init, and health & ready endpoints
#include "canister.h"

#include <string>
#include <unordered_map>
#include <variant>

#include "chats.h"
#include "http.h"
#include "ic_api.h"

std::string *p_owner_principal{nullptr};
bool ready_for_inference{false};

void set_owner(IC_API &ic_api) {
  CandidTypePrincipal caller = ic_api.get_caller();

  if (p_owner_principal == nullptr) {
    p_owner_principal = new (std::nothrow) std::string();
    if (p_owner_principal == nullptr) {
      IC_API::trap("Allocation of p_owner_principal failed");
    }
  }
  *p_owner_principal = caller.get_text();

  // IC_API::debug_print(std::string(__func__) +
  //                     ": owner_principal = " + *p_owner_principal);
}

bool is_owner(IC_API &ic_api, bool err_to_wire) {
  CandidTypePrincipal caller = ic_api.get_caller();
  if (caller.get_text() == *p_owner_principal) return true;
  else {
    IC_API::debug_print(std::string(__func__) +
                        ": ERROR - caller is not the owner.");

    if (err_to_wire) {
      uint16_t status_code = Http::StatusCode::Unauthorized;
      ic_api.to_wire(CandidTypeVariant{"err", CandidTypeNat16{status_code}});
    }
    return false;
  }
}

void print_canister_metadata() {
  std::string msg = std::string(__func__) + ":";
  msg += "\n*p_owner_principal     = " + *p_owner_principal;
  IC_API::debug_print(msg);
}

// --------------------------------------------------------------------------------------------------
// See: https://internetcomputer.org/docs/current/references/ic-interface-spec#system-api-init
// - This method will be called automatically during INITIAL deployment
void canister_init() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);
  set_owner(ic_api);

  // Create a p_chats instance
  new_p_chats();

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

// readiness endpoint (ready for inference)
void ready() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  IC_API::debug_print("llama2 readiness for inference = " +
                      std::to_string(ready_for_inference));
  ic_api.to_wire(CandidTypeBool{ready_for_inference});
}