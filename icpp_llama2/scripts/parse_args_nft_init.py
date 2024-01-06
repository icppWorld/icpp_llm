"""Import command line arguments for the scripts."""

import argparse


def parse_args() -> argparse.Namespace:
    """Returns the command line arguments"""
    parser = argparse.ArgumentParser(description="Initialize the NFT collection")
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
        "--nft-supply-cap",
        type=int,
        default=25,
        help="The max number of NFTs that will ever be minted.",
    )
    parser.add_argument(
        "--nft-symbol",
        type=str,
        default="CHARLES",
        help="Symbol of the NFT Collection",
    )
    parser.add_argument(
        "--nft-name",
        type=str,
        default="Sir Charles The 3rd",
        help="Name of the NFT Collection",
    )
    parser.add_argument(
        "--nft-description",
        type=str,
        default=(
            "A Bitcoin Ordinal Collection powered by a "
            "C++ LLM running on the Internet Computer"
        ),
        help="Description of the NFT Collection",
    )

    args = parser.parse_args()
    return args
