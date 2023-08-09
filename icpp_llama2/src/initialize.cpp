// Initializes llama2's config, weights & tokenizer with the uploaded file-bytes

#include "initialize.h"

#include <string>
#include <unordered_map>
#include <variant>

#include "canister.h"
#include "http.h"
#include "ic_api.h"
#include "upload.h"

#include "run.h"

Config config;
TransformerWeights weights;
char **vocab;
float *vocab_scores;
unsigned int max_token_length;

// This is an exact copy of run.c,
// Modified to read the data from the uploaded tokenizer.bin bytes
void initialize_tokenizer() {
  // create a pointer to the start of the vector data
  const uint8_t *data_ptr = p_tokenizer_bytes->vec.data();

  // allocate memory for vocab and vocab_scores
  vocab = (char **)malloc(config.vocab_size * sizeof(char *));
  vocab_scores = (float *)malloc(config.vocab_size * sizeof(float));

  // read max_token_length from data
  memcpy(&max_token_length, data_ptr, sizeof(int));
  data_ptr += sizeof(int);

  int len;
  for (int i = 0; i < config.vocab_size; i++) {
    memcpy(vocab_scores + i, data_ptr, sizeof(float));
    data_ptr += sizeof(float);

    memcpy(&len, data_ptr, sizeof(int));
    data_ptr += sizeof(int);

    vocab[i] = (char *)malloc(len + 1);

    memcpy(vocab[i], data_ptr, len);
    data_ptr += len;

    vocab[i][len] = '\0'; // add the string terminating token
  }
}

void initialize() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_owner(ic_api)) return;

  // Copy the data into config
  int *data = reinterpret_cast<int *>(p_model_bytes->vec.data());
  memcpy(&config, data, sizeof(Config));

  // negative vocab size is hacky way of signaling unshared weights. bit yikes.
  int shared_weights = config.vocab_size > 0 ? 1 : 0;
  config.vocab_size = abs(config.vocab_size);

  // Copy the data into weights
  float *weights_ptr =
      reinterpret_cast<float *>(data + sizeof(Config) / sizeof(int));
  checkpoint_init_weights(&weights, &config, weights_ptr, shared_weights);

  // initialize tokenizer
  initialize_tokenizer();

  ready_for_inference = true;

  ic_api.to_wire(
      CandidTypeVariant{"ok", CandidTypeNat16{Http::StatusCode::OK}});
}

void print_config() {
  std::string msg = std::string(__func__) + "model config:";
  msg += "\nconfig.dim             = " + std::to_string(config.dim);
  msg += "\nconfig.hidden_dim      = " + std::to_string(config.hidden_dim);
  msg += "\nconfig.n_layers        = " + std::to_string(config.n_layers);
  msg += "\nconfig.n_heads         = " + std::to_string(config.n_heads);
  msg += "\nconfig.n_kv_heads      = " + std::to_string(config.n_kv_heads);
  msg += "\nconfig.vocab_size      = " + std::to_string(config.vocab_size);
  msg += "\nconfig.seq_len         = " + std::to_string(config.seq_len);
  IC_API::debug_print(msg);
}

void get_model_config() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  CandidTypePrincipal caller = ic_api.get_caller();
  std::string principal = caller.get_text();

  print_config();

  CandidTypeRecord r_out;
  r_out.append("dim", CandidTypeInt{config.dim});
  r_out.append("hidden_dim", CandidTypeInt{config.hidden_dim});
  r_out.append("n_layers", CandidTypeInt{config.n_layers});
  r_out.append("n_heads", CandidTypeInt{config.n_heads});
  r_out.append("n_kv_heads", CandidTypeInt{config.n_kv_heads});
  r_out.append("vocab_size", CandidTypeInt{config.vocab_size});
  r_out.append("seq_len", CandidTypeInt{config.seq_len});
  ic_api.to_wire(r_out);
}