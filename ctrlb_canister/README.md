# ctrlb_canister (Control & Load Balancer)

Developed for Oxford Hackathon

### Setup

Install mops (https://mops.one/docs/install)
Install motoko dependencies:

```bash
# from folder backend/ctrlb_canister:

mops install
```

### Deploy

We assume you already have the `backend/llm_0` deployed, and the ctrlb_canister whitelisted.

Then you deploy the ctrlb_canister with:

```bash
# from folder backend/ctrlb_canister:

# Activate the same python environment as you defined for deploying the LLMs
# (This might not be needed, but I did it this way...)
conda activate llama2_c

# Deploy, initialize and test it
# WARNING: This will re-deploy the ctrlb_canister with loss of all existing data!
scripts/deploy.sh --network [local/ic]
```

### Test with dfx

```bash
dfx canister call ctrlb_canister whoami

# Run with same identity used to deploy (as a controller)
$ dfx canister call ctrlb_canister amiController
(variant { Ok = record { status_code = 200 : nat16 } })

# This call checks if the ctrlb_canister is whitelisted in the llm_0
$ dfx canister call ctrlb_canister isWhitelistLogicOk
(variant { Ok = record { status_code = 200 : nat16 } })

# Call the Inference endpoint
$ dfx canister call ctrlb_canister Inference '(record {prompt="Joe went swimming in the pool"; steps=30; temperature=0.1; topp=0.9; rng_seed=0})'
(
  variant {
    Ok = record {
      token_id = "pklc3-dnt23-rylls-wtd3z-a4nod-5b6iu-gha67-4dj4k-uqzci-srgi5-6ae";
      story = "Joe went swimming in the pool. He saw a big, shiny rock. He wanted to swim in the sky. He wanted to swim in the sky. He wanted to swim in the sky.\nJohn wanted to swim in the sky. He wanted to swing on the rock. He put the rock in the rock and put it in his rock. He put it in his rock. He put it in his rock. He pulled the rock and pulled it.\nJohn was sad. He wanted to help the rock. He did not know what to do. He did not know what to do. He did not know what to do. He did not know what to do. He did not know what to do.\nJohn said, \"I want to play with you. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it. You can\'t find it.\"\nThey run to the house and find a new friend. They are happy. They played with their";
      prompt = record {
        temperature = 0.1 : float64;
        topp = 0.9 : float64;
        steps = 60 : nat64;
        rng_seed = 0 : nat64;
        prompt = "Joe went swimming in the pool";
      };
    }
  },
)

# ----------------------------
# Below is for NFT collection

# Update a story for an NFT
$ dfx canister call ctrlb_canister NFTUpdate '(record {token_id="placeholder-0"})'
```
