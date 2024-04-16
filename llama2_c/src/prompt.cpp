// The prompt

#include "prompt.h"

#include <string>

#include "ic_api.h"

void print_prompt(const Prompt &wire_prompt) {
  std::string msg = std::string(__func__) + "- model config:";
  msg += "\nwire_prompt.prompt       = " + wire_prompt.prompt;
  msg += "\nwire_prompt.steps        = " + std::to_string(wire_prompt.steps);
  msg +=
      "\nwire_prompt.temperature  = " + std::to_string(wire_prompt.temperature);
  msg += "\nwire_prompt.topp         = " + std::to_string(wire_prompt.topp);
  msg += "\nwire_prompt.rng_seed     = " + std::to_string(wire_prompt.rng_seed);
  IC_API::debug_print(msg);
}