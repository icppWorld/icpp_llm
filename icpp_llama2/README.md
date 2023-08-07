# llama2: Inference with the Llama 2 model running in a Canister

- [Install the IC development environment for C++](https://docs.icpp.world/installation.html)
- Following instructions similar to [karpathy/llama2.c](https://github.com/karpathy/llama2.c):

   You need a model checkpoint. The demo scripts of the next step depend on you downloading the 15M parameter model that was trained on the TinyStories dataset (~60MB download) and store it in the `models` folder:

   ```powershell
   # in Windows PowerShell (Miniconda recommended)
   if (-not (Test-Path -Path .\models)) {
    New-Item -Path .\models -ItemType Directory
   }
   Invoke-WebRequest -Uri https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin -OutFile .\models\stories15M.bin
   ```

   ```bash
   # on Linux/Mac
   mkdir -p models
   wget -P models https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin
   ```

- Now you can see how it works by running these shell scripts:  
  - `.\demo.ps1` , in Windows PowerShell (Miniconda recommended)
  - `./demo.sh`  , on Linux / Mac

# Limitations

- When asking llama2 to generate a story of 25 steps, 
- The weights are stored in static/global memory, so they are Orthogonally Persisted. There is a 4 Gb limit at the moment.
