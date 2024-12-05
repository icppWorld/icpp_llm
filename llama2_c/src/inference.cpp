// Inference endpoint generating a token string for a given prompt

#include "inference.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

#include "canister.h"
#include "chats.h"
#include "prompt.h"
#include "http.h"
#include "initialize.h"
#include "run.h"
#include "upload.h"

#include "ic_api.h"

// Replacement for run.c safe_printf
std::string safe_stringify(const char *piece) {
  std::string result;

  // piece might be a raw byte token, and we only want to convert printable chars or whitespace
  // because some of the other bytes can be various control codes, backspace, etc.
  if (piece == NULL) {
    return result;
  }

  if (piece[0] == '\0') {
    return result;
  }

  if (piece[1] == '\0') {
    unsigned char byte_val = piece[0];
    if (!(isprint(byte_val) || isspace(byte_val))) {
      return result; // bad byte, don't include it in the result
    }
  }

  result = piece; // Assign the valid string to the result
  return result;
}

// Copied from run.c and modified slightly
std::string generate(IC_API ic_api, Chat *chat, Transformer *transformer,
                     Tokenizer *tokenizer, Sampler *sampler, std::string prompt,
                     int steps, bool *error) {
  // --- DEBUG TEST
  // *error = true;
  // return "Testing return of error=true from 'generate'.";
  //--- DEBUG TEST END
  *error = false;
  std::string output;

  // encode the (string) prompt into tokens sequence
  int num_prompt_tokens = 0;

  // +3 for '\0', ?BOS, ?EOS
  int *prompt_tokens = (int *)malloc((prompt.length() + 3) * sizeof(int));
  if (!prompt_tokens) {
    *error = true;
    return "Failed to allocate memory for prompt_tokens.";
  }
  // We do not pass bos, but next, which is 1 after new_chat, else last token of previous call
  int error_code = 0;
  encode(tokenizer, prompt.c_str(), chat->next, chat->eos, prompt_tokens,
         &num_prompt_tokens, &error_code);
  if (error_code != 0) {
    std::string error_msg;
    if (error_code == 1) {
      error_msg = "cannot encode NULL text in 'encode' function of LLM.";
    } else if (error_code == 2) {
      error_msg =
          "allocation failed of str_buffer in 'encode' function of LLM.";
    } else {
      error_msg = "Unknown error occured in 'encode' function of LLM.";
    }
    *error = true;
    return error_msg;
  }

  // icpp: make sure we do not re-add bos next time, unless reset by new_chat
  chat->bos = 0;
  chat->eos = 0;

  int token = chat->next; // token that was predicted last, or BOS
  int pos = chat->pos;    // position in the total sequence
  int prompt_pos = 0;     // position in the current prompt
  // if (num_prompt_tokens > 0) {
  //   token = prompt_tokens
  //       [prompt_pos]; // kick off with the first token in the prompt
  // }

  // chat->total_steps += num_prompt_tokens + steps;
  // When we have a prompt, we do NOT take additional steps
  if (prompt.length() > 0) {
    steps = 0;
  }
  // chat->total_steps += num_prompt_tokens + steps;
  // // override to ~max length
  // if (chat->total_steps > transformer->config.seq_len)
  //   chat->total_steps = transformer->config.seq_len;

  unsigned long long max_total_steps =
      chat->total_steps + num_prompt_tokens + steps;
  if (max_total_steps > transformer->config.seq_len)
    max_total_steps = transformer->config.seq_len;

  // start the main loop
  chat->inference_steps = 0;
  long start =
      0;    // used to time our code, only initialized after first iteration
  int next; // will store the next token in the sequence
  // icpp: use -1, so the exact prompt will be returned when steps is 0
  //       this is critical when building the prompt in multiple calls
  // while (pos < chat->total_steps - 1) {
  while (pos < max_total_steps - 1) {

    // forward the transformer to get logits for the next token
    float *logits = forward(chat, transformer, token, pos);

    // increase our counts
    chat->inference_steps++;
    chat->total_steps++;

    // advance the state state machine
    if (prompt_pos < num_prompt_tokens - 1) {
      // if we are still processing the input prompt, force the next prompt token
      next = prompt_tokens[prompt_pos + 1];
      prompt_pos++;
    } else {
      // icpp: stop if we have exhausted the prompt_tokens, and caller did not ask for
      //       any additional steps to be generated
      if (steps == 0) {
        break;
      }
      // otherwise sample the next token from the logits
      next = sample(sampler, logits);
    }
    pos++;

    // std::cout << "pos = " << pos << std::endl;

    // data-dependent terminating condition: the BOS (=1) token delimits sequences
    if (next == 1) {
      break;
    }

    // print the token as string, decode it with the Tokenizer object
    char *piece = decode(tokenizer, token, next);
    // safe_printf(piece); // same as printf("%s", piece), but skips "unsafe" bytes
    output += safe_stringify(piece);

    // fflush(stdout);
    token = next;

    // safe state in the chat, used in follow-up calls to the endpoint
    chat->next = next;
    chat->pos = pos;

    // init the timer here because the first iteration can be slower
    // if (start == 0) { start = time_in_ms(); }
  }

  // printf("\n");
  // output += "\n"; // icpp: don't do this... we're continuining in next call

  // report achieved tok/s (pos-1 because the timer starts after first iteration)
  // if (pos > 1) {
  //     long end = time_in_ms();
  //     fprintf(stderr, "achieved tok/s: %f\n", (pos-1) / (double)(end-start)*1000);
  // }

  free(prompt_tokens);

  return output;
}

