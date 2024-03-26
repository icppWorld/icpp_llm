#!/bin/sh

#######################################################################
# For Linux & Mac
#######################################################################

echo " "
echo "--------------------------------------------------"
echo "Stopping the local network"
dfx stop

echo " "
echo "--------------------------------------------------"
echo "Starting the local network as a background process"
dfx start --clean --background

#######################################################################
echo "--------------------------------------------------"
echo "Building the wasm with wasi-sdk"
icpp build-wasm --to-compile all
# icpp build-wasm --to-compile mine

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Deploying the wasm to a canister on the local network"
dfx deploy llama2_260K
dfx deploy llama2

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Setting canister_mode to chat-principal"
dfx canister call llama2_260K set_canister_mode chat-principal
dfx canister call llama2 set_canister_mode chat-principal

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Checking health endpoint"
dfx canister call llama2_260K health
dfx canister call llama2 health

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Initializing the canister configurations"
python -m scripts.nft_init --network local --canister llama2_260K --nft-supply-cap 0 --nft-symbol "" --nft-name "" --nft-description ""
python -m scripts.nft_init --network local --canister llama2 --nft-supply-cap 0 --nft-symbol "" --nft-name "" --nft-description ""

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Upload the model & tokenizer for 260K and 15M"
python -m scripts.upload --canister llama2_260K --model stories260K/stories260K.bin --tokenizer stories260K/tok512.bin
python -m scripts.upload --canister llama2 --model models/stories15M.bin --tokenizer tokenizers/tokenizer.bin

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Checking readiness endpoint"
dfx canister call llama2_260K ready
dfx canister call llama2 ready

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Generate a new story, 10 tokens at a time, starting with an empty prompt."
dfx canister call llama2 new_chat '()'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

echo " "
echo "--------------------------------------------------"
echo "Generate a new story, 10 tokens at a time, using a starting prompt"
dfx canister call llama2 new_chat '()'
# You can build the prompt in multiple calls
dfx canister call llama2 inference '(record {prompt = "Lilly went to"           : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "the beach this morning." : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "She saw a little boat"   : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = "with her friend Billy"   : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# Followed by building out the story
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

# #######################################################################
# echo " "
# echo "--------------------------------------------------"
# echo "Running the full smoketests with pytest"
# pytest --network=local

#######################################################################
# echo "--------------------------------------------------"
# echo "Stopping the local network"
# dfx stop

# #######################################################################
# echo " "
# echo "--------------------------------------------------"
# echo "Building the OS native debug executable with clang++"
# icpp build-native --to-compile all
# # icpp build-native --to-compile mine

# #######################################################################
# echo " "
# echo "--------------------------------------------------"
# echo "Running the OS native debug executable"
# ./build-native/mockic.exe