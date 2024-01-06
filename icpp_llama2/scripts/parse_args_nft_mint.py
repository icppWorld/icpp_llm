"""Import command line arguments for the scripts."""

import argparse


def parse_args() -> argparse.Namespace:
    """Returns the command line arguments"""
    parser = argparse.ArgumentParser(
        description="Generate a story and optionally mint it."
    )
    parser.add_argument(
        "--network",
        type=str,
        default="local",
        help="Network: ic or local",
    )
    parser.add_argument(
        "--canister",
        type=str,
        default="charles",
        help="canister name in dfx.json",
    )
    parser.add_argument(
        "--candid",
        type=str,
        default="src/llama2.did",
        help="canister's candid file",
    )

    parser.add_argument(
        "--nft-config",
        type=str,
        default="charles-001.toml",
        help="A toml file with prompt and inference params for your NFTs' story",
    )

    args = parser.parse_args()
    return args
