#pragma once

#include <string>
#include <cstdint>

class Prompt {
public:
  std::string prompt{""};
  uint64_t steps{256};
  float temperature{1.0};
  float topp{0.9};
  uint64_t rng_seed{0};
};

// Motoko does not support float32, so we use float64, and then map PromptMo onto Prompt
class PromptMo {
public:
  std::string prompt{""};
  uint64_t steps{256};
  double temperature{1.0};
  double topp{0.9};
  uint64_t rng_seed{0};
};

void print_prompt(const Prompt &wire_prompt);