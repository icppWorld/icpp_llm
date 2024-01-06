"""Mint a CHARLES from a toml file

Run with:

    python -m scripts.nft_mint --network local --canister charles --nft-config <path-to-your-nft-config-toml-file>   # pylint: disable=line-too-long
"""

# pylint: disable=invalid-name, too-few-public-methods, no-member, too-many-statements, broad-except

import sys
from pathlib import Path
from pprint import pprint
from typing import Dict, Any
from .ic_py_canister import get_canister
from .parse_args_nft_mint import parse_args
from .nft_config import read_nft_config

ROOT_PATH = Path(__file__).parent.parent

#  0 - none
#  1 - minimal
#  2 - a lot
DEBUG_VERBOSE = 1


def main() -> int:
    """Summarize the NFT Collection."""

    args = parse_args()

    network = args.network
    canister_name = args.canister
    candid_path = ROOT_PATH / args.candid

    nft_config_path = Path(args.nft_config)

    dfx_json_path = ROOT_PATH / "dfx.json"

    print(
        f"Summary of canister & NFT Collection:"
        f"\n - network         = {network}"
        f"\n - canister        = {canister_name}"
        f"\n - dfx_json_path   = {dfx_json_path}"
        f"\n - candid_path     = {candid_path}"
        f"\n - nft_config_path = {nft_config_path}"
    )

    nft_config: Dict[Any, Any] = read_nft_config(nft_config_path)

    # ---------------------------------------------------------------------------
    # get ic-py based Canister instance
    canister_llama2 = get_canister(canister_name, candid_path, network)

    # check health (liveness)
    print("--\nChecking liveness of canister (did we deploy it!)")
    response = canister_llama2.health()
    if response == [True]:
        print("Ok!")
    else:
        print("Not OK, response is:")
        print(response)

    # ---------------------------------------------------------------------------
    # check readiness for inference
    print("--\nChecking if the canister is ready for inference.")
    response = canister_llama2.ready()
    if response[0] is True:
        if DEBUG_VERBOSE >= 2:
            print("OK!")
    else:
        print("Something went wrong:")
        print(response)
        sys.exit()

    # ---------------------------------------------------------------------------
    # Mint the NFT
    #
    bitcoin_ordinal = nft_config["bitcoin_ordinal"]
    print(
        "--\nMinting NFT for:"
        f"\n - bitcoin_ordinal_id  = {bitcoin_ordinal['bitcoin_ordinal_id']}"
    )
    error: bool = False
    try:
        response = canister_llama2.nft_mint(bitcoin_ordinal)
        print(response)
        if "ok" in response[0].keys():
            if DEBUG_VERBOSE >= 2:
                print("OK!")
        else:
            error = True
    except Exception as e:
        print(f"An error occurred: {e}")
        error = True

    if error:
        print("Something went wrong")
        user_input = input("Do you still want to continue? [yes/no] ")
        if user_input.lower() not in ("yes", "y"):
            sys.exit()

    # ---------------------------------------------------------------------------
    # Start the story

    prompt: Dict[Any, Any] = nft_config["prompt"]

    print(
        "--\nStarting a new story with:"
        f"\n - prompt          = {prompt['prompt']}"
        f"\n - temperature     = {prompt['temperature']}"
        f"\n - topp            = {prompt['topp']}"
        f"\n - steps           = {prompt['steps']}"
        f"\n - rng_seed        = {prompt['rng_seed']}"
    )
    response = canister_llama2.nft_story_start(bitcoin_ordinal, prompt)
    print(response)
    if "ok" in response[0].keys():
        if DEBUG_VERBOSE >= 2:
            print("OK!")
    else:
        print("Something went wrong:")
        sys.exit()

    # ---------------------------------------------------------------------------
    # Continue the story until we reached the end.

    print("--\nContinuing the story until done...")
    prompt["prompt"] = ""
    while True:
        response = canister_llama2.nft_story_continue(bitcoin_ordinal, prompt)
        print(response)
        if "ok" in response[0].keys():
            # Check if the response is an empty string. If it is, break out of the loop.
            if response[0]["ok"] == "":
                print("The end!")
                break
        else:
            print("Something went wrong:")
            sys.exit()

    # ---------------------------------------------------------------------------
    # Summarize the NFT Collection
    print("--\nSummary of the NFT Collection.")
    response = canister_llama2.nft_metadata()
    pprint(response[0])

    # ---------------------------------------------------------------------------
    return 0


if __name__ == "__main__":
    sys.exit(main())
