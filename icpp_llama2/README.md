# [karpathy/llama2.c](https://github.com/karpathy/llama2.c) for the Internet Computer

Video: [How to run llama2.c on the Internet Computer](https://www.loom.com/share/a065b678df63462fb2f637d1b550b5d2?sid=1aeee693-25c0-4d1f-be0c-8231b53eb742)

After deploying icpp_llama2, you can call the llama2 canister's inference endpoint:

```bash
dfx canister call llama2 inference '(record {"prompt" = "" : text; "steps" = 20 : nat64; "temperature" = 0.9 : float32;})'
(
  variant {
    ok = "Once upon a time, there was a little boat named Bob. Bob loved to float on the water"
  },
)
```

# Instructions

- Install the C++ development environment for the Internet Computer ([docs](https://docs.icpp.world/installation.html)):
  - Install the required python packages *(icpp-pro & ic-py)*:
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
    ```
    *(Note: On Windows, just install dfx in wsl, and icpp-pro in PowerShell will know where to find it. )*
    

- Get a model checkpoint, as explained in [karpathy/llama2.c](https://github.com/karpathy/llama2.c):

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

   

- The *demo* script starts the local network, deploys llama2, and uploads the model & tokenizer:
  - `./demo.sh`  , on Linux / Mac
  - `.\demo.ps1` , in Windows PowerShell (Miniconda recommended)


- Call the llama2 canister's *inference* endpoint:
  ```bash
  dfx canister call llama2 inference '(record {"prompt" = "" : text; "steps" = 20 : nat64; "temperature" = 0.9 : float32;})'
  (
    variant {
      ok = "Once upon a time, there was a little boat named Bob. Bob loved to float on the water"
    },
  )
  ```



# Limitations

- When asking llama2 to generate a story of 25 steps or more, the canister throws an error due to a current limit on number of instructions per message.
- The weights are stored in static/global memory, so they are Orthogonally Persisted. Canisters have a 4 Gb limit at the moment.
