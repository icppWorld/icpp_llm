#pragma once

// Enable calling from C++
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

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
  // freq_cis for RoPE relatively positional embeddings
  float *freq_cis_real; // (seq_len, head_size/2)
  float *freq_cis_imag; // (seq_len, head_size/2)
  // (optional) classifier weights for the logits, on the last layer
  float *wcls;
} TransformerWeights;

typedef struct {
  float prob;
  int index;
} ProbIndex; // struct used when sorting probabilities during top-p sampling

typedef struct {
  // current wave of activations
  float *x;             // activation at current time stamp (dim,)
  float *xb;            // same, but inside a residual branch (dim,)
  float *xb2;           // an additional buffer just for convenience (dim,)
  float *hb;            // buffer for hidden dimension in the ffn (hidden_dim,)
  float *hb2;           // buffer for hidden dimension in the ffn (hidden_dim,)
  float *q;             // query (dim,)
  float *k;             // key (dim,)
  float *v;             // value (dim,)
  float *att;           // buffer for scores/attention values (n_heads, seq_len)
  float *logits;        // output logits
  ProbIndex *probindex; // buffer used in top-p sampling
  // kv cache
  float *key_cache;   // (layer, seq_len, dim)
  float *value_cache; // (layer, seq_len, dim)
} RunState;

// stories15M.bin
extern Config config;
extern TransformerWeights weights;

// tokenizer.bin
extern char **vocab;
extern float *vocab_scores;
extern unsigned int max_token_length;

// At inference
extern unsigned long long rng_seed;

bool malloc_run_state(RunState *s, Config *p);
void free_run_state(RunState *s);
void checkpoint_init_weights(TransformerWeights *w, Config *p, float *f,
                             int shared_weights);
bool bpe_encode(const char *text, char **vocab, float *vocab_scores,
                int vocab_size, unsigned int max_token_length, int *tokens,
                int *n_tokens);
void transformer(int token, int pos, Config *p, RunState *s,
                 TransformerWeights *w);
int argmax(float *v, int n);
void softmax(float *x, int size);
int sample(float *probabilities, int n);
int sample_topp(float *probabilities, int n, float topp, ProbIndex *probindex);

#ifdef __cplusplus
}
#endif