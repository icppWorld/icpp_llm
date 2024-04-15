// Endpoints for uploading the trained model           & the tokenizer files
// eg.                         models/stories15Mtok4096.bin   & tokenizers/tok4096.bin

#include "upload.h"

#include <string>

#include "canister.h"
#include "http.h"
#include "ic_api.h"

ModelBytes *p_model_bytes{nullptr};
TokenizerBytes *p_tokenizer_bytes{nullptr};

// 0 - none
// 1 - minimal
// 2 - a lot
int DEBUG_VERBOSE = 1;

bool new_model_bytes_memory(IC_API &ic_api) {
  if (p_model_bytes == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating ModelBytes Instance.");
    p_model_bytes = new (std::nothrow) ModelBytes();
    if (p_model_bytes == nullptr) {
      std::string error_msg = "Allocation of p_model_bytes failed";
      ic_api.to_wire(CandidTypeVariant{
          "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
      return false;
    }
  }
  return true;
}

void delete_model_bytes_memory() {
  if (p_model_bytes) {
    delete p_model_bytes;
    p_model_bytes = nullptr;
  }
}

bool new_tokenizer_bytes_memory(IC_API &ic_api) {
  if (p_tokenizer_bytes == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating TokenizerBytes Instance.");
    p_tokenizer_bytes = new (std::nothrow) TokenizerBytes();
    if (p_tokenizer_bytes == nullptr) {
      std::string error_msg = "Allocation of p_tokenizer_bytes failed";
      ic_api.to_wire(CandidTypeVariant{
          "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
      return false;
    }
  }
  return true;
}

void delete_tokenizer_bytes_memory() {
  if (p_tokenizer_bytes) {
    delete p_tokenizer_bytes;
    p_tokenizer_bytes = nullptr;
  }
}

void print_upload_model_bytes_summary(std::string calling_function,
                                      const std::vector<uint8_t> &v) {
  std::string msg;
  if (DEBUG_VERBOSE == 0) {
    return;
  } else if (DEBUG_VERBOSE == 1) {
    msg += "chunk size = " + std::to_string(v.size()) +
           "; total size = " + std::to_string(p_model_bytes->vec.size());
  } else {
    msg += calling_function + ":";
    msg += "\n- received v<uint8_t> of size " + std::to_string(v.size());
    msg += "\n- v[0]                  = " + std::to_string(v[0]);
    msg += "\n- v.back()              = " + std::to_string(v.back());
    msg += "\n- p_model_bytes->vec now has size " +
           std::to_string(p_model_bytes->vec.size());
    msg += "\n- p_model_bytes->vec[0]     = " +
           std::to_string(p_model_bytes->vec[0]);
    msg += "\n- p_model_bytes->vec.back() = " +
           std::to_string(p_model_bytes->vec.back());
  }
  IC_API::debug_print(msg);
}

void print_upload_tokenizer_bytes_summary(std::string calling_function,
                                          const std::vector<uint8_t> &v) {
  std::string msg;
  if (DEBUG_VERBOSE == 0) {
    return;
  } else if (DEBUG_VERBOSE == 1) {
    msg += "chunk size = " + std::to_string(v.size()) +
           "; total size = " + std::to_string(p_model_bytes->vec.size());
  } else {
    msg += calling_function + ":";
    msg += "\n- received v<uint8_t> of size " + std::to_string(v.size());
    msg += "\n- v[0]                  = " + std::to_string(v[0]);
    msg += "\n- v.back()              = " + std::to_string(v.back());
    msg += "\n- p_tokenizer_bytes->vec now has size " +
           std::to_string(p_tokenizer_bytes->vec.size());
    msg += "\n- p_tokenizer_bytes->vec[0]     = " +
           std::to_string(p_tokenizer_bytes->vec[0]);
    msg += "\n- p_tokenizer_bytes->vec.back() = " +
           std::to_string(p_tokenizer_bytes->vec.back());
  }
  IC_API::debug_print(msg);
}

void reset_model() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api)) return;

  ready_for_inference = false;

  delete_model_bytes_memory();

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

void reset_tokenizer() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api)) return;

  ready_for_inference = false;

  delete_tokenizer_bytes_memory();

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

// Endpoint for uploading the stories15Mtok4096.bin file as bytes
void upload_model_bytes_chunk() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api)) return;

  std::vector<uint8_t> v;
  ic_api.from_wire(CandidTypeVecNat8{&v});

  if (p_model_bytes == nullptr) {
    if (!new_model_bytes_memory(ic_api)) return;
  }
  p_model_bytes->vec.insert(p_model_bytes->vec.end(), v.begin(), v.end());

  print_upload_model_bytes_summary(std::string(__func__), v);

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

// Endpoint for uploading the tok4096.bin file as bytes
void upload_tokenizer_bytes_chunk() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api)) return;

  std::vector<uint8_t> v;
  ic_api.from_wire(CandidTypeVecNat8{&v});

  if (p_tokenizer_bytes == nullptr) {
    if (!new_tokenizer_bytes_memory(ic_api)) return;
  }
  p_tokenizer_bytes->vec.insert(p_tokenizer_bytes->vec.end(), v.begin(),
                                v.end());

  print_upload_tokenizer_bytes_summary(std::string(__func__), v);

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}