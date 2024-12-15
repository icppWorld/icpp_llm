# IC limitations

The IC has limitations with respect to memory & number of instructions per message.

From release to release, these limitations are being removed.

This README contains details how we test the IC improvements.

## SIMD support

*(Compile with the simd flags)*
*(Could not yet get this to work...)*

| Test                     | Max # tokens |
| ------------------------ | ------------ |
| 15Mtok4096 - update call | ?            |
| 42Mtok4096 - update call | ?            |

## With improved float handling

*(No code changes needed)*

| Test                     | Max # tokens |
| ------------------------ | ------------ |
| 15Mtok4096 - update call | 200+         |
| 42Mtok4096 - update call | 94           |



# Appendix A: SIMD in WebAssembly with Clang++

Add these flags to `icpp.toml`:

1. Enable WebAssembly SIMD:

-msimd128: This flag enables WebAssembly SIMD instructions.

NOTE: the flag `-O3` is on by default, for optimization and auto-vectorization.

2. Check Vectorization in WebAssembly:
You can use these flags for vectorization reporting with WebAssembly targets:

-Rpass=loop-vectorize: To check which loops were vectorized.
-Rpass-missed=loop-vectorize: To identify loops where vectorization was attempted but missed.
-Rpass-analysis=loop-vectorize: To analyze vectorization opportunities.
