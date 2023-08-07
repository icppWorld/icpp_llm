#!/bin/sh

#######################################################################
# This is a Linux & Mac shell script
#
# (-) Install icpp-pro in a python environment
# (-) Install dfx
# (-) In a terminal:
#
#     ./demo.sh
#
#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Make sure the LLM specific python dependencies are installed"
pip install -r requirements.txt

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
dfx deploy

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Checking health endpoint"
dfx canister call llama2 health

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Upload the model & tokenizer"
python -m scripts.upload

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Checking readiness endpoint"
dfx canister call llama2 ready

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Inference call  (Generate a story...)"
dfx canister call llama2 inference '(record {"prompt" = "" : text; "steps" = 20 : nat64; "temperature" = 0.9 : float32;})'

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