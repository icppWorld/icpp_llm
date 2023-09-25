# [karpathy/llama2.c](https://github.com/karpathy/llama2.c) for the Internet Computer

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

- The *demo* script starts the local network, deploys llama2, uploads the model & tokenizer, and generates two stories:
  - `./demo.sh`  , on Linux / Mac
  - `.\demo.ps1` , in Windows PowerShell (Miniconda recommended)

​        This screenshot shows the generation of the second story:

![icpp_llama2_without_limits](../assets/icpp_llama2_without_limits.png)


# stories260K

The default model is`stories15M.bin`, with `tokenizer.bin`, which contains the default llama2 tokenizer using 32000 tokens. 

For testing, it is nice to be able to work with a smaller model & tokenizer:
- Download the model & tokenizer from [huggingface stories260K](https://huggingface.co/karpathy/tinyllamas/tree/main/stories260K) and store them in:
  - stories260K/stories260K.bin
  - stories260K/tok512.bin
  - stories260K/tok512.model
- Deploy the canister:
  ```bash
  icpp build-wasm
  dfx deploy
  ```
- Upload the model & tokenizer:
  ```bash
  python -m scripts.upload --model stories260K/stories260K.bin --tokenizer stories260K/tok512.bin
  ```
- Inference is now possible with many more tokens before hitting the instruction limit, but off course, the stories are not as good:
  ```bash
  $ dfx canister call llama2 inference '(record {prompt = "Lilly went swimming yesterday  " : text; steps = 100 : nat64; temperature = 0.9 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
  (
    variant {
      ok = "Lilly went swimming yesterday  order. She had a great eyes that was closed. One day, she asked her mom why the cloud was close to the pond. \n\"Mommy, I will take clothes away,\" Lila said. \"Th\n"
    },
  )
  ```

# Fine tuning

  When making your own checkpoint via fine-tuning, make sure to train with the correct version of [karpathy/llama2.c](https://github.com/karpathy/llama2.c):

  | release | commit sha                                |
  | --------| ----------------------------------------- |
  | 0.1.0   |  b28c1e26c5ab5660267633e1bdc910a43b7255bf |
  | 0.2.0   |  57bf0e9ee4bbd61c98c4ad204b72f2b8881ac8cd |
  | 0.3.0   |  b9fb86169f56bd787bb644c62a80bbab56f8dccc |