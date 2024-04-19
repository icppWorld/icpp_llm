#!/bin/sh

#######################################################################
# For Linux & Mac
#######################################################################
export PYTHONPATH="${PYTHONPATH}:$(realpath ../../../icpp_llm/llama2_c)"


#######################################################################
# --network [local|ic]
#######################################################################

# Default network type is local
NETWORK_TYPE="local"
NUM_LLMS_DEPLOYED=10
NUM_LLMS_ROUND_ROBIN=10 # how many LLMs we actually use


NUM_NFTS=25 # Total number of NFTs in the collection. Set to zero for ICGPT deploy
NUM_STEPS=60
TEMPERATURE=0.0  # for consistent performance testing
# TEMPERATURE=0.1  # for production deployment

# Copy this from .env in llms folder after deployment of the llms
CANISTER_ID_LLM_9='a4tbr-q4aaa-aaaaa-qaafq-cai'
CANISTER_ID_LLM_8='a3shf-5eaaa-aaaaa-qaafa-cai'
CANISTER_ID_LLM_7='asrmz-lmaaa-aaaaa-qaaeq-cai'
CANISTER_ID_LLM_6='avqkn-guaaa-aaaaa-qaaea-cai'
CANISTER_ID_LLM_5='by6od-j4aaa-aaaaa-qaadq-cai'
CANISTER_ID_LLM_4='b77ix-eeaaa-aaaaa-qaada-cai'
CANISTER_ID_LLM_3='bw4dl-smaaa-aaaaa-qaacq-cai'
CANISTER_ID_LLM_2='br5f7-7uaaa-aaaaa-qaaca-cai'
CANISTER_ID_LLM_1='be2us-64aaa-aaaaa-qaabq-cai'
CANISTER_ID_LLM_0='bkyz2-fmaaa-aaaaa-qaaaq-cai'

# Parse command line arguments for network type
while [ $# -gt 0 ]; do
    case "$1" in
        --network)
            shift
            if [ "$1" = "local" ] || [ "$1" = "ic" ]; then
                NETWORK_TYPE=$1
                if [ "$NETWORK_TYPE" = "ic" ]; then
                    CANISTER_ID_LLM_9='lhwda-diaaa-aaaag-ak5ka-cai'
                    CANISTER_ID_LLM_8='k77wd-niaaa-aaaag-ak5oa-cai'
                    CANISTER_ID_LLM_7='kkyho-maaaa-aaaag-ak5nq-cai'
                    CANISTER_ID_LLM_6='knzb2-byaaa-aaaag-ak5na-cai'
                    CANISTER_ID_LLM_5='ke2kg-xqaaa-aaaag-ak5mq-cai'
                    CANISTER_ID_LLM_4='kd3ms-2iaaa-aaaag-ak5ma-cai'
                    CANISTER_ID_LLM_3='lovi4-vaaaa-aaaag-ak5lq-cai'
                    CANISTER_ID_LLM_2='ljuoi-yyaaa-aaaag-ak5la-cai'
                    CANISTER_ID_LLM_1='laxfu-oqaaa-aaaag-ak5kq-cai'
                    CANISTER_ID_LLM_0='lkh5o-3yaaa-aaaag-acguq-cai'
                fi
            else
                echo "Invalid network type: $1. Use 'local' or 'ic'."
                exit 1
            fi
            shift
            ;;
        *)
            echo "Unknown argument: $1"
            echo "Usage: $0 --network [local|ic]"
            exit 1
            ;;
    esac
done

echo "Using network type: $NETWORK_TYPE"

#######################################################################

echo " "
echo "--------------------------------------------------"
echo "Deploying the ctrlb_canister"
echo dfx deploy ctrlb_canister -m reinstall --network $NETWORK_TYPE
dfx deploy ctrlb_canister -m reinstall --network $NETWORK_TYPE

echo " "
echo "--------------------------------------------------"
echo "Checking health endpoint"
output=$(dfx canister call ctrlb_canister health --network $NETWORK_TYPE)

if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
    echo "ctrlb_canister is not healthy. Exiting."
    exit 1
else
    echo "ctrlb_canister is healthy."
fi

echo " "
echo "--------------------------------------------------"
echo "Checking whitelist logic"
output=$(dfx canister call ctrlb_canister isWhitelistLogicOk --network $NETWORK_TYPE)

if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
    echo "ctrlb_canister is not whitelisted. Exiting."
    exit 1
else
    echo "ctrlb_canister is whitelisted."
fi

