// Inference endpoint generating a token string for a given prompt

#include "inference.h"

#include <string>
#include <unordered_map>
#include <variant>

#include "canister.h"
#include "http.h"
#include "initialize.h"
#include "run.h"
#include "upload.h"

#include "ic_api.h"

class Prompt {
public:
  std::string prompt{""};
  uint64_t steps{256};
  float temperature{1.0};
  float topp{1.0};
};

void print_prompt(const Prompt &prompt) {
  std::string msg = std::string(__func__) + "- model config:";
  msg += "\nprompt.prompt       = " + prompt.prompt;
  msg += "\nprompt.steps        = " + std::to_string(prompt.steps);
  msg += "\nprompt.temperature  = " + std::to_string(prompt.temperature);
  msg += "\nprompt.topp         = " + std::to_string(prompt.topp);
  IC_API::debug_print(msg);
}

// Copied from main() of run.c and modified slightly
std::string generate(IC_API ic_api, const Prompt &prompt_,
                     uint64_t *token_per_sec, std::string *sampling_method) {
  std::string output;

  // 0.0 = greedy deterministic. 1.0 = original. don't set higher
  float temperature = prompt_.temperature;
  // top-p in nucleus sampling. 1.0 = off. 0.9 works well, but slower
  float topp = prompt_.topp;
  int steps = prompt_.steps; // max number of steps to run for, 0: use seq_len
  const char *prompt = NULL; // prompt string
  if (prompt_.prompt.size() > 0) prompt = prompt_.prompt.c_str();

  // seed rng with time. if you want deterministic behavior use temperature 0.0
  rng_seed = ic_api.time(); // time in ns

  // right now we cannot run for more than config.seq_len steps
  if (steps <= 0 || steps > config.seq_len) {
    steps = config.seq_len;
  }

  // create and init the application RunState
  RunState state;
  if (!malloc_run_state(&state, &config))
    IC_API::trap("malloc_run_state failed!");

  // process the prompt, if any
  int *prompt_tokens = NULL;
  int num_prompt_tokens = 0;
  if (prompt != NULL) {
    prompt_tokens = (int *)malloc(strlen(prompt) * sizeof(int));
    if (!bpe_encode(prompt, vocab, vocab_scores, config.vocab_size,
                    max_token_length, prompt_tokens, &num_prompt_tokens))
      IC_API::trap("ERROR: bpe_encode of the prompt failed.");
  }

  if (temperature == 0.0f) {
    *sampling_method =
        "greedy argmax sampling: take the token with the highest probability";
  } else {
    if (topp <= 0 || topp >= 1) {
      *sampling_method =
          "simply sample from the predicted probability distribution";
    } else {
      *sampling_method =
          "top-p (nucleus) sampling, clamping the least likely tokens to zero";
    }
  }

  // start the main loop
  // used to time our code, only initialized after first iteration
  uint64_t start = 0;
  // will store the next token in the sequence
  int next;
  // init with token 1 (=BOS), as done in Llama-2 sentencepiece tokenizer
  int token = 1;
  int pos = 0; // position in the sequence
  while (pos < steps) {

    // forward the transformer to get logits for the next token
    transformer(token, pos, &config, &state, &weights);

    // advance the state state machine
    if (pos < num_prompt_tokens) {
      // if we are still processing the input prompt, force the next prompt token
      next = prompt_tokens[pos];
    } else {
      // sample the next token
      if (temperature == 0.0f) {
        // greedy argmax sampling: take the token with the highest probability
        next = argmax(state.logits, config.vocab_size);
      } else {
        // apply the temperature to the logits
        for (int q = 0; q < config.vocab_size; q++) {
          state.logits[q] /= temperature;
        }
        // apply softmax to the logits to get the probabilities for next token
        softmax(state.logits, config.vocab_size);
        // we sample from this distribution to get the next token
        if (topp <= 0 || topp >= 1) {
          // simply sample from the predicted probability distribution
          next = sample(state.logits, config.vocab_size);
        } else {
          // top-p (nucleus) sampling, clamping the least likely tokens to zero
          next = sample_topp(state.logits, config.vocab_size, topp,
                             state.probindex);
        }
      }
    }
    pos++;

    // data-dependent terminating condition: the BOS (1) token delimits sequences
    if (next == 1) {
      break;
    }

    // following BOS (1) token, sentencepiece decoder strips any leading whitespace (see PR #89)
    char *token_str =
        (token == 1 && vocab[next][0] == ' ') ? vocab[next] + 1 : vocab[next];
    output += token_str;
    IC_API::debug_print(output);
    token = next;

    // init the timer here because the first iteration can be slower
    if (start == 0) {
      start = ic_api.time(); // time in ns
    }
  }

  // report achieved tok/s (pos-1 because the timer starts after first iteration)
  if (pos > 1) {
    uint64_t end = ic_api.time(); // time in ns
    *token_per_sec = (pos - 1) / (double)(end - start) * 1e9;
  }

  // memory and file handles cleanup
  free_run_state(&state);
  if (prompt_tokens != NULL) free(prompt_tokens);

  IC_API::debug_print(output);
  return output;
}

// Based on a given prompt, llama2 will generate a token string
void inference() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  IC_API::debug_print("ready_for_inference =" +
                      std::to_string(ready_for_inference));
  if (!ready_for_inference) {
    ic_api.to_wire(CandidTypeVariant{
        "err", CandidTypeText{
                   "The Llama2 canister is not (yet) ready for inference."}});
    return;
  }

  // Get the Prompt from the wire
  Prompt prompt;
  CandidTypeRecord r_in;
  r_in.append("prompt", CandidTypeText{&prompt.prompt});
  r_in.append("steps", CandidTypeNat64{&prompt.steps});
  r_in.append("temperature", CandidTypeFloat32{&prompt.temperature});
  r_in.append("topp", CandidTypeFloat32{&prompt.topp});
  ic_api.from_wire(r_in);
  print_prompt(prompt);

  // Generate the response
  RunState state;
  uint64_t token_per_sec;
  std::string sampling_method;
  std::string output =
      generate(ic_api, prompt, &token_per_sec, &sampling_method);
  IC_API::debug_print(output);
  IC_API::debug_print("Achieved token/second: " +
                      std::to_string(token_per_sec));
  IC_API::debug_print("Sampling method used: " + sampling_method);

  // Return the generated response
  ic_api.to_wire(CandidTypeVariant{"ok", CandidTypeText{output}});
}
