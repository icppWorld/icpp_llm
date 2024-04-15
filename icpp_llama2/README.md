# [karpathy/llama2.c](https://github.com/karpathy/llama2.c) for the Internet Computer

# Try it out

The 15M parameter model is the backend of [ICGPT](https://icgpt.icpp.world/).

# Getting Started

- Install the C++ development environment for the Internet Computer ([docs](https://docs.icpp.world/installation.html)):

  - Create a python environment. (We like MiniConda, but use whatever you like!)
    ```bash
    conda create --name myllama2 python=3.11
    conda activate myllama2
    ```
  - Clone this repo and enter the icpp_llama2 folder
    ```bash
    git clone https://github.com/icppWorld/icpp_llm.git
    cd icpp_llm/icpp_llama2
    ```
  - Install the required python packages _(icpp-pro & ic-py)_:
    ```bash
    pip install -r requirements.txt
    ```
  - Install the [wasi-sdk](https://github.com/WebAssembly/wasi-sdk) compiler:
    ```bash
    icpp install-wasi-sdk
    ```
  - Install dfx:

    ```bash
    sh -ci "$(curl -fsSL https://internetcomputer.org/install.sh)"

    # Configure your shell
    source "$HOME/.local/share/dfx/env"
    ```

    _(Note: On Windows, just install dfx in wsl, and icpp-pro in PowerShell will know where to find it. )_

- Deploy the 15M parameter pre-trained model to canister `llama2_15M`:

  - Start the local network:
    ```bash
    dfx start --clean
    ```
  - Compile & link to WebAssembly (wasm), as defined in `icpp.toml`:
    ```bash
    icpp build-wasm
    ```
  - Deploy the wasm to a canister on the local network:
    ```bash
    dfx deploy llama2_15M
    ```
  - Check the health endpoint of the `llama2_15M` canister:
    ```bash
    $ dfx canister call llama2_15M health
    (variant { Ok = record { status_code = 200 : nat16 } })
    ```
  - Set the canister mode to 'chat-principal'
    ```
    $ dfx canister call llama2_15M set_canister_mode chat-principal
    (variant { Ok = record { status_code = 200 : nat16 } })
    ```
  - Upload the 15M parameter model & tokenizer:
    _(We have included a fine-tuned model based on a 4096 tokens tokenizer)_
    ```bash
    python -m scripts.upload --network local --canister llama2_15M --model models/stories15Mtok4096.bin --tokenizer tokenizers/tok4096.bin
    ```
  - Check the readiness endpoint, indicating it can be used for inference:
    ```bash
    $ dfx canister call llama2_15M ready
    (variant { Ok = record { status_code = 200 : nat16 } })
    ```

- Test it with dfx.

  - Generate a new story, 60 tokens at a time, starting with an empty prompt:

    _(Your story will be slightly different, because the temperature > 0.0)_

    ```bash
    $ dfx canister call llama2_15M new_chat '()'
    (variant { Ok = record { status_code = 200 : nat16 } })

    $ dfx canister call llama2_15M inference '(record {prompt = "" : text; steps = 60 : nat64; temperature = 0.1 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
    (
      variant {
        Ok = record {
          num_tokens = 60 : nat64;
          inference = "Once upon a time, there was a little girl named Lily. She loved to play outside in the park. One day, she saw a big tree with a swing hanging from it. She ran to the swing and started to swing back and forth. It was so much fun!\nSuddenly,";
        }
      },
    )

    $ dfx canister call llama2_15M inference '(record {prompt = "" : text; steps = 60 : nat64; temperature = 0.1 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
    (
      variant {
        Ok = record {
          num_tokens = 60 : nat64;
          inference = " Lily saw a boy who was crying. She asked him what was wrong. The boy said he lost his toy car. Lily felt sad for him and wanted to help. She asked the boy if he wanted to play with her. The boy smiled and said yes.\nLily and the boy played together";
        }
      },
    )

    # etc.
    # If you keep going, at some point the LLM will end the story
    ```

  - Now generate a new story, starting with your own, non-empty prompt:

    ```bash
    $ dfx canister call llama2_15M new_chat '()'
    (variant { Ok = record { status_code = 200 : nat16 } })

    $ dfx canister call llama2_15M inference '(record {prompt = "Timmy climbed in a tree" : text; steps = 60 : nat64; temperature = 0.1 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
    (
      variant {
        Ok = record {
          num_tokens = 5 : nat64;
          inference = "Timmy climbed in a tree";
        }
      },
    )

    $ dfx canister call llama2_15M inference '(record {prompt = "" : text; steps = 60 : nat64; temperature = 0.1 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
    (
      variant {
        Ok = record {
          num_tokens = 60 : nat64;
          inference = ". He was so excited to see what was on the roof. He looked up and saw a big bird. It was so big and it was so high up. Timmy wanted to get closer to the bird, so he started to climb.\nHe climbed and climbed until he reached the roof.";
        }
      },
    )

    # etc.
    # If you keep going, at some point the LLM will end the story
    ```

# Next steps

You also will notice that using dfx to generate stories is not very user friendly. We created a little react frontend, available as an open source project: https://github.com/icppWorld/icgpt, and deployed to the IC as deployed as [ICGPT](https://icgpt.icpp.world/).

# llama2_260K

For quick tests, we have included a really small model, with only 260K parameters and fine-tuned with a tokenizer of 512 tokens.

- model: stories260K/stories260k.bin
- tokenizer: stories260K/tok512.bin

The CI/CD using a GitHub actions workflow, and the demo_pytest.sh script are based on this model.

# demo_pytest.sh

- The demo_pytest.sh script starts the local network, deploys llama2_260K, uploads the model & tokenizer, and runs the QA with pytest:

  - `./demo_pytest.sh` , on Linux / Mac

# demo shell scripts

- The _demo_ script starts the local network, deploys llama2, uploads the model & tokenizer, and generates two stories:
  - `./demo.sh` , on Linux / Mac
  - `.\demo.ps1` , in Windows PowerShell (Miniconda recommended)

# More models

- You can get other model checkpoints, as explained in [karpathy/llama2.c](https://github.com/karpathy/llama2.c):

  This command downloads the 15M parameter model that was trained on the TinyStories dataset (~60MB download) and stores it in a `models` folder:

  ```bash
  # on Linux/Mac
  mkdir -p models
  wget -P models https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin
  ```

  ```powershell
  # in Windows PowerShell (Miniconda recommended)
  if (-not (Test-Path -Path .\models)) {
   New-Item -Path .\models -ItemType Directory
  }
  Invoke-WebRequest -Uri https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin -OutFile .\models\stories15M.bin
  ```

# Deploying to the IC main net

- Deploying IC main network is as usual, but you will likely run into a time-out error during upload of the model. You have to patch ic-py as described here:

  ```bash
  #
  # IMPORTANT: ic-py will through a timeout => patch it here:
  # /home/arjaan/miniconda3/envs/icpp-pro-w-llama2/lib/python3.11/site-packages/httpx/_config.py
  # # DEFAULT_TIMEOUT_CONFIG = Timeout(timeout=5.0)
  # DEFAULT_TIMEOUT_CONFIG = Timeout(timeout=99999999.0)
  # And perhaps here:
  # /home/arjaan/miniconda3/envs/<your-env>/lib/python3.11/site-packages/httpcore/_backends/sync.py#L28-L29
  with map_exceptions(exc_map):
            # PATCH AB
            timeout = 999999999
            # ENDPATCH
            self._sock.settimeout(timeout)
            return self._sock.recv(max_bytes)


  # Now, this command should work
  python -m scripts.upload --network local --canister llama2_15M --model models/stories15Mtok4096.bin --tokenizer tokenizers/tok4096.bin

  ```

# Run llama2.c natively

To do some prompt testing, it is nice to run llama2.c directly from the llama2.c github repo.

```bash
git clone https://github.com/icppWorld/llama2.c
cd llama2.c

conda create --name llama2-c python=3.10
conda activate llama2-c
pip install -r requirements.txt

make run

# Example command
./run models/stories15Mtok4096.bin -z tokenizers/tok4096.bin -t 0.1 -p 0.9 -i "Tony went swimming on the beach"
```

# Fine tuning

When making your own checkpoint via fine-tuning, make sure to train with the correct version of [karpathy/llama2.c](https://github.com/karpathy/llama2.c):

| release | commit sha                               |
| ------- | ---------------------------------------- |
| 0.1.0   | b28c1e26c5ab5660267633e1bdc910a43b7255bf |
| 0.2.0   | 57bf0e9ee4bbd61c98c4ad204b72f2b8881ac8cd |
| 0.3.0   | b9fb86169f56bd787bb644c62a80bbab56f8dccc |
