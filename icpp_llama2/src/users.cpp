// Endpoints for user metadata get & set

#include "users.h"

#include <string>

#include "canister.h"
#include "chats.h"
#include "http.h"
#include "ic_api.h"

#include "run.h"

void whoami() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  CandidTypePrincipal caller = ic_api.get_caller();
  ic_api.to_wire(CandidTypeText{caller.get_text()});
}

void get_user_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  uint64_t user_count = 0;
  if (p_metadata_users) {
    user_count = p_metadata_users->umap.size();
  }

  ic_api.to_wire(CandidTypeNat64{user_count});
}

void get_user_ids() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  std::vector<std::string> user_ids;

  if (p_metadata_users) {
    for (const auto &pair : p_metadata_users->umap) {
      user_ids.push_back(pair.first);
    }
  }

  ic_api.to_wire(CandidTypeVecText{user_ids});
}

void get_user_metadata() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  std::string in_principal;
  ic_api.from_wire(CandidTypeText{&in_principal});

  std::vector<uint64_t> chats_start_time;
  std::vector<uint64_t> chats_total_steps;

  auto it = p_metadata_users->umap.find(in_principal);
  if (it != p_metadata_users->umap.end()) {
    // Found the user metadata for the given in_principal
    for (const MetadataChat &chat : it->second.metadata_chats) {
      chats_start_time.push_back(chat.start_time);
      chats_total_steps.push_back(chat.total_steps);
    }
  }

  CandidArgs args_out;
  args_out.append(CandidTypeVecNat64{chats_start_time});
  args_out.append(CandidTypeVecNat64{chats_total_steps});
  ic_api.to_wire(args_out);
}