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
      // called from canister_init, so trap is correct!
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
    ic_api.to_wire(CandidTypeVariant{"Err", CandidTypeNat16{status_code}});
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
      // called from canister_init, so trap is correct!
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
  if (!is_canister_owner(ic_api, false)) {
    std::string error_msg = "Access Denied";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  // Do not call if static memory not yet set
  if (!p_nft_whitelist) {
    std::string error_msg = "Memory for p_nft_whitelist not yet allocated";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

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

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

void nft_ami_whitelisted() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  if (!nft_is_whitelisted(ic_api, false)) {
    std::string error_msg = "Access Denied";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

// Initialize the NFT Collection
void nft_init() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Only the canister owner is allowed to initialize the NFT Collection
  if (!is_canister_owner(ic_api, false)) {
    std::string error_msg = "Access Denied";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  // Do not call if static memory not yet set
  if (!p_nft_collection) {
    std::string error_msg = "Memory for NFT Collection not yet allocated";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  // Protect from accidential re-initialization
  if (p_nft_collection->initialized) {
    std::string error_msg = "NFT Collection is already initialized";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

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

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

// Get metadata of the NFT Collection
void nft_metadata() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  if (!p_nft_collection) {
    std::string error_msg = "Memory for NFT Collection not yet allocated";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  if (!p_nft_collection->initialized) {
    std::string error_msg = "NFT Collection not yet initialized";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  // Return a NFTCollectionRecordResult
  CandidTypeRecord nft_collection_record;
  nft_collection_record.append("nft_supply_cap",
                               CandidTypeNat64(p_nft_collection->supply_cap));
  nft_collection_record.append("nft_total_supply",
                               CandidTypeNat64(p_nft_collection->nfts.size()));
  nft_collection_record.append("nft_symbol",
                               CandidTypeText(p_nft_collection->symbol));
  nft_collection_record.append("nft_name",
                               CandidTypeText(p_nft_collection->name));
  nft_collection_record.append("nft_description",
                               CandidTypeText(p_nft_collection->description));
  ic_api.to_wire(
      CandidTypeVariant{"Ok", CandidTypeRecord{nft_collection_record}});
}

// Checks if an NFT exists in the collection
bool nft_exists_(const std::string &token_id) {
  for (const auto &existing_nft : p_nft_collection->nfts) {
    if (existing_nft.token_id == token_id) {
      return true;
    }
  }
  return false;
}

// Create an NFT from the user's currently active chat
void nft_mint() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Only the canister owner is allowed to mint NFTs
  if (!is_canister_owner(ic_api, false)) {
    std::string error_msg = "Access Denied";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  // The token_id is passed by argument
  std::string token_id;
  CandidTypeRecord r_in;
  r_in.append("token_id", CandidTypeText{&token_id});
  ic_api.from_wire(r_in);

  // Have we reached the limit? (supply_cap of zero means no limit)
  if (p_nft_collection->supply_cap != 0) {
    if (p_nft_collection->nfts.size() >= p_nft_collection->supply_cap) {
      std::string error_msg = "Cannot mint, reached supply_cap of " +
                              std::to_string(p_nft_collection->supply_cap);
      ic_api.to_wire(CandidTypeVariant{
          "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
      return;
    }
  }

  if (nft_exists_(token_id)) {
    // We don't care if it already exists
    // std::string error_msg =
    //     "An NFT for the token_id " + token_id + " already exists.";
    // ic_api.to_wire(CandidTypeVariant{
    //     "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    // return;
  } else {

    // Create the NFT with an empty story:
    // (-) We checked above that the caller must be the canister_owner
    // (-) We checked above that there is no other NFT for this token_id
    NFT nft;
    nft.token_id = token_id;

    // Store the NFT in the NFTCollection
    p_nft_collection->nfts.push_back(nft);
  }
  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

// Endpoints for the story of an NFT, callable by whitelisted principals only
void nft_story_start() { nft_story_(true, false); }
void nft_story_start_mo() { nft_story_(true, true); }
void nft_story_continue() { nft_story_(false, false); }
void nft_story_continue_mo() { nft_story_(false, true); }

void nft_story_(bool story_start, bool from_motoko) {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_mode_nft_ordinal()) {
    std::string error_msg = "Access Denied - Canister is not in NFT mode.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }
  if (!nft_is_whitelisted(ic_api, false)) {
    std::string error_msg =
        "Access Denied - You are not authorized to call this function.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }
  if (!is_ready_and_authorized(ic_api)) return;

  // Get the token_id & Prompt from the wire
  std::string token_id;
  CandidTypeRecord r_in1;
  r_in1.append("token_id", CandidTypeText{&token_id});

  PromptMo wire_prompt_motoko; // Motoko does not support float32, uses float64
  Prompt wire_prompt;
  CandidTypeRecord r_in2;
  r_in2.append("prompt", CandidTypeText{&wire_prompt.prompt});
  r_in2.append("steps", CandidTypeNat64{&wire_prompt.steps});
  if (from_motoko) {
    r_in2.append("temperature",
                 CandidTypeFloat64{&wire_prompt_motoko.temperature});
    r_in2.append("topp", CandidTypeFloat64{&wire_prompt_motoko.topp});
  } else {
    r_in2.append("temperature", CandidTypeFloat32{&wire_prompt.temperature});
    r_in2.append("topp", CandidTypeFloat32{&wire_prompt.topp});
  }
  r_in2.append("rng_seed", CandidTypeNat64{&wire_prompt.rng_seed});

  CandidArgs args;
  args.append(r_in1);
  args.append(r_in2);
  ic_api.from_wire(args);

  if (from_motoko) {
    wire_prompt.temperature =
        static_cast<float>(wire_prompt_motoko.temperature);
    wire_prompt.topp = static_cast<float>(wire_prompt_motoko.topp);
  }

  print_prompt(wire_prompt);

  if (story_start or
      (p_chats && p_chats->umap.find(token_id) == p_chats->umap.end())) {
    // Does not yet exist
    if (!build_new_chat(token_id, ic_api)) return;
  }
  Chat *chat = &p_chats->umap[token_id];
  std::string *output_history = &p_chats_output_history->umap[token_id];
  MetadataUser *metadata_user = &p_metadata_users->umap[token_id];

  bool error{false};
  std::string output = do_inference(ic_api, wire_prompt, chat, output_history,
                                    metadata_user, &error);

  if (error) {
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{output}}});
    return;
  }

  // IC_API::debug_print(output);
  // Send the generated response to the wire
  CandidTypeRecord inference_record;
  inference_record.append("inference", CandidTypeText{output});
  inference_record.append("num_tokens", CandidTypeNat64{chat->inference_steps});
  ic_api.to_wire(CandidTypeVariant{"Ok", CandidTypeRecord{inference_record}});
}

// Checks if a story exists for an NFT in the collection
bool nft_story_exists_(const std::string &token_id) {
  if ((p_chats && p_chats->umap.find(token_id) != p_chats->umap.end()) &&
      (p_chats_output_history && p_chats_output_history->umap.find(token_id) !=
                                     p_chats_output_history->umap.end())) {
    std::string story = p_chats_output_history->umap[token_id];
    if (!story.empty()) {
      return true;
    }
  }
  return false;
}

// For an NFT get the story
void nft_get_story() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // The token_id is passed by argument
  std::string token_id;
  CandidTypeRecord r_in;
  r_in.append("token_id", CandidTypeText{&token_id});
  ic_api.from_wire(r_in);

  std::string error_msg;
  if (!nft_exists_(token_id)) {
    error_msg = "NFT " + token_id + " does not exists.";
  } else if (!nft_story_exists_(token_id)) {
    error_msg = "The story for NFT " + token_id + " does not exists.";
  }
  if (!error_msg.empty()) {
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  CandidTypeRecord story_record;
  story_record.append("story",
                      CandidTypeText{p_chats_output_history->umap[token_id]});
  ic_api.to_wire(CandidTypeVariant{"Ok", story_record});
}