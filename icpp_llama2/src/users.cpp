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

void get_users() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api, false)) {
    std::string error_msg = "Access Denied";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  // Get the number of users
  uint64_t user_count = 0;
  if (p_metadata_users) {
    user_count = p_metadata_users->umap.size();
  }

  // Get the user ids
  std::vector<std::string> user_ids;
  if (p_metadata_users) {
    for (const auto &pair : p_metadata_users->umap) {
      user_ids.push_back(pair.first);
    }
  }

  CandidTypeRecord users_record;
  users_record.append("user_count", CandidTypeNat64{user_count});
  users_record.append("user_ids", CandidTypeVecText{user_ids});
  ic_api.to_wire(CandidTypeVariant{"Ok", users_record});
}

void get_user_metadata() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api, false)) {
    std::string error_msg = "Access Denied";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

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

  CandidTypeRecord user_metadata_record;
  user_metadata_record.append("chats_start_time",
                              CandidTypeVecNat64{chats_start_time});
  user_metadata_record.append("chats_total_steps",
                              CandidTypeVecNat64{chats_total_steps});
  ic_api.to_wire(CandidTypeVariant{"Ok", user_metadata_record});
}