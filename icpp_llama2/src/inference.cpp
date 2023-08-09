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
  uint64_t steps{0};
  float temperature{0.0};
};

void print_prompt(const Prompt &prompt) {
  std::string msg = std::string(__func__) + "model config:";
  msg += "\nprompt.prompt       = " + prompt.prompt;
  msg += "\nprompt.steps        = " + std::to_string(prompt.steps);
  msg += "\nprompt.temperature  = " + std::to_string(prompt.temperature);
  IC_API::debug_print(msg);
}

// Copied from run.c and modified slightly
std::string generate(IC_API ic_api, const Prompt &prompt_) {
  std::string output;

  float temperature = prompt_.temperature; // e.g. 1.0, or 0.0
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
    prompt_tokens = (int *)malloc(config.seq_len * sizeof(int));
    if (!bpe_encode(prompt, vocab, vocab_scores, config.vocab_size,
                    max_token_length, prompt_tokens, &num_prompt_tokens))
      IC_API::trap("bpe_encode of the prompt failed!");
  }

  // start the main loop
  long start =
      0;    // used to time our code, only initialized after first iteration
  int next; // will store the next token in the sequence
  int token =
      1; // init with token 1 (=BOS), as done in Llama-2 sentencepiece tokenizer
  int pos = 0; // position in the sequence
  while (pos < steps) {

    // forward the transformer to get logits for the next token
    transformer(token, pos, &config, &state, &weights);

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
        next = sample(state.logits, config.vocab_size);
      }
    }

    // following BOS token (1), sentencepiece decoder strips any leading whitespace (see PR #89)
    char *token_str =
        (token == 1 && vocab[next][0] == ' ') ? vocab[next] + 1 : vocab[next];
    output += token_str;
    IC_API::debug_print(output);

    // advance forward
    token = next;
    pos++;
    // init our timer here because the first iteration is slow due to memmap
    // if (start == 0) {
    //   start = time_in_ms();
    // }
  }

  // report achieved tok/s
  //   long end = time_in_ms();
  //   printf("\nachieved tok/s: %f\n", (steps - 1) / (double)(end - start) * 1000);

  // memory and file handles cleanup
  free_run_state(&state);
  for (int i = 0; i < config.vocab_size; i++) {
    free(vocab[i]);
  }
  free(vocab);
  free(vocab_scores);
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
  ic_api.from_wire(r_in);
  print_prompt(prompt);

  // Generate the response
  RunState state;
  std::string output = generate(ic_api, prompt);
  IC_API::debug_print(output);

  // Return the generated response
  ic_api.to_wire(CandidTypeVariant{"ok", CandidTypeText{output}});
}
