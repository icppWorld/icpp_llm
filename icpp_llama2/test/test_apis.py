"""Test canister APIs

   First deploy the canister, then run:

   $ pytest --network=[local/ic] test_apis.py

"""
# pylint: disable=unused-argument, missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import, line-too-long

from pathlib import Path
import pytest
from icpp.smoketest import call_canister_api

# Path to the dfx.json file
DFX_JSON_PATH = Path(__file__).parent / "../dfx.json"

# Canister in the dfx.json file we want to test
CANISTER_NAME = "llama2"


def test__health(identity_anonymous: dict[str, str], network: str) -> None:
    # for IC network, the update calls take longer
    update_timeout_seconds = 3
    if network == "ic":
        update_timeout_seconds = 10

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="health",
        canister_argument="()",
        network=network,
        timeout_seconds=update_timeout_seconds,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__ready(identity_anonymous: dict[str, str], network: str) -> None:
    # for IC network, the update calls take longer
    update_timeout_seconds = 3
    if network == "ic":
        update_timeout_seconds = 10

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="ready",
        canister_argument="()",
        network=network,
        timeout_seconds=update_timeout_seconds,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__reset_model_err(identity_anonymous: dict[str, str], network: str) -> None:
    # for IC network, the update calls take longer
    update_timeout_seconds = 3
    if network == "ic":
        update_timeout_seconds = 10

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="reset_model",
        canister_argument="()",
        network=network,
        timeout_seconds=update_timeout_seconds,
    )
    expected_response = "(variant { err = 401 : nat16 })"
    assert response == expected_response


def test__inference_1(identity_anonymous: dict[str, str], network: str) -> None:
    # for IC network, the update calls take longer
    update_timeout_seconds = 3
    if network == "ic":
        update_timeout_seconds = 10

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="inference",
        canister_argument='(record {prompt = "" : text; steps = 19 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})',
        network=network,
        timeout_seconds=update_timeout_seconds,
    )
    assert "ok" in response


def test__inference_2(identity_anonymous: dict[str, str], network: str) -> None:
    # for IC network, the update calls take longer
    update_timeout_seconds = 3
    if network == "ic":
        update_timeout_seconds = 10

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="inference",
        canister_argument='(record {prompt = "Lilly " : text; steps = 19 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})',
        network=network,
        timeout_seconds=update_timeout_seconds,
    )
    assert "ok" in response
