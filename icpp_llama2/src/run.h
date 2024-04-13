#pragma once

// Enable calling from C++
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/types.h>

// ----------------------------------------------------------------------------
// Transformer and RunState structs, and related memory management

typedef struct {
  int dim;        // transformer dimension
  int hidden_dim; // for ffn layers
  int n_layers;   // number of layers
  int n_heads;    // number of query heads
  int n_kv_heads; // number of key/value heads (can be < query heads because of multiquery)
  int vocab_size; // vocabulary size, usually 256 (byte-level)
  int seq_len;    // max sequence length
} Config;

typedef struct {
  // token embedding table
  float *token_embedding_table; // (vocab_size, dim)
  // weights for rmsnorms
  float *rms_att_weight; // (layer, dim) rmsnorm weights
  float *rms_ffn_weight; // (layer, dim)
  // weights for matmuls. note dim == n_heads * head_size
  float *wq; // (layer, dim, n_heads * head_size)
  float *wk; // (layer, dim, n_kv_heads * head_size)
  float *wv; // (layer, dim, n_kv_heads * head_size)
  float *wo; // (layer, n_heads * head_size, dim)
  // weights for ffn
  float *w1; // (layer, hidden_dim, dim)
  float *w2; // (layer, dim, hidden_dim)
  float *w3; // (layer, hidden_dim, dim)
  // final rmsnorm
  float *rms_final_weight; // (dim,)
  // (optional) classifier weights for the logits, on the last layer
  float *wcls;
} TransformerWeights;

typedef struct {
  // current wave of activations
  float *x;      // activation at current time stamp (dim,)
  float *xb;     // same, but inside a residual branch (dim,)
  float *xb2;    // an additional buffer just for convenience (dim,)
  float *hb;     // buffer for hidden dimension in the ffn (hidden_dim,)
  float *hb2;    // buffer for hidden dimension in the ffn (hidden_dim,)
  float *q;      // query (dim,)
  float *k;      // key (dim,)
  float *v;      // value (dim,)
  float *att;    // buffer for scores/attention values (n_heads, seq_len)
  float *logits; // output logits
  // kv cache
  float *key_cache;   // (layer, seq_len, dim)
  float *value_cache; // (layer, seq_len, dim)
} RunState;

typedef struct {
  Config config; // the hyperparameters of the architecture (the blueprint)
  TransformerWeights weights; // the weights of the model
  // icpp: we are storing RunState per user, not per model
  // RunState state; // buffers for the "wave" of activations in the forward pass
  // some more state needed to properly clean up the memory mapping (sigh)
  // icpp: we do not use these, because we do not read from file
  // int fd;            // file descriptor for memory mapping
  // float *data;       // memory mapped data pointer
  // ssize_t file_size; // size of the checkpoint file in bytes
} Transformer;

typedef struct {
  RunState state; // buffers for the "wave" of activations in the forward pass
  // icpp: to support generation across endpoint calls, we need to save the next token predicted
  int pos;    // position in the sequence
  int next;   // next token that was predicted
  int8_t bos; // add begin-of-sentence token or not
  int8_t eos; // add end-of-sentence token or not
  unsigned long long
      total_steps; // total steps generated, including previous calls
  unsigned long long
      inference_steps; // actual steps generated during current inference call, excluding previous calls
} Chat;

typedef struct {
  char *str;
  int id;
} TokenIndex;

typedef struct {
  char **vocab;
  float *vocab_scores;
  TokenIndex *sorted_vocab;
  int vocab_size;
  unsigned int max_token_length;
  unsigned char byte_pieces[512]; // stores all single-byte strings
} Tokenizer;

typedef struct {
  float prob;
  int index;
} ProbIndex; // struct used when sorting probabilities during top-p sampling

typedef struct {
  int vocab_size;
  ProbIndex *probindex; // buffer used in top-p sampling
  float temperature;
  float topp;
  unsigned long long rng_state;
} Sampler;

extern Config config;
extern Transformer transformer;
extern Tokenizer tokenizer;
extern Sampler sampler;

// At inference
extern unsigned long long rng_seed;

int compare_tokens(const void *a, const void *b);
bool malloc_run_state(RunState *s, Config *p);
void memory_map_weights(TransformerWeights *w, Config *p, float *ptr,
                        int shared_weights);
void encode(Tokenizer *t, const char *text, int bos, int eos, int *tokens,
            int *n_tokens, int *error_code);
float *forward(Chat *chat, Transformer *transformer, int token, int pos);
char *decode(Tokenizer *t, int prev_token, int token);
void build_sampler(Sampler *sampler, int vocab_size, float temperature,
                   float topp, unsigned long long rng_seed);
int sample(Sampler *sampler, float *logits);
int sample_topp(float *probabilities, int n, float topp, ProbIndex *probindex,
                float coin);

void free_run_state(RunState *s);
void free_sampler(Sampler *sampler);
void free_tokenizer(Tokenizer *t);
// void free_transformer(Transformer *t);

#ifdef __cplusplus
}
#endif