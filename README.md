# icpp-llm

LLMs for the Internet Computer, using [icpp-pro](https://docs.icpp.world).

For example, after deploying icpp_llama2, you can call an inference endpoint of the llama2 canister:

```bash
dfx canister call llama2 inference '(record {"prompt" = "" : text; "steps" = 20 : nat64; "temperature" = 0.9 : float32;})'
(
  variant {
    ok = "Once upon a time, there was a little boat named Bob. Bob loved to float on the water"
  },
)
```

See this video for a walk-through of build/deploy/test: [How to run llama2.c on the Internet Computer](https://www.loom.com/share/a065b678df63462fb2f637d1b550b5d2?sid=16bf073f-ee30-4248-8368-b6b79b9e8486)


## The LLMs included in this repo

Each LLM is a standalone icpp-pro project:

| LLM folder        | content                                                      |
| ------------- | ------------------------------------------------------------ |
| icpp_llama2   | To deploy [karpathy/llama2.c](https://github.com/karpathy/llama2.c) in a canister |

## Setup

- Follow the [icpp-pro installation](https://docs.icpp.world/installation.html) instructions
- Then check the README in each LLM folder