echo " "
echo "--------------------------------------------------"
echo "Registering $NUM_LLMS_DEPLOYED llm canisterIDs with the ctrlb_canister"
CANISTER_ID_LLMS=(
    $CANISTER_ID_LLM_0
    $CANISTER_ID_LLM_1
    $CANISTER_ID_LLM_2
    $CANISTER_ID_LLM_3
    $CANISTER_ID_LLM_4
    $CANISTER_ID_LLM_5
    $CANISTER_ID_LLM_6
    $CANISTER_ID_LLM_7
    $CANISTER_ID_LLM_8
    $CANISTER_ID_LLM_9
)
llm_id_start=0
llm_id_end=$((NUM_LLMS_DEPLOYED - 1))
for i in $(seq $llm_id_start $llm_id_end)
do
    CANISTER_ID_LLM=${CANISTER_ID_LLMS[$i]}
    output=$(dfx canister call ctrlb_canister set_llm_canister_id "(record { canister_id = \"$CANISTER_ID_LLM\" })" --network $NETWORK_TYPE)

    if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
        echo "Error calling set_llm_canister_id for $CANISTER_ID_LLM. Exiting."
        exit 1
    else
        echo "Successfully called set_llm_canister_id for $CANISTER_ID_LLM."
    fi
done

echo " "
echo "--------------------------------------------------"
echo "Setting NUM_LLMS_ROUND_ROBIN to $NUM_LLMS_ROUND_ROBIN"
output=$(dfx canister call ctrlb_canister setRoundRobinLLMs "($NUM_LLMS_ROUND_ROBIN)" --network $NETWORK_TYPE)

if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
    echo "setRoundRobinLLMs call failed. Exiting."
    exit 1
else
    echo "setRoundRobinLLMs was successful."
fi



echo " "
echo "--------------------------------------------------"
echo "Checking readiness endpoint"
output=$(dfx canister call ctrlb_canister ready --network $NETWORK_TYPE)

if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
    echo "ctrlb_canister is not ready. Exiting."
    exit 1
else
    echo "ctrlb_canister is ready for inference."
fi


echo " "
echo "--------------------------------------------------"
echo "Generating bindings for the frontend"
dfx generate


echo " "
echo "--------------------------------------------------"
echo "Test the Inference endpoint used by ICGPT"

command="dfx canister call ctrlb_canister Inference \"(record {prompt=\\\"Joe went swimming in the pool\\\"; steps=$NUM_STEPS; temperature=0.1; topp=0.9; rng_seed=0})\" --network $NETWORK_TYPE"
echo "Calling Inference endpoint with: $command"
eval "$command"

if [ $NUM_NFTS -eq 0 ]; then
    echo "All done with ICGPT backend deployment"
    echo "Skipping NFT setup"
    exit 0
fi

echo " "
echo "--------------------------------------------------"
echo "Adding NFTs with a Startprompt as used by CHARLES"

echo "For performance testing purposes, we will use an empty prompt and temperature=0 for every NFT"
promptTexts=()
for ((i=0; i<$NUM_NFTS; i++)); do
    promptTexts[$i]=""
done

for promptText in "${promptTexts[@]}"; do
    output=$(dfx canister call ctrlb_canister NFTAddStartPrompt "(record {prompt=\"$promptText\"; steps=$NUM_STEPS; temperature=$TEMPERATURE; topp=0.9; rng_seed=0})" --network $NETWORK_TYPE)

    if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
        echo "Error when calling NFTAddStartPrompt. Exiting."
        exit 1
    else
        echo "Successfully added NFT with: prompt=\"$promptText\", steps=$NUM_STEPS, temperature=$TEMPERATURE, topp=0.9, rng_seed=0 "
    fi
done


echo " "
echo "--------------------------------------------------"
echo "Set TokenIds (using defaults for now)"
output=$(dfx canister call ctrlb_canister NFTSetTokenIds "()" --network $NETWORK_TYPE)

if [ "$output" != "(variant { Ok = record { status_code = 200 : nat16 } })" ]; then
    echo "Error when calling NFTSetTokenIds. Exiting."
    exit 1
else
    echo "Successfully called NFTSetTokenIds."
fi


echo " "
echo "--------------------------------------------------"
echo "Test the NFTUpdate endpoint for placeholder-0"
dfx canister call ctrlb_canister NFTUpdate '(record {token_id="placeholder-0"})' --network $NETWORK_TYPE

echo " "
echo "--------------------------------------------------"
echo "Test the NFTUpdate endpoint for placeholder-999"
dfx canister call ctrlb_canister NFTUpdate '(record {token_id="placeholder-999"})' --network $NETWORK_TYPE

echo " "
echo "--------------------------------------------------"
echo "IMPORTANT: we deployed with TEMPERATURE = $TEMPERATURE"
echo "*** THIS DEPLOYMENT IS FOR PERFORMANCE TESTING ONLY ***"
