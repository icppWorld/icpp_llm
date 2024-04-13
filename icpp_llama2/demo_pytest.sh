#!/bin/sh

dfx identity use default

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

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Setting canister_mode to chat-principal"
dfx canister call llama2_260K set_canister_mode chat-principal

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Checking health endpoint"
dfx canister call llama2_260K health

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Initializing the canister configurations"
python -m scripts.nft_init --network local --canister llama2_260K --nft-supply-cap 0 --nft-symbol "" --nft-name "" --nft-description ""

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Uploading the model & tokenizer"
python -m scripts.upload --network local --canister llama2_260K --model stories260K/stories260K.bin --tokenizer stories260K/tok512.bin

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Checking readiness endpoint"
dfx canister call llama2_260K ready

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the full smoketests with pytest"
pytest --network=local

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