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
CANISTER_NAME = "llama2_260K"
# CANISTER_NAME = "llama2" # 15M
# CANISTER_NAME = "llama2_42M"
# CANISTER_NAME = "llama2_110M"


def test__health(identity_anonymous: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="health",
        canister_argument="()",
        network=network,
        timeout_seconds=10,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__ready(identity_anonymous: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="ready",
        canister_argument="()",
        network=network,
        timeout_seconds=10,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__whoami(identity_default: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="whoami",
        canister_argument="()",
        network=network,
        timeout_seconds=10,
    )
    expected_response = f'("{identity_default["principal"]}")'
    assert response == expected_response


# ----------------------------------------------------------------------------------
def test__reset_model_err(identity_anonymous: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="reset_model",
        canister_argument="()",
        network=network,
        timeout_seconds=10,
    )
    expected_response = '(variant { Err = variant { Other = "Access Denied" } })'
    assert response == expected_response


def test__new_chat_anonymous(identity_anonymous: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="new_chat",
        canister_argument="()",
        network=network,
        timeout_seconds=10,
    )
    assert "Err" in response


def test__new_chat(identity_default: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="new_chat",
        canister_argument="()",
        network=network,
        timeout_seconds=10,
    )
    assert "Ok" in response


def test__inference_1_anonymous(
    identity_anonymous: dict[str, str], network: str
) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="inference",
        canister_argument='(record {prompt = "" : text; steps = 19 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})',
        network=network,
        timeout_seconds=10,
    )
    assert "Err" in response


def test__inference_1(identity_default: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="inference",
        canister_argument='(record {prompt = "" : text; steps = 19 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})',
        network=network,
        timeout_seconds=10,
    )
    assert "Ok" in response


def test__inference_2(identity_default: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="inference",
        canister_argument='(record {prompt = "Lilly " : text; steps = 19 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})',
        network=network,
        timeout_seconds=10,
    )
    assert "Ok" in response


# ----------------------------------------------------------------------------------
# Users data, requires that identity_default is the owner
# So, deploy with that default identity !!!
#
def test__get_users(identity_default: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_users",
        canister_argument="()",
        network=network,
    )
    principal = identity_default["principal"]
    expected_response = f'(variant {{ Ok = record {{ user_count = 1 : nat64; user_ids = vec {{ "{principal}";}};}} }})'
    assert response == expected_response


def test__get_user_metadata(identity_default: dict[str, str], network: str) -> None:
    _response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_user_metadata",
        canister_argument="4449444c000171093269626f372d646961",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    # No assert. A test just to make sure it returns.


# ----------------------------------------------------------------------------------
# Err testing
#
# Verify that Err when not owner
def test__err_get_users(identity_anonymous: dict[str, str], network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_users",
        canister_argument="4449444c0000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert (
        "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564"
        == response
    )


def test__err_get_user_metadata(
    identity_anonymous: dict[str, str], network: str
) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_user_metadata",
        canister_argument="4449444c000171093269626f372d646961",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert (
        "4449444c026b01b0ad8fcd0c716b01c5fed20100010100000d4163636573732044656e696564"
        == response
    )
