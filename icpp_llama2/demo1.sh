#!/bin/sh

#######################################################################
# For Linux & Mac
#######################################################################

#######################################################################
# Before running this, create an identity 'demo1', with:
#    $ dfx identity new demo1
echo " "
echo "--------------------------------------------------"
echo "Generate a new story, 10 tokens at a time, starting with an empty prompt."
dfx canister call --identity demo1 llama2 new_chat '()'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'