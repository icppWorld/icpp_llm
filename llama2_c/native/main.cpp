// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#define get_current_dir _getcwd
#else
#include <unistd.h>
#define get_current_dir getcwd
#endif

#include "../src/canister.h"
#include "../src/chats.h"
#include "../src/http.h"
#include "../src/inference.h"
#include "../src/initialize.h"
#include "../src/nft_collection.h"
#include "../src/upload.h"
#include "../src/users.h"

// The Mock IC
#include "mock_ic.h"

// Helper function to chunk a vector into x Mb
std::vector<std::pair<size_t, size_t>>
chunk_file(const std::vector<uint8_t> &model_bytes, const float x_chunk = 1.0) {
  const size_t chunk_size = std::round(x_chunk * 1024 * 1024); // x_chunk & 1 MB
  std::vector<std::pair<size_t, size_t>> chunk_indices;

  for (size_t i = 0; i < model_bytes.size(); i += chunk_size) {
    size_t end = i + chunk_size;

    // If end goes beyond the size of model_bytes, reduce it to the size of model_bytes
    if (end > model_bytes.size()) end = model_bytes.size();

    chunk_indices.push_back(std::make_pair(i, end));
  }

  return chunk_indices;
}

int main() {
  MockIC mockIC(true);

  // Placeholder strings for the raw candid messages going in and out
  std::string candid_in{""};           // what the client will send
  std::string candid_out_expected{""}; // what the canister must send back

  // -----------------------------------------------------------------------------
  // Configs for the tests:

  // The pretend principals used for the tests
  // Owner of canister
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  // Whitelisted principals for NFT minting (user or canister principal id)
  std::string nft_whitelist_principal_id_user{
      "nfxu4-cn7qt-x7r3c-5dhnk-dcrct-gmgoz-67gcg-5glvc-2krhv-gcmsr-qqe"};
  std::string nft_whitelist_principal_id_canister{
      "bd3sg-teaaa-aaaaa-qaaba-cai"};
  // Authenticated user
  std::string your_principal{"2ibo7-dia"};
  // Anonymous
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  // The model & tokenizer to use
  int model_to_use = 1; // 1=260K, 2=15M, 3=42M, 4=110M (TinyStories)

  std::string model_path;
  std::string tokenizer_path;
  if (model_to_use == 1) {
    // Use this really small model during development
    model_path = "stories260K/stories260K.bin";
    tokenizer_path = "stories260K/tok512.bin";
  } else if (model_to_use == 2) {
    // Use this during final QA
    model_path = "models/stories15Mtok4096.bin";
    tokenizer_path = "tokenizers/tokenizer.bin";
  } else if (model_to_use == 3) {
    model_path = "models/stories42M.bin";
    tokenizer_path = "tokenizers/tokenizer.bin";
  } else if (model_to_use == 4) {
    model_path = "models/stories110M.bin";
    tokenizer_path = "tokenizers/tokenizer.bin";
  } else {
    std::cout << "ERROR: Uknown value of 'model_to_use'";
    exit(1);
  }
  std::cout << "model_path = " << model_path << "\n";
  std::cout << "tokenizer_path = " << tokenizer_path << "\n";

  // -----------------------------------------------------------------------------
  // Read the models file into a bytes vector

  std::streamsize file_size;        // file size bytes
  std::vector<uint8_t> model_bytes; // bytes to upload
  {
    // Open the file in binary mode. 'ate' means open at end
    std::ifstream file(model_path, std::ios::binary | std::ios::ate);
    if (!file) {
      std::cout << "Couldn't open file " << model_path << '\n';
      char cwd[FILENAME_MAX];
      if (!get_current_dir(cwd, sizeof(cwd))) {
        std::cout << "Couldn't determine current_dir " << '\n';
        return errno;
      }
      std::cout << "The current working directory is " << std::string(cwd)
                << '\n';
      return 1;
    }

    // Figure out the total size of the file in bytes
    file_size = file.tellg();

    // Seek back to the start
    file.seekg(0, std::ios::beg);

    // Resize the bytes vector to the correct size
    model_bytes.resize(file_size);

    // Read the file into the vector
    if (!file.read(reinterpret_cast<char *>(model_bytes.data()), file_size)) {
      std::cout << "File read failed!\n";
      return 1;
    }
  }

  // -----------------------------------------------------------------------------
  // Read the tokenizer file into a bytes vector

  // std::streamsize file_size;       // file size bytes
  std::vector<uint8_t> tokenizer_bytes; // bytes to upload
  {
    // Open the file in binary mode. 'ate' means open at end
    std::ifstream file(tokenizer_path, std::ios::binary | std::ios::ate);
    if (!file) {
      std::cout << "Couldn't open file " << tokenizer_path << '\n';
      char cwd[FILENAME_MAX];
      if (!get_current_dir(cwd, sizeof(cwd))) {
        std::cout << "Couldn't determine current_dir " << '\n';
        return errno;
      }
      std::cout << "The current working directory is " << std::string(cwd)
                << '\n';
      return 1;
    }

    // Figure out the total size of the file in bytes
    file_size = file.tellg();

    // Seek back to the start
    file.seekg(0, std::ios::beg);

    // Resize the bytes vector to the correct size
    tokenizer_bytes.resize(file_size);

    // Read the file into the vector
    if (!file.read(reinterpret_cast<char *>(tokenizer_bytes.data()),
                   file_size)) {
      std::cout << "File read failed!\n";
      return 1;
    }
  }

  // -----------------------------------------------------------------------------
  // During deployment to IC, this is called automatically. It allocates OP memory
  // '()' -> canister_init does not return directly, so skip validation check
  mockIC.run_test("canister_init", canister_init, "4449444c0000", "",
                  silent_on_trap, my_principal);

  // ####################################
  // # canister_mode = 'chat-principal' #
  // ####################################

  // '("wrong-mode": text)'
  // -> '(variant { Err = "ERROR: unknown canister_mode = wrong-mode" : text})'
  mockIC.run_test(
      "set_canister_mode to wrong-mode", set_canister_mode,
      "4449444c0001710a77726f6e672d6d6f6465",
      "4449444c026b01b0ad8fcd0c716b01c5fed2010001010000294552524f523a20756e6b6e6f776e2063616e69737465725f6d6f6465203d2077726f6e672d6d6f6465",
      silent_on_trap, my_principal);

  // '("chat-principal": text)'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("set_canister_mode to chat-principal", set_canister_mode,
                  "4449444c0001710e636861742d7072696e636970616c",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------
  // The canister health & readiness checks
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("health", health, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, anonymous_principal);

  // '()' -> '(variant { Err = record { Other = "Model not yet uploaded or initialize endpoint not yet called"} })'
  mockIC.run_test(
      "ready", ready, "4449444c0000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100003c4d6f64656c206e6f74207965742075706c6f61646564206f7220696e697469616c697a6520656e64706f696e74206e6f74207965742063616c6c6564",
      silent_on_trap, anonymous_principal);

  // -----------------------------------------------------------------------------

  // '()' -> '("expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")'
  mockIC.run_test(
      "whoami", whoami, "4449444c0000",
      "4449444c0001713f6578706d742d67747873772d696e66746a2d747461626a2d71687035732d6e6f7a75702d6e3362626f2d6b377a766e2d64673468652d6b6e6163332d6c6165",
      silent_on_trap, my_principal);

  // '()' -> '(variant { Err = record { Other = "Access Denied"} })'
  // Call with non owner principal must be denied
  mockIC.run_test(
      "reset_model Err", reset_model, "4449444c0000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, anonymous_principal);

  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  // Call with owner principal
  mockIC.run_test("reset_model", reset_model, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // ==========================================================================
  std::cout << "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "Sending bytes of " << model_path << "\n";
  float x_chunk = 25; // To speed up native test. IC can handle only < 2.0 Mb
  auto chunk_indices = chunk_file(model_bytes, x_chunk);

  // Iterate over all chunks
  int i = 0;
  int count_bytes = 0;
  for (auto &indices : chunk_indices) {
    auto chunk_start = indices.first;
    auto chunk_end = indices.second;

    // Construct a temporary vector for the chunk
    std::vector<uint8_t> chunk(model_bytes.begin() + chunk_start,
                               model_bytes.begin() + chunk_end);

    count_bytes += chunk.size();

    std::cout << "Sending candid for " << std::to_string(chunk.size())
              << " bytes : \n";
    std::cout << "- i            = " << std::to_string(i) << "\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "- progress = "
              << (static_cast<double>(count_bytes) / model_bytes.size()) * 100
              << " % " << std::endl;

    std::cout << "- chunk[0]     = " << std::to_string(chunk[0]) << "\n";
    std::cout << "- chunk.back() = " << std::to_string(chunk.back()) << "\n";
    std::cout << "Serializing... \n";
    candid_in = CandidSerialize(CandidTypeVecNat8{chunk}).as_hex_string();
    std::cout << "- candid_raw   = ";
    if (candid_in.size() <= 20) std::cout << candid_in << "\n";
    else
      std::cout << candid_in.substr(0, 10) << "..."
                << candid_in.substr(candid_in.size() - 10) << "\n";

    // candid_in -> '(variant { Ok = record { status_code = 200 : nat16} })'
    candid_out_expected = "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800";
    mockIC.run_test("upload_model_bytes_chunk", upload_model_bytes_chunk,
                    candid_in, candid_out_expected, silent_on_trap,
                    my_principal);

    i++;
  }

  // ==========================================================================
  std::cout << "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "Sending bytes of " << tokenizer_path << "\n";
  x_chunk = 1.9;
  chunk_indices = chunk_file(tokenizer_bytes, x_chunk);

  // Iterate over all chunks
  i = 0;
  count_bytes = 0;
  for (auto &indices : chunk_indices) {
    auto chunk_start = indices.first;
    auto chunk_end = indices.second;

    // Construct a temporary vector for the chunk
    std::vector<uint8_t> chunk(tokenizer_bytes.begin() + chunk_start,
                               tokenizer_bytes.begin() + chunk_end);
    count_bytes += chunk.size();

    std::cout << "Sending candid for " << std::to_string(chunk.size())
              << " bytes : \n";
    std::cout << "- i            = " << std::to_string(i) << "\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "- progress = "
              << (static_cast<double>(count_bytes) / tokenizer_bytes.size()) *
                     100
              << " % " << std::endl;

    std::cout << "- chunk[0]     = " << std::to_string(chunk[0]) << "\n";
    std::cout << "- chunk.back() = " << std::to_string(chunk.back()) << "\n";

    std::cout << "Serializing... \n";
    candid_in = CandidSerialize(CandidTypeVecNat8{chunk}).as_hex_string();
    std::cout << "- candid_raw   = ";
    if (candid_in.size() <= 20) std::cout << candid_in << "\n";
    else
      std::cout << candid_in.substr(0, 10) << "..."
                << candid_in.substr(candid_in.size() - 10) << "\n";

    // candid_in -> '(variant { Ok = record { status_code = 200 : nat16} })'
    candid_out_expected = "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800";
    mockIC.run_test("upload_tokenizer_bytes_chunk",
                    upload_tokenizer_bytes_chunk, candid_in,
                    candid_out_expected, silent_on_trap, my_principal);

    i++;
  }

  // ==========================================================================
  // Mimic two principals at once are building a story...

  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("initialize", initialize, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, your_principal);

  // '()' -> '(variant { Err =  "The Llama2 canister does not allow calling with anonymous principal." : text })'
  mockIC.run_test(
      "new_chat", new_chat, "4449444c0000",
      "4449444c016b01c5fed2017101000044546865204c6c616d61322063616e697374657220646f6573206e6f7420616c6c6f772063616c6c696e67207769746820616e6f6e796d6f7573207072696e636970616c2e",
      silent_on_trap, anonymous_principal);

  std::string expected_response = "-to-do-";
  if (model_to_use == 1) {
    /*
  '()' -> 
  '(
    record {
      dim = 64 : int;
      hidden_dim = 4 : int;
      n_layers = 512 : int;
      n_heads = 172 : int;
      n_kv_heads = 5 : int;
      vocab_size = 512 : int;
      seq_len = 8 : int;
    },
  )'
  */
    expected_response =
        "4449444c016c07c8fab0027cb087c0d9017cd58488bc027cb3fdc984037cf3e0d4d6057cf5cfd3fc057c82c3e4f60f7c0100c000048004ac0105800408";
  } else if (model_to_use == 2) {
    /*
  '()' -> 
  '(
    record {
      dim = 288 : int;
      hidden_dim = 768 : int;
      n_layers = 6 : int;
      n_heads = 6 : int;
      n_kv_heads = 6 : int;
      vocab_size = 32_000 : int;
      seq_len = 256 : int;
    },
  )'
  */
    expected_response =
        "4449444c016c07c8fab0027cb087c0d9017cd58488bc027cb3fdc984037cf3e0d4d6057cf5cfd3fc057c82c3e4f60f7c0100a0020680fa01800606800206";
  } else if (model_to_use == 3) {
    /*
  '()' -> 
  '(
    record {
      dim = 512 : int;
      hidden_dim = 1_376 : int;
      n_layers = 8 : int;
      n_heads = 8 : int;
      n_kv_heads = 8 : int;
      vocab_size = 32_000 : int;
      seq_len = 1024 : int;
    },
  )'
  */
    expected_response =
        "4449444c016c07c8fab0027cb087c0d9017cd58488bc027cb3fdc984037cf3e0d4d6057cf5cfd3fc057c82c3e4f60f7c010080040880fa01e00a08800808";
  } else if (model_to_use == 4) {
    /*
  '()' -> 
  '(
    record {
      dim = 768 : int;
      hidden_dim = 2_048 : int;
      n_layers = 12 : int;
      n_heads = 12 : int;
      n_kv_heads = 12 : int;
      vocab_size = 32_000 : int;
      seq_len = 1024 : int;
    },
  )'
  */
    expected_response =
        "4449444c016c07c8fab0027cb087c0d9017cd58488bc027cb3fdc984037cf3e0d4d6057cf5cfd3fc057c82c3e4f60f7c010080060c80fa0180100c80080c";
  }
  mockIC.run_test("get_model_config", get_model_config, "4449444c0000",
                  expected_response, silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  std::array<std::string, 2> principals = {my_principal, your_principal};

  // Repeat it two times, for two principals intermittently
  for (int k = 0; k < 2; k++) {
    for (const auto &principal : principals) {

      // A new chat, using a prompt built in multiple steps
      // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
      mockIC.run_test("new_chat", new_chat, "4449444c0000",
                      "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                      silent_on_trap, principal);
    }

    // Loop to create a long story, 10 tokens at a time
    // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
    std::array<std::string, 2> prompt = {"Lilly went to", "Billy came from"};
    std::array<uint64_t, 2> steps = {0, 0};
    std::array<float, 2> temperature = {0.0, 0.0};
    std::array<float, 2> topp = {0.9, 0.9};
    std::array<uint64_t, 2> rng_seed = {0, 0};

    std::array<std::string, 2> generated_tokens = {"", ""};
    std::array<uint64_t, 2> num_tokens = {0, 0};
    std::array<std::string, 2> story = {"", ""};

    for (int i = 0; i < 10; i++) {
      for (int j = 0; j < 2; j++) {

        CandidTypeRecord r_in;
        r_in.append("prompt", CandidTypeText(prompt[j]));
        r_in.append("steps", CandidTypeNat64(steps[j]));
        r_in.append("temperature", CandidTypeFloat32(temperature[j]));
        r_in.append("topp", CandidTypeFloat32(topp[j]));
        r_in.append("rng_seed", CandidTypeNat64(uint64_t(rng_seed[j])));
        candid_in = CandidSerialize(r_in).as_hex_string();

        std::string candid_out;
        mockIC.run_test("inference 0a", inference, candid_in, "",
                        silent_on_trap, principals[j], &candid_out);

        CandidTypeRecord inference_record;
        inference_record.append("inference",
                                CandidTypeText{&generated_tokens[j]});
        inference_record.append("num_tokens", CandidTypeNat64{&num_tokens[j]});
        std::string err_text;
        CandidTypeVariant v_out;
        v_out.append("Ok", inference_record);
        v_out.append("Err",
                     CandidTypeVariant{"Other", CandidTypeText(&err_text)});

        CandidArgs A;
        A.append(v_out);
        CandidDeserialize(candid_out, A);
        if (err_text.size() > 0) {
          std::cout << "Err returned by inference function:\n"
                    << err_text << "\n";
          exit(1);
        }
        story[j] += generated_tokens[j];
        std::cout << story[j] << "\n";
      }
      if (i == 0) {
        prompt[0] = "the beach this morning. ";
        prompt[1] = "the pool this afternoon. ";
        steps[0] = 0;
        steps[1] = 0;
      } else if (i == 1) {
        prompt[0] = "She saw a little boat";
        prompt[1] = "He went swimming";
        steps[0] = 0;
        steps[1] = 0;
      } else if (i == 2) {
        prompt[0] = " with her friend Billy";
        prompt[1] = " with his friend Lilly";
        steps[0] = 0;
        steps[1] = 0;
      } else {
        prompt[0] = "";
        prompt[1] = "";
        steps[0] = 10;
        steps[1] = 10;
      }
    }
  }

  // -----------------------------------------------------------------------------------------
  { // A new chat, starting with an empty prompt
    // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
    mockIC.run_test("new_chat", new_chat, "4449444c0000",
                    "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                    silent_on_trap, your_principal);

    // Loop to create 1000 token long story, 10 tokens at a time
    // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
    std::string prompt = "";
    uint64_t steps = 10;
    float temperature = 0.0;
    float topp = 0.9;
    uint64_t rng_seed = 0;

    std::string generated_tokens = "";
    uint64_t num_tokens = 0;
    std::string story = "";
    for (int i = 0; i < 100; i++) {
      CandidTypeRecord r_in;
      r_in.append("prompt", CandidTypeText(prompt));
      r_in.append("steps", CandidTypeNat64(steps));
      r_in.append("temperature", CandidTypeFloat32(temperature));
      r_in.append("topp", CandidTypeFloat32(topp));
      r_in.append("rng_seed", CandidTypeNat64(uint64_t(rng_seed)));
      candid_in = CandidSerialize(r_in).as_hex_string();

      std::string candid_out;
      mockIC.run_test("inference 0b", inference, candid_in, "", silent_on_trap,
                      your_principal, &candid_out);

      CandidTypeRecord inference_record;
      inference_record.append("inference", CandidTypeText{&generated_tokens});
      inference_record.append("num_tokens", CandidTypeNat64{&num_tokens});
      std::string err_text;
      CandidTypeVariant v_out;
      v_out.append("Ok", inference_record);
      v_out.append("Err",
                   CandidTypeVariant{"Other", CandidTypeText(&err_text)});

      // std::string err_text;
      // CandidTypeVariant v_out;
      // v_out.append("Ok", CandidTypeText(&generated_tokens));
      // v_out.append("Err", CandidTypeText(&err_text));

      CandidArgs A;
      A.append(v_out);
      CandidDeserialize(candid_out, A);
      if (err_text.size() > 0) {
        std::cout << "ERROR returned by inference function.";
        exit(1);
      }
      story += generated_tokens;

      prompt = "";
    }
    std::cout << story;
  }

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> '(variant { Ok = record { inference = "...story..." : text; num_tokens = 100 } })'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    expected_response =
        "4449444c026c02f3feb4990678d9b3b9980f716b01bc8a01000101006400000000000000fd014f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e20746865207061726b2e204f6e65206461792c20736865207361772061206269672c207265642062616c6c2e205368652077616e74656420746f20706c617920776974682069742c206275742069742077617320746f6f20686967682e0a4c696c792773206d6f6d20736169642c20224c696c792c206c6574277320676f20746f20746865207061726b2e22204c696c79207761732073616420616e64206469646e2774206b6e6f772077";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  mockIC.run_test(
      "inference 1", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000000000803f6400000000000000000000000000000000",
      expected_response, silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat, pretend it being called from Motoko, using float64
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature=0.0 & topp=0.9, still greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.0 : float64; topp = 0.9 : float64; rng_seed = 0 : nat64;})'
  mockIC.run_test(
      "inference_mo 2", inference_mo,
      "4449444c016c05b4e8c2e40372bbb885e80472a7f7b9a00878c5c8cea60878a4a3e1aa0b7101000000000000000000cdccccccccccec3f6400000000000000000000000000000000",
      expected_response, silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature>0.0 & topp=1.0: regular sampling
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 3", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f0000803f6400000000000000000000000000000000",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature>0.0 & topp<1.0: top-pp (nucleus) sampling
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 4", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f6666663f6400000000000000000000000000000000",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> '(variant { Ok = "..." : text })'
  mockIC.run_test(
      "inference 1a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000000000803f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature=0.0 & topp=0.9, still greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  // -> '(variant { Ok = "..." : text })'
  mockIC.run_test(
      "inference 2a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000006666663f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature>0.0 & topp=1.0: regular sampling
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 3a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f0000803f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // With temperature>0.0 & topp<1.0: top-pp (nucleus) sampling
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 4a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f6666663f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // #################################
  // # canister_mode = 'nft-ordinal' #
  // #################################

  // '("nft-ordinal": text)'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("set_canister_mode to nft-ordinal", set_canister_mode,
                  "4449444c0001710b6e66742d6f7264696e616c",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------
  // Whitelist principals that can mint NFTs

  // '(record { id = principal "nfxu4-cn7qt-x7r3c-5dhnk-dcrct-gmgoz-67gcg-5glvc-2krhv-gcmsr-qqe"; description = "A whitelisted user"; })'
  // -> '(variant { Err = record { Other = "Access Denied"} })'
  // Call with non owner principal must be denied
  mockIC.run_test(
      "nft_whitelist a user as non-owner", nft_whitelist,
      "4449444c016c02dbb70168fc91f4f805710100011dbf84eff8ec5d19daa18a2299986767df308dd32ea2d2a27a984c94610212412077686974656c69737465642075736572",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, your_principal);

  // '(record { id = principal "nfxu4-cn7qt-x7r3c-5dhnk-dcrct-gmgoz-67gcg-5glvc-2krhv-gcmsr-qqe"; description = "A whitelisted user"; })'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test(
      "nft_whitelist a user", nft_whitelist,
      "4449444c016c02dbb70168fc91f4f805710100011dbf84eff8ec5d19daa18a2299986767df308dd32ea2d2a27a984c94610212412077686974656c69737465642075736572",
      "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800", silent_on_trap,
      my_principal);

  // '(record { id = principal "bd3sg-teaaa-aaaaa-qaaba-cai"; description = "A whitelisted canister"; })'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test(
      "nft_whitelist a canister", nft_whitelist,
      "4449444c016c02dbb70168fc91f4f805710100010a8000000000100002010116412077686974656c69737465642063616e6973746572",
      "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800", silent_on_trap,
      my_principal);

  // '()'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("nft_ami_whitelisted Ok", nft_ami_whitelisted, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, nft_whitelist_principal_id_canister);

  // '()'
  // -> '(variant { Err = record { Other = "Access Denied"} })'
  mockIC.run_test(
      "nft_ami_whitelisted Err", nft_ami_whitelisted, "4449444c0000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, your_principal);

  // -----------------------------------------------------------------------------
  // Initialize the NFT Collection

  // '(record { nft_supply_cap = 2 : nat64; nft_total_supply = 0 : nat64; nft_symbol = "CHARLES"; nft_name = "Sir Charles The 3rd"; nft_description = "A Bitcoin Ordinal Collection powered by a C++ LLM running on the Internet Computer"})'
  // -> '(variant { Err = record { Other = "Access Denied"} })'
  // Call with non owner principal must be denied
  mockIC.run_test(
      "nft_init as non-owner", nft_init,
      "4449444c016c05a5d0bfc60278ee86c3950871bb9effa20f71b9e4b5ac0f71ad91e7ca0f78010002000000000000001353697220436861726c6573205468652033726407434841524c4553524120426974636f696e204f7264696e616c20436f6c6c656374696f6e20706f7765726564206279206120432b2b204c4c4d2072756e6e696e67206f6e2074686520496e7465726e657420436f6d70757465720000000000000000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, your_principal);

  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test(
      "nft_init", nft_init,
      "4449444c016c05a5d0bfc60278ee86c3950871bb9effa20f71b9e4b5ac0f71ad91e7ca0f78010002000000000000001353697220436861726c6573205468652033726407434841524c4553524120426974636f696e204f7264696e616c20436f6c6c656374696f6e20706f7765726564206279206120432b2b204c4c4d2072756e6e696e67206f6e2074686520496e7465726e657420436f6d70757465720000000000000000",
      "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800", silent_on_trap,
      my_principal);

  // -> '(variant { Err = record { Other = "NFT Collection is already initialized"} })'
  // Re-initialization must be denied
  mockIC.run_test(
      "nft_init again", nft_init,
      "4449444c016c05a5d0bfc60278ee86c3950871bb9effa20f71b9e4b5ac0f71ad91e7ca0f78010002000000000000001353697220436861726c6573205468652033726407434841524c4553524120426974636f696e204f7264696e616c20436f6c6c656374696f6e20706f7765726564206279206120432b2b204c4c4d2072756e6e696e67206f6e2074686520496e7465726e657420436f6d70757465720000000000000000",
      "4449444c026b01b0ad8fcd0c716b01c5fed2010001010000254e465420436f6c6c656374696f6e20697320616c726561647920696e697469616c697a6564",
      silent_on_trap, my_principal);

  // '()' -> '(variant { Ok = record { nft_supply_cap = 2 : nat64; nft_total_supply = 0 : nat64; nft_symbol = "CHARLES"; nft_name = "Sir Charles The 3rd"; nft_description = "A Bitcoin Ordinal Collection powered by a C++ LLM running on the Internet Computer"}})'
  mockIC.run_test(
      "nft_metadata", nft_metadata, "4449444c0000",
      "4449444c026c05a5d0bfc60278ee86c3950871bb9effa20f71b9e4b5ac0f71ad91e7ca0f786b01bc8a010001010002000000000000001353697220436861726c6573205468652033726407434841524c4553524120426974636f696e204f7264696e616c20436f6c6c656374696f6e20706f7765726564206279206120432b2b204c4c4d2072756e6e696e67206f6e2074686520496e7465726e657420436f6d70757465720000000000000000",
      silent_on_trap, anonymous_principal);

  // ------------------------------------------------------------------------
  // Mint some NFTs, which simply sets their bitcoin ordinal
  // Note: There is no story yet

  // Ownership is given to a bitcoin ordinal id
  // A Bitcoin Ordinal ID is a numerical value that uniquely identifies each
  // satoshi, the smallest unit of Bitcoin. Since the total supply of Bitcoin
  // is capped at 21 million BTC, and each Bitcoin is divisible into 100 million
  // satoshis, the highest ordinal number possible would be
  // 2,100,000,000,000,000 (2.1 quadrillion).
  // We will use this in all our tests, ensuring that it will fit.

  // Verify it traps when not owner of the canister
  // A regular user cannot mint NFTs
  // '(record {token_id = "token-A" : text})'
  // -> '(variant { Err = record { Other = "Access Denied"} })'
  mockIC.run_test(
      "nft_mint Err test 1", nft_mint,
      "4449444c016c01a1a1c1da0271010007746f6b656e2d41",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, your_principal);

  // A whitelisted principal can NOT mint NFTs, they can only create new stories
  mockIC.run_test(
      "nft_mint Err test 1", nft_mint,
      "4449444c016c01a1a1c1da0271010007746f6b656e2d41",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, nft_whitelist_principal_id_user);

  // -----------
  // Mint one
  // '(record {token_id = "token-A" : text})'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("nft_mint", nft_mint,
                  "4449444c016c01a1a1c1da0271010007746f6b656e2d41",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // Summarize NFT collection metadata
  // '()' -> '(variant {Ok = record { nft_supply_cap = 2 : nat64; nft_total_supply = 1 : nat64; nft_symbol = "CHARLES"; nft_name = "Sir Charles The 3rd"; nft_description = "A Bitcoin Ordinal Collection powered by a C++ LLM running on the Internet Computer"}})'
  mockIC.run_test(
      "nft_metadata - total_supply = 1", nft_metadata, "4449444c0000",
      "4449444c026c05a5d0bfc60278ee86c3950871bb9effa20f71b9e4b5ac0f71ad91e7ca0f786b01bc8a010001010002000000000000001353697220436861726c6573205468652033726407434841524c4553524120426974636f696e204f7264696e616c20436f6c6c656374696f6e20706f7765726564206279206120432b2b204c4c4d2072756e6e696e67206f6e2074686520496e7465726e657420436f6d70757465720100000000000000",
      silent_on_trap, anonymous_principal);

  // -----------
  // Minting it again for the same ordinal is Ok.
  // '(record {token_id = "token-A" : text})'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("nft_mint only one per ordinal", nft_mint,
                  "4449444c016c01a1a1c1da0271010007746f6b656e2d41",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // -----------
  // Mint another NFT with different token_id
  // '(record {token_id = "token-B" : text})'
  // -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("nft_mint again", nft_mint,
                  "4449444c016c01a1a1c1da0271010007746f6b656e2d42",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);
  // Summarize NFT collection metadata
  // '()' -> '(variant = {Ok = record { nft_supply_cap = 2 : nat64; nft_total_supply = 2 : nat64; nft_symbol = "CHARLES"; nft_name = "Sir Charles The 3rd"; nft_description = "A Bitcoin Ordinal Collection powered by a C++ LLM running on the Internet Computer"}})'
  mockIC.run_test(
      "nft_metadata - total_supply = 2", nft_metadata, "4449444c0000",
      "4449444c026c05a5d0bfc60278ee86c3950871bb9effa20f71b9e4b5ac0f71ad91e7ca0f786b01bc8a010001010002000000000000001353697220436861726c6573205468652033726407434841524c4553524120426974636f696e204f7264696e616c20436f6c6c656374696f6e20706f7765726564206279206120432b2b204c4c4d2072756e6e696e67206f6e2074686520496e7465726e657420436f6d70757465720200000000000000",
      silent_on_trap, anonymous_principal);

  // -----------
  // Minting another NFT for another ordinal must Err, because we reached supply_cap
  // '(record {token_id = "token-C" : text})'
  // -> '(variant { Err = record { Other = "Cannot mint, reached supply_cap of 2"} })'
  mockIC.run_test(
      "nft_mint hit supply_cap", nft_mint,
      "4449444c016c01a1a1c1da0271010007746f6b656e2d43",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100002443616e6e6f74206d696e742c207265616368656420737570706c795f636170206f662032",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // calling nft_get_story on non-existing token_id must fail

  // '(record {token_id = "token-ERR" : text})'
  // -> '(variant { Err = variant { Other = "NFT token-ERR does not exists." : text} })'
  mockIC.run_test(
      "nft_get_story Err 0", nft_get_story,
      "4449444c016c01a1a1c1da0271010009746f6b656e2d455252",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100001e4e465420746f6b656e2d45525220646f6573206e6f74206578697374732e",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // calling nft_get_story on existing token_id without a story must fail

  // '(record {token_id = "token-A" : text})'
  // -> '(variant { Err = variant { Other = "The story for NFT token-A does not exists." : text} })'
  mockIC.run_test(
      "nft_get_story Err 1", nft_get_story,
      "4449444c016c01a1a1c1da0271010007746f6b656e2d41",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100002a5468652073746f727920666f72204e465420746f6b656e2d4120646f6573206e6f74206578697374732e",
      silent_on_trap, my_principal);
  // ------------------------------------------------------------------------
  // Create the story for nft_id=0, with token_id="token-A"

  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {token_id = "token-A" : text}, record{ prompt = "It was a bright sunny day and Charles went to the beach with his fishing pole." : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    // -> '(variant { Ok = record { inference = "...some story..." : text;} })'
    expected_response =
        "4449444c026c02f3feb4990678d9b3b9980f716b01bc8a010001010026000000000000004e4974207761732061206272696768742073756e6e792064617920616e6420436861726c65732077656e7420746f207468652062656163682077697468206869732066697368696e6720706f6c652e";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  // Verify it returns Err when caller is not whitelisted
  // A regular user cannot create new stories for NFTs
  // -> '(variant { Err = variant { Other = "Access Denied - You are not authorized to call this function." } })'
  mockIC.run_test(
      "nft_story_start Err test", nft_story_start,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d41000000000000803f640000000000000000000000000000004e4974207761732061206272696768742073756e6e792064617920616e6420436861726c65732077656e7420746f207468652062656163682077697468206869732066697368696e6720706f6c652e",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100003d4163636573732044656e696564202d20596f7520617265206e6f7420617574686f72697a656420746f2063616c6c20746869732066756e6374696f6e2e",
      silent_on_trap, your_principal);
  mockIC.run_test(
      "nft_story_start 0", nft_story_start,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d41000000000000803f640000000000000000000000000000004e4974207761732061206272696768742073756e6e792064617920616e6420436861726c65732077656e7420746f207468652062656163682077697468206869732066697368696e6720706f6c652e",
      expected_response, silent_on_trap, my_principal);

  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {token_id = "token-A" : text}, record{ prompt = "" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    // -> '(variant { Ok = record { num_tokens = 100; inference = "...some story..." : text;} })'
    expected_response =
        "4449444c026c02f3feb4990678d9b3b9980f716b01bc8a01000101006400000000000000a601204865207761732076657279206578636974656420746f2073656520776861742077617320696e736964652e204865207761732076657279206578636974656420746f2073656520776861742077617320696e736964652e0a2248656c6c6f2c20436861726c69652122207361696420436861726c69652e0a2249276d20736f7272792c22207361696420436861726c69652e0a2249276d20736f7272792c22207361696420";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  // Verify it returns Err when caller is not whitelisted
  // A regular user cannot continue stories for NFTs
  // -> '(variant { Err = variant { Other = "Access Denied - You are not authorized to call this function." } })'
  mockIC.run_test(
      "nft_story_continue Err test", nft_story_continue,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d41000000000000803f6400000000000000000000000000000000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100003d4163636573732044656e696564202d20596f7520617265206e6f7420617574686f72697a656420746f2063616c6c20746869732066756e6374696f6e2e",
      silent_on_trap, your_principal);
  mockIC.run_test(
      "nft_story_continue 0", nft_story_continue,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d41000000000000803f6400000000000000000000000000000000",
      expected_response, silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Create the story for nft_id=1 with token_id="token-B", being called from Motoko with PromptMo (float64 types)

  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {token_id = "token-B" : text}, record{ prompt = "Charles had a boat." : text; steps = 100 : nat64; temperature = 0.0 : float64; topp = 1.0 : float64; rng_seed = 0 : nat64;})'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    // -> '(variant { Ok = record { num_tokens = 12; inference = "...some story..." : text;} })'
    expected_response =
        "4449444c026c02f3feb4990678d9b3b9980f716b01bc8a01000101000c0000000000000013436861726c657320686164206120626f61742e";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  // Verify it returns Err when caller is not whitelisted
  // A regular user cannot create new stories for NFTs
  // -> '(variant { Err = variant { Other = "Access Denied - You are not authorized to call this function." } })'
  mockIC.run_test(
      "nft_story_start_mo 1 Err test", nft_story_start_mo,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40372bbb885e80472a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d420000000000000000000000000000f03f6400000000000000000000000000000013436861726c657320686164206120626f61742e",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100003d4163636573732044656e696564202d20596f7520617265206e6f7420617574686f72697a656420746f2063616c6c20746869732066756e6374696f6e2e",
      silent_on_trap, your_principal);
  mockIC.run_test(
      "nft_story_start_mo 1", nft_story_start_mo,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40372bbb885e80472a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d420000000000000000000000000000f03f6400000000000000000000000000000013436861726c657320686164206120626f61742e",
      expected_response, silent_on_trap, my_principal);

  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {token_id = "token-B" : text}, record{ prompt = "" : text; steps = 100 : nat64; temperature = 0.0 : float64; topp = 1.0 : float64; rng_seed = 0 : nat64;})'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    // -> '(variant { Ok = record { num_tokens = 100, inference = "...some story..." : text;} })'
    expected_response =
        "4449444c026c02f3feb4990678d9b3b9980f716b01bc8a010001010064000000000000008702204865206c696b656420746f20706c617920776974682068697320746f797320616e642072756e2061726f756e642074686520726f6f6d2e2048652077617320766572792068617070792e2048652077616e74656420746f20706c617920776974682068697320746f79732e0a4f6e65206461792c20436861726c69652073617720612062696720626f61742e2054686520626f6174207761732076657279207363617265642e2048652077616e74656420746f20706c617920776974682074686520626f61742e2048652077616e74656420746f20706c617920776974682074686520626f61742e2048652077616e74656420746f20706c617920776974682074686520626f";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  // Verify it returns Err when caller is not whitelisted
  // A regular user cannot continue stories for NFTs
  // -> '(variant { Err = variant { Other = "Access Denied - You are not authorized to call this function." } })'
  mockIC.run_test(
      "nft_story_continue_mo Err test", nft_story_continue_mo,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40372bbb885e80472a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d420000000000000000000000000000f03f6400000000000000000000000000000000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100003d4163636573732044656e696564202d20596f7520617265206e6f7420617574686f72697a656420746f2063616c6c20746869732066756e6374696f6e2e",
      silent_on_trap, your_principal);
  mockIC.run_test(
      "nft_story_continue_mo 1", nft_story_continue_mo,
      "4449444c026c01a1a1c1da02716c05b4e8c2e40372bbb885e80472a7f7b9a00878c5c8cea60878a4a3e1aa0b7102000107746f6b656e2d420000000000000000000000000000f03f6400000000000000000000000000000000",
      expected_response, silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Get the story for nft_id=0, with token_id="token-A"

  // '(record {token_id = "token-A" : text})'
  // -> '(variant { Ok = record { story = "...story..." : text;} })'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    expected_response =
        "4449444c026c01f5a7d8a008716b01bc8a0100010100f4014974207761732061206272696768742073756e6e792064617920616e6420436861726c65732077656e7420746f207468652062656163682077697468206869732066697368696e6720706f6c652e204865207761732076657279206578636974656420746f2073656520776861742077617320696e736964652e204865207761732076657279206578636974656420746f2073656520776861742077617320696e736964652e0a2248656c6c6f2c20436861726c69652122207361696420436861726c69652e0a2249276d20736f7272792c22207361696420436861726c69652e0a2249276d20736f7272792c22207361696420";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  mockIC.run_test("nft_get_story 0", nft_get_story,
                  "4449444c016c01a1a1c1da0271010007746f6b656e2d41",
                  expected_response, silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Get the story for nft_id=1, with token_id="token-B"

  // '(record {token_id = "token-B" : text})'
  // -> '(variant { Ok = record { story = "...story..." : text;} })'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    expected_response =
        "4449444c026c01f5a7d8a008716b01bc8a01000101009a02436861726c657320686164206120626f61742e204865206c696b656420746f20706c617920776974682068697320746f797320616e642072756e2061726f756e642074686520726f6f6d2e2048652077617320766572792068617070792e2048652077616e74656420746f20706c617920776974682068697320746f79732e0a4f6e65206461792c20436861726c69652073617720612062696720626f61742e2054686520626f6174207761732076657279207363617265642e2048652077616e74656420746f20706c617920776974682074686520626f61742e2048652077616e74656420746f20706c617920776974682074686520626f61742e2048652077616e74656420746f20706c617920776974682074686520626f";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  mockIC.run_test("nft_get_story 1", nft_get_story,
                  "4449444c016c01a1a1c1da0271010007746f6b656e2d42",
                  expected_response, silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Call to the http_request endpoint, to get the story of nft_id 0
  // (
  //   record {                                       // IC_HttpRequest
  //     5_843_823 = "/api/nft/0";                    // url
  //     156_956_385 = "GET";                         // method
  //     1_092_319_906 = vec {};                      // body
  //     1_661_489_734 = vec { record {...} };        // headers
  //     1_661_892_784 = opt (2 : nat16);             // certificate_version
  //   },
  // )
  // I encoded the anonimized version of an actual request to MAINNET:
  // '(record { 5_843_823 = "/api/nft/0"; 156_956_385 = "GET"; 1_092_319_906 = blob "{}"; 1_661_489_734 = vec { record { "host"; "xxxxx-xxxxx-xxxxx-xxxxx-cai.icp0.io";}; record { "x-real-ip"; "xx.xx.xxx.xxx";}; record { "x-forwarded-for"; "xx.xx.xxx.xxx";}; record { "x-forwarded-proto"; "https";}; record { "x-request-id"; "15aad0a6-e422-26e5-8992-c3fe477ffedd";}; record { "x-icx-require-certification"; "1";}; record { "pragma"; "no-cache";}; record { "cache-control"; "no-cache";}; record { "sec-ch-ua"; "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Brave\";v=\"120\"";}; record { "sec-ch-ua-mobile"; "?0";}; record { "user-agent"; "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";}; record { "sec-ch-ua-platform"; "\"Linux\"";}; record { "accept"; "*/*";}; record { "sec-gpc"; "1";}; record { "origin"; "null";}; record { "sec-fetch-site"; "cross-site";}; record { "sec-fetch-mode"; "cors";}; record { "sec-fetch-dest"; "empty";}; record { "accept-encoding"; "gzip, deflate, br, identity";}; record { "accept-language"; "en-US,en;q=0.9";};}; 1_661_892_784 = opt (2 : nat16); }, )'
  //
  // Note: Encoding this gives an error on the empty vec, not sure yet why:
  // '(record { 5_843_823 = "/api/nft/0"; 156_956_385 = "GET"; 1_092_319_906 = vec {}; 1_661_489_734 = vec { record { "host"; "xxxxx-xxxxx-xxxxx-xxxxx-cai.icp0.io";}; record { "x-real-ip"; "xx.xx.xxx.xxx";}; record { "x-forwarded-for"; "xx.xx.xxx.xxx";}; record { "x-forwarded-proto"; "https";}; record { "x-request-id"; "15aad0a6-e422-26e5-8992-c3fe477ffedd";}; record { "x-icx-require-certification"; "1";}; record { "pragma"; "no-cache";}; record { "cache-control"; "no-cache";}; record { "sec-ch-ua"; "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Brave\";v=\"120\"";}; record { "sec-ch-ua-mobile"; "?0";}; record { "user-agent"; "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";}; record { "sec-ch-ua-platform"; "\"Linux\"";}; record { "accept"; "*/*";}; record { "sec-gpc"; "1";}; record { "origin"; "null";}; record { "sec-fetch-site"; "cross-site";}; record { "sec-fetch-mode"; "cors";}; record { "sec-fetch-dest"; "empty";}; record { "accept-encoding"; "gzip, deflate, br, identity";}; record { "accept-language"; "en-US,en;q=0.9";};}; 1_661_892_784 = opt (2 : nat16); }, )'
  //
  // -> Returns an IC_HttpResponse
  // (
  //   record {                                                                     // IC_HttpResponse
  //     1_092_319_906 = blob "{\22story\22:\22Once upon a time, ...\5c\22\22}";    // body
  //     1_661_489_734 = vec {                                                      // headers
  //       record { "Content-Type"; "application/json" };
  //       record { "Content-Length"; "803" };
  //     };
  //     1_664_201_884 = opt false;                                                 // upgrade
  //     3_475_804_314 = 200 : nat16;                                               // status_code
  //   },
  // )
  expected_response = "-to-do-";
  if (model_to_use == 1) {
    expected_response =
        "4449444c0a6c02000101016d716c006c02007101716d036c02007101716c02007101716c04a2f5ed880408c6a4a19806049ce9c69906099aa1b2f90c7a6d7b6e7e0107a9027b226e66745f6964223a302c2273746f7279223a224974207761732061206272696768742073756e6e792064617920616e6420436861726c65732077656e7420746f207468652062656163682077697468206869732066697368696e6720706f6c652e204865207761732076657279206578636974656420746f2073656520776861742077617320696e736964652e204865207761732076657279206578636974656420746f2073656520776861742077617320696e736964652e5c6e5c2248656c6c6f2c20436861726c6965215c22207361696420436861726c69652e5c6e5c2249276d20736f7272792c5c22207361696420436861726c69652e5c6e5c2249276d20736f7272792c5c22207361696420222c22746f6b656e5f6964223a22746f6b656e2d41227d020c436f6e74656e742d54797065106170706c69636174696f6e2f6a736f6e0e436f6e74656e742d4c656e677468033239370100c800";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  mockIC.run_test(
      "http_request anonimized with blob body", http_request,
      "4449444c056c05efd6e40271e1edeb4a71a2f5ed880401c6a4a1980602b0f1b99806046d7b6d036c02007101716e7a01000a2f6170692f6e66742f3003474554027b7d1404686f73742378787878782d78787878782d78787878782d78787878782d6361692e696370302e696f09782d7265616c2d69700d78782e78782e7878782e7878780f782d666f727761726465642d666f720d78782e78782e7878782e78787811782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642431356161643061362d653432322d323665352d383939322d6333666534373766666564641b782d6963782d726571756972652d63657274696669636174696f6e013106707261676d61086e6f2d63616368650d63616368652d636f6e74726f6c086e6f2d6361636865097365632d63682d756138224e6f745f41204272616e64223b763d2238222c20224368726f6d69756d223b763d22313230222c20224272617665223b763d2231323022107365632d63682d75612d6d6f62696c65023f300a757365722d6167656e74654d6f7a696c6c612f352e3020285831313b204c696e7578207838365f363429204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3132302e302e302e30205361666172692f3533372e3336127365632d63682d75612d706c6174666f726d07224c696e75782206616363657074032a2f2a077365632d6770630131066f726967696e046e756c6c0e7365632d66657463682d736974650a63726f73732d736974650e7365632d66657463682d6d6f646504636f72730e7365632d66657463682d6465737405656d7074790f6163636570742d656e636f64696e671b677a69702c206465666c6174652c2062722c206964656e746974790f6163636570742d6c616e67756167650e656e2d55532c656e3b713d302e39010200",
      expected_response, silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Call to the http_request endpoint, to get the story of nft_id 1
  // (
  //   record {                                       // IC_HttpRequest
  //     5_843_823 = "/api/nft/1";                    // url
  //     156_956_385 = "GET";                         // method
  //     1_092_319_906 = vec {};                      // body
  //     1_661_489_734 = vec { record {...} };        // headers
  //     1_661_892_784 = opt (2 : nat16);             // certificate_version
  //   },
  // )
  // I encoded the anonimized version of an actual request to MAINNET:
  // '(record { 5_843_823 = "/api/nft/1"; 156_956_385 = "GET"; 1_092_319_906 = blob "{}"; 1_661_489_734 = vec { record { "host"; "xxxxx-xxxxx-xxxxx-xxxxx-cai.icp0.io";}; record { "x-real-ip"; "xx.xx.xxx.xxx";}; record { "x-forwarded-for"; "xx.xx.xxx.xxx";}; record { "x-forwarded-proto"; "https";}; record { "x-request-id"; "15aad0a6-e422-26e5-8992-c3fe477ffedd";}; record { "x-icx-require-certification"; "1";}; record { "pragma"; "no-cache";}; record { "cache-control"; "no-cache";}; record { "sec-ch-ua"; "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Brave\";v=\"120\"";}; record { "sec-ch-ua-mobile"; "?0";}; record { "user-agent"; "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";}; record { "sec-ch-ua-platform"; "\"Linux\"";}; record { "accept"; "*/*";}; record { "sec-gpc"; "1";}; record { "origin"; "null";}; record { "sec-fetch-site"; "cross-site";}; record { "sec-fetch-mode"; "cors";}; record { "sec-fetch-dest"; "empty";}; record { "accept-encoding"; "gzip, deflate, br, identity";}; record { "accept-language"; "en-US,en;q=0.9";};}; 1_661_892_784 = opt (2 : nat16); }, )'
  //
  // Note: Encoding this gives an error on the empty vec, not sure yet why:
  // '(record { 5_843_823 = "/api/nft/1"; 156_956_385 = "GET"; 1_092_319_906 = vec {}; 1_661_489_734 = vec { record { "host"; "xxxxx-xxxxx-xxxxx-xxxxx-cai.icp0.io";}; record { "x-real-ip"; "xx.xx.xxx.xxx";}; record { "x-forwarded-for"; "xx.xx.xxx.xxx";}; record { "x-forwarded-proto"; "https";}; record { "x-request-id"; "15aad0a6-e422-26e5-8992-c3fe477ffedd";}; record { "x-icx-require-certification"; "1";}; record { "pragma"; "no-cache";}; record { "cache-control"; "no-cache";}; record { "sec-ch-ua"; "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Brave\";v=\"120\"";}; record { "sec-ch-ua-mobile"; "?0";}; record { "user-agent"; "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";}; record { "sec-ch-ua-platform"; "\"Linux\"";}; record { "accept"; "*/*";}; record { "sec-gpc"; "1";}; record { "origin"; "null";}; record { "sec-fetch-site"; "cross-site";}; record { "sec-fetch-mode"; "cors";}; record { "sec-fetch-dest"; "empty";}; record { "accept-encoding"; "gzip, deflate, br, identity";}; record { "accept-language"; "en-US,en;q=0.9";};}; 1_661_892_784 = opt (2 : nat16); }, )'
  //
  // -> Returns an IC_HttpResponse
  // (
  //   record {                                                                     // IC_HttpResponse
  //     1_092_319_906 = blob "{\22story\22:\22Once upon a time, ...\5c\22\22}";    // body
  //     1_661_489_734 = vec {                                                      // headers
  //       record { "Content-Type"; "application/json" };
  //       record { "Content-Length"; "803" };
  //     };
  //     1_664_201_884 = opt false;                                                 // upgrade
  //     3_475_804_314 = 200 : nat16;                                               // status_code
  //   },
  // )
  expected_response = "-to-do-";
  if (model_to_use == 1) {
    expected_response =
        "4449444c0a6c02000101016d716c006c02007101716d036c02007101716c02007101716c04a2f5ed880408c6a4a19806049ce9c69906099aa1b2f90c7a6d7b6e7e0107c7027b226e66745f6964223a312c2273746f7279223a22436861726c657320686164206120626f61742e204865206c696b656420746f20706c617920776974682068697320746f797320616e642072756e2061726f756e642074686520726f6f6d2e2048652077617320766572792068617070792e2048652077616e74656420746f20706c617920776974682068697320746f79732e5c6e4f6e65206461792c20436861726c69652073617720612062696720626f61742e2054686520626f6174207761732076657279207363617265642e2048652077616e74656420746f20706c617920776974682074686520626f61742e2048652077616e74656420746f20706c617920776974682074686520626f61742e2048652077616e74656420746f20706c617920776974682074686520626f222c22746f6b656e5f6964223a22746f6b656e2d42227d020c436f6e74656e742d54797065106170706c69636174696f6e2f6a736f6e0e436f6e74656e742d4c656e677468033332370100c800";
  } else if (model_to_use == 2) {
  } else if (model_to_use == 3) {
  } else if (model_to_use == 4) {
  }
  mockIC.run_test(
      "http_request nft_id 1 anonimized with blob body", http_request,
      "4449444c056c05efd6e40271e1edeb4a71a2f5ed880401c6a4a1980602b0f1b99806046d7b6d036c02007101716e7a01000a2f6170692f6e66742f3103474554027b7d1404686f73742378787878782d78787878782d78787878782d78787878782d6361692e696370302e696f09782d7265616c2d69700d78782e78782e7878782e7878780f782d666f727761726465642d666f720d78782e78782e7878782e78787811782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642431356161643061362d653432322d323665352d383939322d6333666534373766666564641b782d6963782d726571756972652d63657274696669636174696f6e013106707261676d61086e6f2d63616368650d63616368652d636f6e74726f6c086e6f2d6361636865097365632d63682d756138224e6f745f41204272616e64223b763d2238222c20224368726f6d69756d223b763d22313230222c20224272617665223b763d2231323022107365632d63682d75612d6d6f62696c65023f300a757365722d6167656e74654d6f7a696c6c612f352e3020285831313b204c696e7578207838365f363429204170706c655765624b69742f3533372e333620284b48544d4c2c206c696b65204765636b6f29204368726f6d652f3132302e302e302e30205361666172692f3533372e3336127365632d63682d75612d706c6174666f726d07224c696e75782206616363657074032a2f2a077365632d6770630131066f726967696e046e756c6c0e7365632d66657463682d736974650a63726f73732d736974650e7365632d66657463682d6d6f646504636f72730e7365632d66657463682d6465737405656d7074790f6163636570742d656e636f64696e671b677a69702c206465666c6174652c2062722c206964656e746974790f6163636570742d6c616e67756167650e656e2d55532c656e3b713d302e39010200",
      expected_response, silent_on_trap, my_principal);

  // #########################################################################################
  // -----------------------------------------------------------------------------------------
  // Users data

  // Verify that calls Err when not owner
  // (variant { Err = variant { Other = "Access Denied" } })
  mockIC.run_test(
      "get_users Err test", get_users, "4449444c0000",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, your_principal);
  mockIC.run_test(
      "get_user_metadata Err test", get_user_metadata,
      "4449444c000171093269626f372d646961",
      "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564",
      silent_on_trap, your_principal);

  // '()' -> '(4 : nat64)'
  /*
  '()' 
  -> '( variant {
          Ok = record {
            user_count = 4 : nat64;
            user_ids = vec {
            "token-B";
            "token-A";
            "2ibo7-dia";
            "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae";
            };
          }
        },
      )'
  */
  mockIC.run_test(
      "get_users", get_users, "4449444c0000",
      "4449444c036c029bd1ed017884ba9db801016d716b01bc8a010001020004000000000000000407746f6b656e2d4207746f6b656e2d41093269626f372d6469613f6578706d742d67747873772d696e66746a2d747461626a2d71687035732d6e6f7a75702d6e3362626f2d6b377a766e2d64673468652d6b6e6163332d6c6165",
      silent_on_trap, my_principal);

  // '("2ibo7-dia")' -> a UserMetadataRecord... two vectors of nat64 ... but we can not check time-stamp!
  mockIC.run_test("get_user_metadata", get_user_metadata,
                  "4449444c000171093269626f372d646961", "", silent_on_trap,
                  my_principal);

  // -----------------------------------------------------------------------------------------
  // Reset the model
  // '()' -> '(variant { Ok = record { status_code = 200 : nat16} })'
  mockIC.run_test("reset_model", reset_model, "4449444c0000",
                  "4449444c026c019aa1b2f90c7a6b01bc8a0100010100c800",
                  silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------
  // returns 1 if any tests failed
  return mockIC.test_summary();
}