// Inference endpoint for ICGPT, with story ownership based on principal of caller
void inference() { inference_(false); }
void inference_mo() {
  inference_(true);
} // Use this when calling from Motoko, with float64
void inference_(bool from_motoko) {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (!is_canister_mode_chat_principal()) {
    std::string error_msg =
        "Access Denied: canister_mode is not set to 'principal'.";
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }
  if (!is_ready_and_authorized(ic_api)) return;

  // Get the Prompt from the wire
  PromptMo wire_prompt_motoko; // Motoko does not support float32, uses float64
  Prompt wire_prompt;
  CandidTypeRecord r_in;
  r_in.append("prompt", CandidTypeText{&wire_prompt.prompt});
  r_in.append("steps", CandidTypeNat64{&wire_prompt.steps});
  if (from_motoko) {
    r_in.append("temperature",
                CandidTypeFloat64{&wire_prompt_motoko.temperature});
    r_in.append("topp", CandidTypeFloat64{&wire_prompt_motoko.topp});
  } else {
    r_in.append("temperature", CandidTypeFloat32{&wire_prompt.temperature});
    r_in.append("topp", CandidTypeFloat32{&wire_prompt.topp});
  }
  r_in.append("rng_seed", CandidTypeNat64{&wire_prompt.rng_seed});
  ic_api.from_wire(r_in);

  if (from_motoko) {
    wire_prompt.temperature =
        static_cast<float>(wire_prompt_motoko.temperature);
    wire_prompt.topp = static_cast<float>(wire_prompt_motoko.topp);
  }
  // print_prompt(wire_prompt);

  CandidTypePrincipal caller = ic_api.get_caller();
  std::string principal = caller.get_text();

  if (p_chats && p_chats->umap.find(principal) == p_chats->umap.end()) {
    if (!build_new_chat(principal, ic_api)) return;
  }
  if (!p_chats || !p_chats_output_history) {
    std::string error_msg =
        "ERROR: null pointers that should not be null in function " +
        std::string(__func__);
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{error_msg}}});
    return;
  }

  Chat *chat = &p_chats->umap[principal];
  std::string *output_history = &p_chats_output_history->umap[principal];
  MetadataUser *metadata_user = &p_metadata_users->umap[principal];

  bool error{false};
  std::string output = do_inference(ic_api, wire_prompt, chat, output_history,
                                    metadata_user, &error);

  if (error) {
    ic_api.to_wire(CandidTypeVariant{
        "Err", CandidTypeVariant{"Other", CandidTypeText{output}}});
    return;
  }

  // IC_API::debug_print(output);
  // Send the generated response to the wire
  CandidTypeRecord inference_record;
  inference_record.append("inference", CandidTypeText{output});
  inference_record.append("num_tokens", CandidTypeNat64{chat->inference_steps});
  ic_api.to_wire(CandidTypeVariant{"Ok", CandidTypeRecord{inference_record}});
}

std::string do_inference(IC_API &ic_api, Prompt wire_prompt, Chat *chat,
                         std::string *output_history,
                         MetadataUser *metadata_user, bool *error) {

  // parameter validation/overrides
  if (wire_prompt.rng_seed <= 0)
    wire_prompt.rng_seed = ic_api.time(); // time in ns
  if (wire_prompt.temperature < 0.0) wire_prompt.temperature = 0.0;
  if (wire_prompt.topp < 0.0 || 1.0 < wire_prompt.topp) wire_prompt.topp = 0.9;
  if (wire_prompt.steps < 0) wire_prompt.steps = 0;

  // icpp: if caller provides a prompt , set bos & eos
  // if (wire_prompt.prompt.size() > 0) {
  //   transformer.bos = 1;
  //   transformer.eos = 1;
  // }

  // icpp: We treat 'steps' as additional steps to generate
  //       Do this check inside generate method
  // if (wire_prompt.steps == 0 || wire_prompt.steps > transformer.config.seq_len)
  //   wire_prompt.steps = transformer.config.seq_len; // override to ~max length
  // IC_API::debug_print("--\nAfter parameter validation/overrides.");
  // print_prompt(wire_prompt);

  // build the Sampler
  Sampler sampler;
  build_sampler(&sampler, transformer.config.vocab_size,
                wire_prompt.temperature, wire_prompt.topp,
                wire_prompt.rng_seed);

  // run!
  std::string output;
  // if (mode == "generate") {
  output += generate(ic_api, chat, &transformer, &tokenizer, &sampler,
                     wire_prompt.prompt, wire_prompt.steps, error);
  // } else if (mode =="chat") {
  // chat(&transformer, &tokenizer, &sampler, prompt, system_prompt, steps);
  // } else {
  //   return an error about: "unsupported mode: " + mode)
  // }

  if (!*error) {
    // Update & persist full output using Orthogonal Persistence
    *output_history += output;

    // Now we have the total_steps, stored with the chat
    // And we can update the metadata_user
    if (!metadata_user->metadata_chats.empty()) {
      MetadataChat &metadata_chat = metadata_user->metadata_chats.back();
      metadata_chat.total_steps += chat->total_steps;
    }
  }

  // memory and file handles cleanup
  free_sampler(&sampler);

  return output;
}
