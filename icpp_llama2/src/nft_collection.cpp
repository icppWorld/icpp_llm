// Maintain one chat per user (principal) in Orthogonal Persistence

#include "nft_collection.h"
#include "canister.h"
#include "chats.h"
#include "prompt.h"
#include "inference.h"
#include "http.h"
#include "ic_api.h"

// Orthogonally Persisted data
NFTWhitelist *p_nft_whitelist{nullptr};
NFTCollection *p_nft_collection{nullptr};

// Create a p_nft_whitelist instance if not yet done
void new_p_nft_whitelist() {
  if (p_nft_whitelist == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Allocating memory for p_nft_whitelist.");
    p_nft_whitelist = new (std::nothrow) NFTWhitelist();
    if (p_nft_whitelist == nullptr) {
      IC_API::trap("Allocation of p_nft_whitelist failed");
    }
  }
}

// Delete the p_nft_whitelist instance
void delete_p_nft_whitelist() {
  if (p_nft_whitelist) {
    delete p_nft_whitelist;
    p_nft_whitelist = nullptr;
  }
}

// Checks if caller is whitelisted to update nfts
bool nft_is_whitelisted(IC_API &ic_api, bool err_to_wire) {
  CandidTypePrincipal caller = ic_api.get_caller();

  // canister owner is always whitelisted
  if (is_canister_owner(ic_api, false)) return true;

  // check if the caller's principal was added to the whitelist
  for (auto &item : p_nft_whitelist->whitelist) {
    if (item.principal.get_text() == caller.get_text()) {
      return true;
    }
  }

  IC_API::debug_print(std::string(__func__) + ": ERROR - caller " +
                      caller.get_text() + " is not whitelisted.");

  if (err_to_wire) {
    uint16_t status_code = Http::StatusCode::Unauthorized;
    ic_api.to_wire(CandidTypeVariant{"err", CandidTypeNat16{status_code}});
  }
  return false;
}

// Create a p_nft_collection instance if not yet done
void new_p_nft_collection() {
  if (p_nft_collection == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Allocating memory for p_nft_collection.");
    p_nft_collection = new (std::nothrow) NFTCollection();
    if (p_nft_collection == nullptr) {
      IC_API::trap("Allocation of p_nft_collection failed");
    }
  }
}

// Delete the p_nft_collection instance
void delete_p_nft_collection() {
  if (p_nft_collection) {
    delete p_nft_collection;
    p_nft_collection = nullptr;
  }
}

// Whitelist principals that are allowed to update NFTs
void nft_whitelist() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Only the canister owner is allowed to whitelist principals
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  // Do not call if static memory not yet set
  if (!p_nft_whitelist)
    IC_API::trap("Memory for p_nft_whitelist not yet allocated");

  // Read the whitelist items
  std::string principal_str;
  std::string description;

  CandidTypeRecord r_in;
  r_in.append("id", CandidTypePrincipal{&principal_str});
  r_in.append("description", CandidTypeText{&description});
  ic_api.from_wire(r_in);

  NFTWhitelistItem item;
  item.principal = CandidTypePrincipal{principal_str};
  item.description = description;
  p_nft_whitelist->whitelist.push_back(item);

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

// Initialize the NFT Collection
void nft_init() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Only the canister owner is allowed to initialize the NFT Collection
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  // Do not call if static memory not yet set
  if (!p_nft_collection)
    IC_API::trap("Memory for NFT Collection not yet allocated");

  // Protect from accidential re-initialization
  if (p_nft_collection->initialized)
    IC_API::trap("NFT Collection is already initialized");

  // Read the metadata
  uint64_t nft_supply_cap{0};
  uint64_t nft_total_supply{0}; // this input is ignored
  std::string nft_symbol;
  std::string nft_name;
  std::string nft_description;

  CandidTypeRecord r_in;
  r_in.append("nft_supply_cap", CandidTypeNat64{&nft_supply_cap});
  r_in.append("nft_total_supply", CandidTypeNat64{&nft_total_supply});
  r_in.append("nft_symbol", CandidTypeText{&nft_symbol});
  r_in.append("nft_name", CandidTypeText{&nft_name});
  r_in.append("nft_description", CandidTypeText{&nft_description});
  ic_api.from_wire(r_in);

  p_nft_collection->supply_cap = nft_supply_cap;
  p_nft_collection->symbol = nft_symbol;
  p_nft_collection->name = nft_name;
  p_nft_collection->description = nft_description;

  p_nft_collection->initialized = true;

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

