[![icpp-llm](https://github.com/icppWorld/icpp-llm/actions/workflows/cicd.yml/badge.svg)](https://github.com/icppWorld/icpp-llm/actions/workflows/cicd.yml)

# LLMs for the Internet Computer

![icpp-llm logo](./assets/icpp-llm-logo.png)

Video: [How to run llama2.c on the Internet Computer](https://www.loom.com/share/a065b678df63462fb2f637d1b550b5d2?sid=1aeee693-25c0-4d1f-be0c-8231b53eb742)

# Try it

`icpp_llama2` with the stories15M.bin is running on-chain in canister `4c4bn-daaaa-aaaag-abvcq-cai`. 

You can call it's inference endpoint with:

```bash
dfx canister call --network ic 4c4bn-daaaa-aaaag-abvcq-cai inference '(record {prompt = "" : text; steps = 20 : nat64; temperature = 0.8 : float32; topp = 1.0 : float32;})'
(
  variant {
    ok = "Once upon a time, there was a little boat named Bob. Bob loved to float on the water"
  },
)
```


## The LLMs included in this repo

| LLM folder    | release | reference (commit sha)                                                     |
| ------------- | --------| ---------------------------------------------------- |
| [icpp_llama2](https://github.com/icppWorld/icpp-llm/tree/main/icpp_llama2)   | 0.1.0 | [karpathy/llama2.c](https://github.com/karpathy/llama2.c) (b28c1e26c5ab5660267633e1bdc910a43b7255bf) |
|   | 0.2.0 | [karpathy/llama2.c](https://github.com/karpathy/llama2.c) (57bf0e9ee4bbd61c98c4ad204b72f2b8881ac8cd) |


## Instructions

See the README in each LLM folder


## Support

For support, kindly create a GitHub Issue as outlined in the [Support](https://docs.icpp.world/support.html) documentation page.
  