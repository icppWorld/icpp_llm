// Initializes llama2's config, weights & tokenizer with the uploaded file-bytes

#include "initialize.h"

#include <string>
#include <unordered_map>
#include <variant>

#include "canister.h"
#include "chats.h"
#include "http.h"
#include "ic_api.h"
#include "upload.h"

#include "run.h"

// Orthogonally persisted model data
Transformer transformer;
Tokenizer tokenizer;

// -----------------------------------------------------------------------

// This is an exact copy of code in this method run.c,
// Modified to read the data from the uploaded tok4096.bin bytes
bool build_tokenizer(Tokenizer *t, int vocab_size, IC_API &ic_api) {
  if (!p_tokenizer_bytes or
      (p_tokenizer_bytes && p_tokenizer_bytes->vec.size() == 0)) {
    std::string error_msg = "ERROR: " + std::string(__func__) +
                            " tokenizer bytes were not yet uploaded!";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }
  // i should have written the vocab_size into the tokenizer file... sigh
  t->vocab_size = vocab_size;
  // malloc space to hold the scores and the strings
  t->vocab = (char **)malloc(vocab_size * sizeof(char *));
  if (!t->vocab) {
    std::string error_msg = "Failed to allocate memory for vocab.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }
  t->vocab_scores = (float *)malloc(vocab_size * sizeof(float));
  if (!t->vocab_scores) {
    std::string error_msg = "Failed to allocate memory for vocab_scores.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }

  for (int i = 0; i < 256; i++) {
    t->byte_pieces[i * 2] = (unsigned char)i;
    t->byte_pieces[i * 2 + 1] = '\0';
  }

  // read in the file
  // FILE *file = fopen(tokenizer_path, "rb");
  // if (!file) {
  //   fprintf(stderr, "couldn't load %s\n", tokenizer_path);
  //   exit(EXIT_FAILURE);
  // }
  // create a pointer to the start of the vector data
  const uint8_t *data_ptr = p_tokenizer_bytes->vec.data();

  // if (fread(&t->max_token_length, sizeof(int), 1, file) != 1) {
  //   fprintf(stderr, "failed read\n");
  //   exit(EXIT_FAILURE);
  // }
  // read max_token_length from data
  memcpy(&t->max_token_length, data_ptr, sizeof(int));
  data_ptr += sizeof(int);
  IC_API::debug_print("max_token_length = " +
                      std::to_string(t->max_token_length));

  int len;
  for (int i = 0; i < vocab_size; i++) {
    // if (fread(t->vocab_scores + i, sizeof(float), 1, file) != 1) {
    //   fprintf(stderr, "failed read\n");
    //   exit(EXIT_FAILURE);
    // }
    memcpy(t->vocab_scores + i, data_ptr, sizeof(float));
    data_ptr += sizeof(float);

    // if (fread(&len, sizeof(int), 1, file) != 1) {
    //   fprintf(stderr, "failed read\n");
    //   exit(EXIT_FAILURE);
    // }
    memcpy(&len, data_ptr, sizeof(int));
    data_ptr += sizeof(int);

    if (len <= 0 or len > t->max_token_length) {
      std::string error_msg;
      error_msg.append("ERROR: Memory for tokenizer is messed up.");
      error_msg.append("\nlen for token " + std::to_string(i) + " is " +
                       std::to_string(len));
      error_msg.append(
          "\nIt must be larger than 0 or less than max_token_length of " +
          std::to_string(t->max_token_length));

      ic_api.to_wire(CandidTypeVariant{
          "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
      return false;
    }

    t->vocab[i] = (char *)malloc(len + 1);
    if (!t->vocab[i]) {
      std::string error_msg = "Failed to allocate memory for vocab[" +
                              std::to_string(i) +
                              "] with len = " + std::to_string(len);
      ic_api.to_wire(CandidTypeVariant{
          "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
      return false;
    }
    // if (fread(t->vocab[i], len, 1, file) != 1) {
    //   fprintf(stderr, "failed read\n");
    //   exit(EXIT_FAILURE);
    // }
    memcpy(t->vocab[i], data_ptr, len);
    data_ptr += len;

    t->vocab[i][len] = '\0'; // add the string terminating token
  }
  // fclose(file);

  // Do this now, and not lazily
  // malloc and sort the vocabulary
  t->sorted_vocab = (TokenIndex *)malloc(t->vocab_size * sizeof(TokenIndex));
  if (!t->sorted_vocab) {
    std::string error_msg = "Failed to allocate memory for sorted_vocab.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return false;
  }

  for (int i = 0; i < t->vocab_size; i++) {
    t->sorted_vocab[i].str = t->vocab[i];
    t->sorted_vocab[i].id = i;
  }
  qsort(t->sorted_vocab, t->vocab_size, sizeof(TokenIndex), compare_tokens);

  // All OK
  return true;
}

// This is an exact copy of code in these methods of run.c
// - read_checkpoint
// - build_transformer
// - free_transformer
// Modified to read the data from the uploaded tok4096.bin bytes
void read_checkpoint(Config *config, TransformerWeights *weights) {
  // FILE *file = fopen(checkpoint, "rb");
  // if (!file) {
  //   fprintf(stderr, "Couldn't open file %s\n", checkpoint);
  //   exit(EXIT_FAILURE);
  // }
  // // read in the config header
  // if (fread(config, sizeof(Config), 1, file) != 1) {
  //   exit(EXIT_FAILURE);
  // }
  // Copy the data into config
  int *data = reinterpret_cast<int *>(p_model_bytes->vec.data());
  memcpy(config, data, sizeof(Config));

  IC_API::debug_print("-------------------------------------");
  IC_API::debug_print("config.dim        = " + std::to_string(config->dim));
  IC_API::debug_print("config.hidden_dim = " +
                      std::to_string(config->hidden_dim));
  IC_API::debug_print("config.n_layers   = " +
                      std::to_string(config->n_layers));
  IC_API::debug_print("config.n_heads    = " + std::to_string(config->n_heads));
  IC_API::debug_print("config.n_kv_heads = " +
                      std::to_string(config->n_kv_heads));
  IC_API::debug_print("config.vocab_size = " +
                      std::to_string(config->vocab_size));
  IC_API::debug_print("config.seq_len    = " + std::to_string(config->seq_len));
  IC_API::debug_print("-------------------------------------");

  // negative vocab size is hacky way of signaling unshared weights. bit yikes.
  int shared_weights = config->vocab_size > 0 ? 1 : 0;
  config->vocab_size = abs(config->vocab_size);
  // // figure out the file size
  // fseek(file, 0, SEEK_END); // move file pointer to end of file
  // *file_size = ftell(file); // get the file size, in bytes
  // fclose(file);
  // // memory map the Transformer weights into the data pointer
  // *fd = open(checkpoint, O_RDONLY); // open in read only mode
  // if (*fd == -1) {
  //   fprintf(stderr, "open failed!\n");
  //   exit(EXIT_FAILURE);
  // }
  // *data = mmap(NULL, *file_size, PROT_READ, MAP_PRIVATE, *fd, 0);
  // if (*data == MAP_FAILED) {
  //   fprintf(stderr, "mmap failed!\n");
  //   exit(EXIT_FAILURE);
  // }
  // float *weights_ptr = *data + sizeof(Config) / sizeof(float);
  // Copy the data into weights
  float *weights_ptr =
      reinterpret_cast<float *>(data + sizeof(Config) / sizeof(int));

  memory_map_weights(weights, config, weights_ptr, shared_weights);
}

bool build_transformer(Transformer *t) {
  // read in the Config and the Weights from the checkpoint
  read_checkpoint(&t->config, &t->weights);

  // icpp: moved into build_active_chat
  // // allocate the RunState buffers
  // malloc_run_state(&t->state, &t->config);

  // //icpp: initialize the token generation settings
  // reset_tokens(t);

  return true;
}

void initialize() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_owner(ic_api, true)) return;

  build_transformer(&transformer);
  if (!build_tokenizer(&tokenizer, transformer.config.vocab_size, ic_api))
    return;

  ready_for_inference = true;

  CandidTypeRecord status_code_record;
  status_code_record.append("status_code",
                            CandidTypeNat16{Http::StatusCode::OK});
  ic_api.to_wire(CandidTypeVariant{"Ok", status_code_record});
}

void print_config() {
  Config config = transformer.config;
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
  Config config = transformer.config;
  r_out.append("dim", CandidTypeInt{config.dim});
  r_out.append("hidden_dim", CandidTypeInt{config.hidden_dim});
  r_out.append("n_layers", CandidTypeInt{config.n_layers});
  r_out.append("n_heads", CandidTypeInt{config.n_heads});
  r_out.append("n_kv_heads", CandidTypeInt{config.n_kv_heads});
  r_out.append("vocab_size", CandidTypeInt{config.vocab_size});
  r_out.append("seq_len", CandidTypeInt{config.seq_len});
  ic_api.to_wire(r_out);
}