// Get metadata of the NFT Collection
void nft_metadata() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  if (!p_nft_collection)
    IC_API::trap("Memory for NFT Collection not yet allocated");

  if (!p_nft_collection->initialized)
    IC_API::trap("NFT Collection not yet initialized");

  // Return a record summarizing the NFTCollection
  CandidTypeRecord r_out;
  r_out.append("nft_supply_cap", CandidTypeNat64(p_nft_collection->supply_cap));
  r_out.append("nft_total_supply",
               CandidTypeNat64(p_nft_collection->nfts.size()));
  r_out.append("nft_symbol", CandidTypeText(p_nft_collection->symbol));
  r_out.append("nft_name", CandidTypeText(p_nft_collection->name));
  r_out.append("nft_description",
               CandidTypeText(p_nft_collection->description));
  ic_api.to_wire(r_out);
}

// Create an NFT from the user's currently active chat
void nft_mint() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Only the canister owner is allowed to mint NFTs
  if (!is_canister_owner(ic_api, false)) IC_API::trap("Access Denied");

  // The token_id is passed by argument
  std::string token_id;
  CandidTypeRecord r_in;
  r_in.append("token_id", CandidTypeText{&token_id});
  ic_api.from_wire(r_in);

  // Have we reached the limit?
  if (p_nft_collection->nfts.size() >= p_nft_collection->supply_cap) {
    IC_API::trap("Cannot mint, reached supply_cap of " +
                 std::to_string(p_nft_collection->supply_cap));
  }

  // Is there already an NFT for this ordinal?
  for (const auto &existing_nft : p_nft_collection->nfts) {
    if (existing_nft.token_id == token_id) {
      IC_API::trap("An NFT for the token_id " + token_id + " already exists.");
    }
  }

  // Create the NFT with an empty story:
  // (-) We checked above that the caller must be the canister_owner
  // (-) We checked above that there is no other NFT for this token_id
  NFT nft;
  nft.token_id = token_id;

  // Store the NFT in the NFTCollection
  p_nft_collection->nfts.push_back(nft);

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

// Endpoints for the story of an NFT, callable by whitelisted principals only
void nft_story_start() { nft_story_(true); }
void nft_story_continue() { nft_story_(false); }

void nft_story_(bool story_start) {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_mode_nft_ordinal()) IC_API::trap("Access Denied");
  if (!nft_is_whitelisted(ic_api, false)) IC_API::trap("Access Denied");
  if (!is_ready_and_authorized(ic_api)) return;

  // Get the token_id & Prompt from the wire
  std::string token_id;
  CandidTypeRecord r_in1;
  r_in1.append("token_id", CandidTypeText{&token_id});

  Prompt wire_prompt;
  CandidTypeRecord r_in2;
  r_in2.append("prompt", CandidTypeText{&wire_prompt.prompt});
  r_in2.append("steps", CandidTypeNat64{&wire_prompt.steps});
  r_in2.append("temperature", CandidTypeFloat32{&wire_prompt.temperature});
  r_in2.append("topp", CandidTypeFloat32{&wire_prompt.topp});
  r_in2.append("rng_seed", CandidTypeNat64{&wire_prompt.rng_seed});

  CandidArgs args;
  args.append(r_in1);
  args.append(r_in2);
  ic_api.from_wire(args);

  print_prompt(wire_prompt);

  if (story_start or
      (p_chats && p_chats->umap.find(token_id) == p_chats->umap.end())) {
    // Does not yet exist
    build_new_chat(token_id);
  }
  Chat *chat = &p_chats->umap[token_id];
  std::string *output_history = &p_chats_output_history->umap[token_id];
  MetadataUser *metadata_user = &p_metadata_users->umap[token_id];

  do_inference(ic_api, wire_prompt, chat, output_history, metadata_user);
}

// For an NFT get the story
void nft_get_story() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // The token_id is passed by argument
  std::string token_id;
  CandidTypeRecord r_in;
  r_in.append("token_id", CandidTypeText{&token_id});
  ic_api.from_wire(r_in);

  if (p_chats && p_chats->umap.find(token_id) == p_chats->umap.end()) {
    IC_API::trap("NFT " + token_id + " does not exists.");
  }

  if (p_chats_output_history && p_chats_output_history->umap.find(token_id) ==
                                    p_chats_output_history->umap.end()) {
    IC_API::trap("The story for NFT " + token_id + " does not exists.");
  }

  ic_api.to_wire(CandidTypeVariant{
      "ok", CandidTypeText{p_chats_output_history->umap[token_id]}});
}