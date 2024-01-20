#######################################################################
# For Windows PowerShell (Miniconda is recommended)
#######################################################################

Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Stopping the local network in wsl"
wsl dfx stop

Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Starting the local network in wsl as a PowerShell background job"
$jobName = "llama2"

# Stop the job if it already exist
# Get a list of all the background jobs with a specific name
$jobs = Get-Job -Name $jobName -ErrorAction SilentlyContinue

# Stop all the jobs
$jobs | Remove-Job

# Start the local network with redirecting error messages to output stream
$job = Start-Job -ScriptBlock { wsl dfx start --clean 2>&1 } -Name $jobName

# Display the details of the job
$job | Format-Table

# Wait for 10 seconds
Write-Host " "
Write-Host "Waiting for 10 seconds to allow the local network to start..."
Start-Sleep -Seconds 10

# Get the output of the job
$output = Receive-Job -Job $job
Write-Host $output -ForegroundColor Green

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Building the wasm with wasi-sdk"
icpp build-wasm --to-compile all
# icpp build-wasm --to-compile mine

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Deploying the wasm to a canister on the local network"
wsl --% dfx deploy

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Checking health endpoint"
wsl --% dfx canister call llama2 health

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Upload the model & tokenizer for 260K and 15M"
python -m scripts.upload --canister llama2_260K --model stories260K/stories260K.bin --tokenizer stories260K/tok512.bin
python -m scripts.upload --canister llama2 --model models/stories15M.bin --tokenizer tokenizers/tokenizer.bin

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Checking readiness endpoint"
wsl --% dfx canister call llama2 ready

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Generate a new story, 10 tokens at a time, starting with an empty prompt."
wsl --% dfx canister call llama2 new_chat '()'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "" : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Generate a new story, 10 tokens at a time, using a starting prompt"
wsl --% dfx canister call llama2 new_chat '()'
# You can build the prompt in multiple calls
wsl --% dfx canister call llama2 inference '(record {prompt = "Lilly went to"           : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "the beach this morning." : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "She saw a little boat"   : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = "with her friend Billy"   : text; steps = 0  : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
# Followed by building out the story
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'
wsl --% dfx canister call llama2 inference '(record {prompt = ""                        : text; steps = 10 : nat64; temperature = 0.0 : float32; topp = 0.9 : float32; rng_seed = 0 : nat64;})'

# #######################################################################
# Write-Host " "
# Write-Host "--------------------------------------------------"
# Write-Host "Running the full smoketests with pytest"
# pytest --network=local

#######################################################################
# Write-Host " "
# Write-Host "--------------------------------------------------"
# Write-Host "Stopping the local network in wsl"
# wsl dfx stop

# #######################################################################
# Write-Host " "
# Write-Host "--------------------------------------------------"
# Write-Host "Building the Windows native debug executable with clang++"
# icpp build-native --to-compile all
# # icpp build-native --to-compile mine

# #######################################################################
# Write-Host " "
# Write-Host "--------------------------------------------------"
# Write-Host "Running the Windows native debug executable"
# .\build-native\mockic.exe