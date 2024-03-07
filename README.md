[![icpp-llm](https://github.com/icppWorld/icpp-llm/actions/workflows/cicd.yml/badge.svg)](https://github.com/icppWorld/icpp-llm/actions/workflows/cicd.yml)

# LLMs for the Internet Computer
<img src="./assets/icpp-llm-logo.png" alt="icpp-llm logo" width="200">

  Try it out in [ICGPT](https://icgpt.icpp.world) !

*The LLMs of this repo run in it's back-end canisters.*

# Getting Started

A step-by-step guide to deploy your first LLM to the internet computer is provided in [icpp_llama2/README.md](https://github.com/icppWorld/icpp_llm/blob/main/icpp_llama2/README.md).

# The Benefits of Running LLMs On-Chain

The canisters within the Internet Computer have certain constraints. They come with memory restrictions, and there's a cap on the number of instructions one can execute per message, as discussed [here](https://forum.dfinity.org/t/instruction-limit-is-crushing-me/22070/10?u=icpp).

This might lead one to question the rationale behind operating an LLM within an Internet Computer's canister.

For me, the primary incentive is the unparalleled simplicity of using the IC in comparison to conventional cloud platforms. You develop, deploy & test using a local replica of the cloud, and when everything is ready, you deploy it to the IC with just one command. Everything becomes instantly and securely accessible online. You can very easily restrict access to the endpoints in case you don't want to make it fully public yet and want to share it with a smaller group instead. 

Thanks to the Internet Computer's foundational cryptographic and blockchain technologies, concerns related to IT and security vanish. It's truly remarkable.

With such user-friendliness, the IC canister runtime serves as an ideal environment for my research pursuits. It complements the type of research presented in this paper that offers a dataset designed to boost the creation, examination, and study of Language Models for areas with scarce resources or specific niches:

 > [TinyStories: How Small Can Language Models Be and Still Speak
Coherent English?](https://arxiv.org/pdf/2305.07759.pdf)

Besides the ease of use and the enhanced security, running LLMs directly on-chain also facilitates a seamless integration of tokenomics, eliminating the need to juggle between a complex blend of web3 and web2 components, and I believe it will lead to a new category of Generative AI based dApps.



## Support

For support, kindly create a GitHub Issue as outlined in the [Support](https://docs.icpp.world/support.html) documentation page.
  
