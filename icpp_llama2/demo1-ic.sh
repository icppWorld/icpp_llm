#!/bin/sh

#######################################################################
# For Linux & Mac
#######################################################################

#######################################################################
# Before running this, create an identity 'demo1', with:
#    $ dfx identity new demo1
echo " "
echo "--------------------------------------------------"
echo "Generate a new story using llama2 (15M model), 10 tokens at a time, starting with an empty prompt."
dfx canister call --identity demo1 --network ic  llama2 new_chat '()'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

echo " "
echo "--------------------------------------------------"
echo "Generate a new story using llama2_260K, 10 tokens at a time, starting with an empty prompt."
dfx canister call --identity demo1 --network ic  llama2_260K new_chat '()'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_260K inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

echo " "
echo "--------------------------------------------------"
echo "Generate a new story using llama2_42M, 10 tokens at a time, starting with an empty prompt."
dfx canister call --identity demo1 --network ic  llama2_42M new_chat '()'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_42M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

echo " "
echo "--------------------------------------------------"
echo "Generate a new story using llama2_110M, 10 tokens at a time, starting with an empty prompt."
dfx canister call --identity demo1 --network ic  llama2_110M new_chat '()'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call --identity demo1 --network ic  llama2_110M inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'


# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" new_chat '()'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# dfx canister call --network ic  "obk3p-xiaaa-aaaag-ab2oa-cai" inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'