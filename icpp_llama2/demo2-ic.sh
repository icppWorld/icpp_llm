#!/bin/sh

#######################################################################
# For Linux & Mac
#######################################################################

#######################################################################
# Before running this, create an identity 'demo2', with:
#    $ dfx identity new demo2
echo " "
echo "--------------------------------------------------"
echo "Generate a new story, 10 tokens at a time, using a starting prompt"
dfx canister call --identity demo2 --network ic  llama2 new_chat '()'
# You can build the prompt in multiple calls
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = "Lilly went to"           : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = "the beach this morning." : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = "She saw a little boat"   : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = "with her friend Billy"   : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# Followed by building out the story
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo2 --network ic  llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'