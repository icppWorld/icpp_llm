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
#include "../src/inference.h"
#include "../src/initialize.h"
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

  // The pretend principals of the caller
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  std::string your_principal{"2ibo7-dia"};
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  // The model & tokenizer to use
  int model_to_use = 2; // 1=260K, 2=15M, 3=42M, 4=110M (TinyStories)

  std::string model_path;
  std::string tokenizer_path;
  if (model_to_use == 1) {
    // Use this really small model during development
    model_path = "stories260K/stories260K.bin";
    tokenizer_path = "stories260K/tok512.bin";
  } else if (model_to_use == 2) {
    // Use this during final QA
    model_path = "models/stories15M.bin";
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
  // '()' -> canister_init does not return directly, so skip validation check
  mockIC.run_test("canister_init", canister_init, "4449444c0000", "",
                  silent_on_trap, my_principal);

  // '()' -> '(true)'
  mockIC.run_test("health", health, "4449444c0000", "4449444c00017e01",
                  silent_on_trap, anonymous_principal);

  // '()' -> '(false)'
  mockIC.run_test("ready", ready, "4449444c0000", "4449444c00017e00",
                  silent_on_trap, anonymous_principal);

  // '()' -> '(variant { err = 401 : nat16 })'
  // Call with non owner principal must be denied
  mockIC.run_test("reset_model err", reset_model, "4449444c0000",
                  "4449444c016b01e58eb4027a0100009101", silent_on_trap,
                  anonymous_principal);

  // '()' -> '(variant { ok = 200 : nat16 })'
  // Call with owner principal
  mockIC.run_test("reset_model", reset_model, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);

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

    // candid_in -> '(variant { ok = 200 : nat16 })'
    candid_out_expected = "4449444c016b019cc2017a010000c800";
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

    // candid_in -> '(variant { ok = 200 : nat16 })'
    candid_out_expected = "4449444c016b019cc2017a010000c800";
    mockIC.run_test("upload_tokenizer_bytes_chunk",
                    upload_tokenizer_bytes_chunk, candid_in,
                    candid_out_expected, silent_on_trap, my_principal);

    i++;
  }

  // ==========================================================================
  // Mimic two principals at once are building a story...

  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("initialize", initialize, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);

  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  your_principal);

  // '()' -> '(variant { err =  "The Llama2 canister does not allow calling with anonymous principal." : text })'
  mockIC.run_test(
      "new_chat", new_chat, "4449444c0000",
      "4449444c016b01e58eb4027101000044546865204c6c616d61322063616e697374657220646f6573206e6f7420616c6c6f772063616c6c696e67207769746820616e6f6e796d6f7573207072696e636970616c2e",
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
      // '()' -> '(variant { ok = 200 : nat16 })'
      mockIC.run_test("new_chat", new_chat, "4449444c0000",
                      "4449444c016b019cc2017a010000c800", silent_on_trap,
                      principal);
    }

    // Loop to create a long story, 10 tokens at a time
    // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
    std::array<std::string, 2> prompt = {"Lilly went to", "Billy came from"};
    std::array<uint64_t, 2> steps = {0, 0};
    std::array<float, 2> temperature = {0.0, 0.0};
    std::array<float, 2> topp = {0.9, 0.9};
    std::array<uint64_t, 2> rng_seed = {0, 0};

    std::array<std::string, 2> generated_tokens = {"", ""};
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

        std::string err_text;
        CandidTypeVariant v_out;
        v_out.append("ok", CandidTypeText(&generated_tokens[j]));
        v_out.append("err", CandidTypeText(&err_text));

        CandidArgs A;
        A.append(v_out);
        CandidDeserialize(candid_out, A);
        if (err_text.size() > 0) {
          std::cout << "ERROR returned by inference function.\n";
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
    // '()' -> '(variant { ok = 200 : nat16 })'
    mockIC.run_test("new_chat", new_chat, "4449444c0000",
                    "4449444c016b019cc2017a010000c800", silent_on_trap,
                    my_principal);

    // Loop to create 1000 token long story, 10 tokens at a time
    // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
    std::string prompt = "";
    uint64_t steps = 10;
    float temperature = 0.0;
    float topp = 0.9;
    uint64_t rng_seed = 0;

    std::string generated_tokens = "";
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
                      my_principal, &candid_out);

      std::string err_text;
      CandidTypeVariant v_out;
      v_out.append("ok", CandidTypeText(&generated_tokens));
      v_out.append("err", CandidTypeText(&err_text));

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
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);

  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  expected_response = "-to-do-B-";
  if (model_to_use == 1) {
    // -> '(variant { ok = ""Once upon a time, there was a little girl named Lily. She loved to play outside in the park. One day, she saw a big, red ball. She wanted to play with it, but it was too high.\nLily\'s mom said, \"Lily, let\'s go to the park.\" Lily was sad and didn\'t know w\n"" : text })'
    expected_response =
        "4449444c016b019cc20171010000fd014f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e20746865207061726b2e204f6e65206461792c20736865207361772061206269672c207265642062616c6c2e205368652077616e74656420746f20706c617920776974682069742c206275742069742077617320746f6f20686967682e0a4c696c792773206d6f6d20736169642c20224c696c792c206c6574277320676f20746f20746865207061726b2e22204c696c79207761732073616420616e64206469646e2774206b6e6f772077";
  } else if (model_to_use == 2) {
    // -> '(variant { ok = "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, red ball in the sky. It was the sun! She thought it was so pretty.\nLily wanted to play with the ball, but it was too high up in the sky. She tried to jump and reach it, but she couldn\'t. Then, she had an idea. She would use a stick to knock the" : text })'
    expected_response =
        "4449444c016b019cc20171010000ed024f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e207468652073756e7368696e652e204f6e65206461792c20736865207361772061206269672c207265642062616c6c20696e2074686520736b792e20497420776173207468652073756e21205368652074686f756768742069742077617320736f207072657474792e0a4c696c792077616e74656420746f20706c61792077697468207468652062616c6c2c206275742069742077617320746f6f206869676820757020696e2074686520736b792e2053686520747269656420746f206a756d7020616e642072656163682069742c206275742073686520636f756c646e27742e205468656e2c207368652068616420616e20696465612e2053686520776f756c6420757365206120737469636b20746f206b6e6f636b20746865";
  } else if (model_to_use == 3) {
    // -> '(variant { ok = "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, yellow flower in the garden. It was a sunflower! Lily thought it was the most beautiful flower she had ever seen.\nLily\'s mom came outside and saw the sunflower too. \"Wow, that\'s a big flower!\" she said. \"Let\'s pick it and put it in" : text })'
    expected_response =
        "4449444c016b019cc20171010000e0024f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e207468652073756e7368696e652e204f6e65206461792c20736865207361772061206269672c2079656c6c6f7720666c6f77657220696e207468652067617264656e2e2049742077617320612073756e666c6f77657221204c696c792074686f756768742069742077617320746865206d6f73742062656175746966756c20666c6f77657220736865206861642065766572207365656e2e0a4c696c792773206d6f6d2063616d65206f75747369646520616e6420736177207468652073756e666c6f77657220746f6f2e2022576f772c2074686174277320612062696720666c6f77657221222073686520736169642e20224c65742773207069636b20697420616e642070757420697420696e";
  } else if (model_to_use == 4) {
    // -> '(variant { ok = "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, red apple on a tree. She wanted to eat it, but it was too high up.\nLily asked her friend, a little bird, \"Can you help me get the apple?\"\nThe bird said, \"Sure, I can fly up and get it for you.\"\nThe bird flew up to the apple" : text })'
    expected_response =
        "4449444c016b019cc20171010000d8024f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e207468652073756e7368696e652e204f6e65206461792c20736865207361772061206269672c20726564206170706c65206f6e206120747265652e205368652077616e74656420746f206561742069742c206275742069742077617320746f6f20686967682075702e0a4c696c792061736b65642068657220667269656e642c2061206c6974746c6520626972642c202243616e20796f752068656c70206d652067657420746865206170706c653f220a546865206269726420736169642c2022537572652c20492063616e20666c7920757020616e642067657420697420666f7220796f752e220a546865206269726420666c657720757020746f20746865206170706c65";
  }
  mockIC.run_test(
      "inference 1", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000000000803f6400000000000000000000000000000000",
      expected_response, silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature=0.0 & topp=0.9, still greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  mockIC.run_test(
      "inference 2", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000006666663f6400000000000000000000000000000000",
      expected_response, silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature>0.0 & topp=1.0: regular sampling
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 3", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f0000803f6400000000000000000000000000000000",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature>0.0 & topp<1.0: top-pp (nucleus) sampling
  // '(record {prompt = "" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 4", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f6666663f6400000000000000000000000000000000",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature=0.0: greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> '(variant { ok = "..." : text })'
  mockIC.run_test(
      "inference 1a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000000000803f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature=0.0 & topp=0.9, still greedy argmax sampling -> the story will be the same every time
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  // -> '(variant { ok = "..." : text })'
  mockIC.run_test(
      "inference 2a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b710100000000006666663f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature>0.0 & topp=1.0: regular sampling
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 1.0 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 3a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f0000803f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // A new chat
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("new_chat", new_chat, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);
  // With temperature>0.0 & topp<1.0: top-pp (nucleus) sampling
  // '(record {prompt = "Yesterday I went for a walk" : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  // -> --can not check on story--
  mockIC.run_test(
      "inference 4a", inference,
      "4449444c016c05b4e8c2e40373bbb885e80473a7f7b9a00878c5c8cea60878a4a3e1aa0b7101006666663f6666663f640000000000000000000000000000001b59657374657264617920492077656e7420666f7220612077616c6b",
      "", silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------------------
  // Users data

  // Verify that calls trap when not owner
  mockIC.run_trap_test("get_user_count", get_user_count, "4449444c0000",
                       silent_on_trap, your_principal);

  mockIC.run_trap_test("get_user_ids", get_user_ids, "4449444c0000",
                       silent_on_trap, your_principal);

  mockIC.run_trap_test("get_user_metadata", get_user_metadata,
                       "4449444c000171093269626f372d646961", silent_on_trap,
                       your_principal);

  // '()' -> '(2 : nat64)'
  mockIC.run_test("get_user_count", get_user_count, "4449444c0000",
                  "4449444c0001780200000000000000", silent_on_trap,
                  my_principal);

  /*
  '()' -> 
  (
    vec {
      "2ibo7-dia";
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae";
    },
  )
  */
  mockIC.run_test(
      "get_user_ids", get_user_ids, "4449444c0000",
      "4449444c016d71010002093269626f372d6469613f6578706d742d67747873772d696e66746a2d747461626a2d71687035732d6e6f7a75702d6e3362626f2d6b377a766e2d64673468652d6b6e6163332d6c6165",
      silent_on_trap, my_principal);

  // '("2ibo7-dia")' -> ... two vectors of nat64 ... but we can not check time-stamp!
  /*
  (
    vec {
      1_695_424_351_661_183_242 : nat64;
      1_695_424_351_661_875_047 : nat64;
      1_695_424_351_885_991_917 : nat64;
    },
    vec { 0 : nat64; 620 : nat64; 620 : nat64 },
  )
  */
  mockIC.run_test("get_user_metadata", get_user_metadata,
                  "4449444c000171093269626f372d646961", "", silent_on_trap,
                  my_principal);

  // -----------------------------------------------------------------------------------------
  // Reset the model
  // '()' -> '(variant { ok = 200 : nat16 })'
  mockIC.run_test("reset_model", reset_model, "4449444c0000",
                  "4449444c016b019cc2017a010000c800", silent_on_trap,
                  my_principal);

  // -----------------------------------------------------------------------------
  // returns 1 if any tests failed
  return mockIC.test_summary();
}