# Canister resource requirements for llama2_c.

Do not edit this file. It is created with the command:

```bash
python -m scripts.icpp_llama2_sizer
```

### Tokenizer Memory (per model)

| Memory Type         | 260K<br>(MB) | 15M<br>(MB) | 42M<br>(MB) | 110M<br>(MB) |
| ------------------- | ------------ | ----------- | ----------- | ------------ |
| vocab_memory        | 0.00         | 0.12        | 0.12        | 0.12         |
| vocab_scores_memory | 0.00         | 0.12        | 0.12        | 0.12         |
| Total               | 0.00         | 0.24        | 0.24        | 0.24         |

### TransformerWeights Memory (per model)

| Memory Type           | 260K<br>(MB) | 15M<br>(MB) | 42M<br>(MB) | 110M<br>(MB) |
| --------------------- | ------------ | ----------- | ----------- | ------------ |
| token_embedding_table | 0.12         | 35.16       | 62.50       | 93.75        |
| rms_att_weight        | 0.00         | 0.01        | 0.02        | 0.04         |
| wq                    | 0.08         | 1.90        | 8.00        | 27.00        |
| wk                    | 0.04         | 1.90        | 8.00        | 27.00        |
| wv                    | 0.04         | 1.90        | 8.00        | 27.00        |
| wo                    | 0.08         | 1.90        | 8.00        | 27.00        |
| rms_ffn_weight        | 0.00         | 0.01        | 0.02        | 0.04         |
| w1                    | 0.21         | 5.06        | 21.50       | 72.00        |
| w2                    | 0.21         | 5.06        | 21.50       | 72.00        |
| w3                    | 0.21         | 5.06        | 21.50       | 72.00        |
| rms_final_weight      | 0.00         | 0.00        | 0.00        | 0.00         |
| wcls                  | 0.12         | 35.16       | 62.50       | 93.75        |
| Total                 | 1.12         | 93.11       | 221.53      | 511.57       |

### RunState Memory (per user)

| Memory Type | 260K<br>(MB) | 15M<br>(MB) | 42M<br>(MB) | 110M<br>(MB) |
| ----------- | ------------ | ----------- | ----------- | ------------ |
| x           | 0.00         | 0.00        | 0.00        | 0.00         |
| xb          | 0.00         | 0.00        | 0.00        | 0.00         |
| xb2         | 0.00         | 0.00        | 0.00        | 0.00         |
| hb          | 0.00         | 0.00        | 0.01        | 0.01         |
| hb2         | 0.00         | 0.00        | 0.01        | 0.01         |
| q           | 0.00         | 0.00        | 0.00        | 0.00         |
| k           | 0.00         | 0.00        | 0.00        | 0.00         |
| v           | 0.00         | 0.00        | 0.00        | 0.00         |
| att         | 0.02         | 0.01        | 0.03        | 0.05         |
| logits      | 0.00         | 0.12        | 0.12        | 0.12         |
| key_cache   | 0.31         | 1.69        | 16.00       | 36.00        |
| value_cache | 0.31         | 1.69        | 16.00       | 36.00        |
| Total       | 0.65         | 3.52        | 32.18       | 72.20        |

### Total Memory

| Memory Type                                 | 260K<br>(MB) | 15M<br>(MB) | 42M<br>(MB) | 110M<br>(MB) |
| ------------------------------------------- | ------------ | ----------- | ----------- | ------------ |
| Total Tokenizer Memory (per model)          | 0.00         | 0.24        | 0.24        | 0.24         |
| Total TransformerWeights Memory (per model) | 1.12         | 93.11       | 221.53      | 511.57       |
| Total RunState Memory (per user)            | 0.65         | 3.52        | 32.18       | 72.20        |
| Overall Total Memory                        | 1.76         | 96.62       | 253.71      | 583.78       |

### Canister Metrics

| Canister Metrics               | 260K<br>(MB) | 15M<br>(MB) | 42M<br>(MB) | 110M<br>(MB) |
| ------------------------------ | ------------ | ----------- | ----------- | ------------ |
| Max number of concurrent users | 6347         | 1138        | 120         | 49           |
