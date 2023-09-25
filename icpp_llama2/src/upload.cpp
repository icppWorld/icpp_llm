// Endpoints for uploading the trained model           & the tokenizer files
// eg.                         models/stories15M.bin   & tokenizers/tokenizer.bin

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

void new_model_bytes_memory() {
  if (p_model_bytes == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating ModelBytes Instance.");
    p_model_bytes = new (std::nothrow) ModelBytes();
    if (p_model_bytes == nullptr) {
      IC_API::trap("Allocation of p_model_bytes failed");
    }
  }
}

void delete_model_bytes_memory() {
  if (p_model_bytes) {
    delete p_model_bytes;
    p_model_bytes = nullptr;
  }
}

void new_tokenizer_bytes_memory() {
  if (p_tokenizer_bytes == nullptr) {
    IC_API::debug_print(std::string(__func__) +
                        ": Creating TokenizerBytes Instance.");
    p_tokenizer_bytes = new (std::nothrow) TokenizerBytes();
    if (p_tokenizer_bytes == nullptr) {
      IC_API::trap("Allocation of p_tokenizer_bytes failed");
    }
  }
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
  if (!is_owner(ic_api)) return;

  ready_for_inference = false;

  delete_model_bytes_memory();

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

void reset_tokenizer() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_owner(ic_api)) return;

  ready_for_inference = false;

  delete_tokenizer_bytes_memory();

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

// Endpoint for uploading the stories15M.bin file as bytes
void upload_model_bytes_chunk() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_owner(ic_api)) return;

  std::vector<uint8_t> v;
  ic_api.from_wire(CandidTypeVecNat8{&v});

  if (p_model_bytes == nullptr) new_model_bytes_memory();
  p_model_bytes->vec.insert(p_model_bytes->vec.end(), v.begin(), v.end());

  print_upload_model_bytes_summary(std::string(__func__), v);

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

// Endpoint for uploading the tokenizer.bin file as bytes
void upload_tokenizer_bytes_chunk() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_owner(ic_api)) return;

  std::vector<uint8_t> v;
  ic_api.from_wire(CandidTypeVecNat8{&v});

  if (p_tokenizer_bytes == nullptr) new_tokenizer_bytes_memory();
  p_tokenizer_bytes->vec.insert(p_tokenizer_bytes->vec.end(), v.begin(),
                                v.end());

  print_upload_tokenizer_bytes_summary(std::string(__func__), v);

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}