"""Import command line arguments for the scripts."""

import argparse


def parse_args() -> argparse.Namespace:
    """Returns the command line arguments"""
    parser = argparse.ArgumentParser(
        description="Load a model and set temperature and steps"
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
        default="llama2",
        help="canister name",
    )
    parser.add_argument(
        "--candid",
        type=str,
        default="src/llama2.did",
        help="canister's candid file",
    )
    parser.add_argument(
        "--model",
        type=str,
        default="models/stories15M.bin",
        help="Model file (e.g. models/stories15M.bin)",
    )
    parser.add_argument(
        "--tokenizer",
        type=str,
        default="tokenizers/tokenizer.bin",
        help="Tokenizer file (e.g. tokenizers/tokenizer.bin)",
    )
    parser.add_argument(
        "--chunksize",
        type=float,
        default=1.9,
        help="Chunk Size for upload, in Mb",
    )
    parser.add_argument(
        "--temperature", type=float, default=0.9, help="Temperature (e.g. 1.0, or 0.0)"
    )
    parser.add_argument(
        "--steps",
        type=int,
        default=256,
        help="Max number of steps to run for, 0: use seq_len",
    )

    args = parser.parse_args()
    return args
