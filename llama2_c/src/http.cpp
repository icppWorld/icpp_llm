// Endpoints return familiar HTTP status codes in case of errors
#include "http.h"

#include "chats.h"
#include "nft_collection.h"
#include "ic_api.h"
#include <json/json.hpp>

#include <charconv>

void http_request() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Outcomment this line if you want to see the Candid that is sent to the canister.
  // IC_API::trap("Hex String from wire = " + ic_api.get_B_in().as_hex_string());

  IC_HttpRequest request;
  ic_api.from_wire(request);

  IC_API::debug_print(request.method);
  IC_API::debug_print(request.url);

  IC_HttpResponse response;
  uint16_t status_code;
  nlohmann::json j_out;

  if (request.method != "GET") {
    status_code = Http::MethodNotAllowed; // 405
    j_out["error"] = "Method Not Allowed: " + request.method;
  } else {

    // Extract the nft_id from /api/nft/<id>, where <id> is a 0-indexed number [0,1, ...]
    bool found_nft_id{false};
    int64_t nft_id;
    std::string prefix = "/api/nft/";
    if (request.url.rfind(prefix, 0) == 0) {
      std::string str = request.url.substr(prefix.length());
      if (!str.empty()) {

        // Convert it to a number, the index in the nfts vector
        auto [ptr, ec] =
            std::from_chars(str.data(), str.data() + str.size(), nft_id);
        if (ec == std::errc()) {
          // Conversion was successful
          found_nft_id = true;
        }
      }
    }

    // Error checks on the nft_id
    bool nft_id_ok{true};
    if (!found_nft_id) {
      status_code = Http::BadRequest; // 400
      j_out["error"] =
          "Bad URL " + request.url +
          "\nIt must be like /api/nft/<id>, with <id> in [0,1,2,...].";
      nft_id_ok = false;
    } else {
      IC_API::debug_print("nft_id = " + std::to_string(nft_id));

      // Check if this nft_id exists
      if (nft_id < 0) {
        status_code = Http::BadRequest; // 400
        j_out["error"] =
            "nft id can not be negative, but found " + std::to_string(nft_id);
        nft_id_ok = false;
      } else if (p_nft_collection->nfts.size() == 0) {
        status_code = Http::BadRequest; // 400
        j_out["error"] = "We did not yet mint any NFTs";
        nft_id_ok = false;
      } else if (nft_id + 1 > p_nft_collection->nfts.size()) {
        status_code = Http::BadRequest; // 400
        j_out["error"] = "The requested nft id (" + std::to_string(nft_id) +
                         ") is not available. We only minted up to nft id" +
                         std::to_string(p_nft_collection->nfts.size());
        nft_id_ok = false;
      }

      // If all good, we return the story
      if (nft_id_ok) {
        status_code = Http::OK;
        std::string token_id = p_nft_collection->nfts[nft_id].token_id;
        IC_API::debug_print("token_id = " + token_id);
        j_out["nft_id"] = nft_id;
        j_out["token_id"] = token_id;
        j_out["story"] = p_chats_output_history->umap[token_id];
      }
    }
  }

  response.status_code = status_code;

  IC_HeaderField contentTypeHeader;
  contentTypeHeader.name = "Content-Type";
  contentTypeHeader.value = "application/json";
  response.headers.push_back(contentTypeHeader);

  std::string s_out = j_out.dump();
  IC_HeaderField contentLengthHeader;
  contentLengthHeader.name = "Content-Length";
  contentLengthHeader.value = std::to_string(s_out.size());
  response.headers.push_back(contentLengthHeader);
  // store s_out in body as an std::vector<uint8_t>
  response.body.assign(s_out.data(), s_out.data() + s_out.size());

  response.upgrade = false;

  ic_api.to_wire(response);
}