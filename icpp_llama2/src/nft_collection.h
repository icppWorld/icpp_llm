#pragma once

#include <string>
#include <unordered_map>

#include "ic_api.h"
#include "run.h"
#include "wasm_symbol.h"

void nft_whitelist() WASM_SYMBOL_EXPORTED("canister_update nft_whitelist");
void nft_ami_whitelisted()
    WASM_SYMBOL_EXPORTED("canister_query nft_ami_whitelisted");
void nft_init() WASM_SYMBOL_EXPORTED("canister_update nft_init");
void nft_metadata() WASM_SYMBOL_EXPORTED("canister_query nft_metadata");
void nft_mint() WASM_SYMBOL_EXPORTED("canister_update nft_mint");
void nft_story_start() WASM_SYMBOL_EXPORTED("canister_update nft_story_start");
void nft_story_start_mo()
    WASM_SYMBOL_EXPORTED("canister_update nft_story_start_mo");
void nft_story_continue()
    WASM_SYMBOL_EXPORTED("canister_update nft_story_continue");
void nft_story_continue_mo()
    WASM_SYMBOL_EXPORTED("canister_update nft_story_continue_mo");
void nft_get_story() WASM_SYMBOL_EXPORTED("canister_query nft_get_story");

// ------------------------------------------------
// Orthogonal persistent data

// Whitelisted principals that can update NFTs
class NFTWhitelistItem {
public:
  CandidTypePrincipal principal;
  std::string description;
};

class NFTWhitelist {
public:
  std::vector<NFTWhitelistItem> whitelist;
};
extern NFTWhitelist *p_nft_whitelist;

class NFT {
public:
  std::string token_id;
};

// Chats saved as an NFT Collection
class NFTCollection {
public:
  bool initialized{false};
  // icrc-7 compliant metadata
  uint64_t supply_cap{0};
  std::string symbol;
  std::string name;
  std::string description;

  // internal storage
  std::vector<NFT> nfts;
};
extern NFTCollection *p_nft_collection;

// ------------------------------------------------
void new_p_nft_whitelist();
void delete_p_nft_whitelist();
bool nft_is_whitelisted(IC_API &ic_api, bool err_to_wire = true);
void new_p_nft_collection();
void delete_p_nft_collection();
void nft_story_(bool story_start, bool from_motoko);
bool nft_exists_(const std::string &token_id);
bool nft_story_exists_(const std::string &token_id);
