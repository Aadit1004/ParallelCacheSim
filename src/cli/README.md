# Command-Line Arguments for `cache_sim`

To run the cache simulator, use the following format:

```bash
./cache_sim -cache_size <size> -threads <num> -policy <replacement> -assoc <ways> -write_policy <wp> -trace <file> [--verbose]
```

## Required Arguments

1. `-cache_size <size>`
    - Defines the cache size.
    - Must be one of: `small`, `medium`, or `large`
    - Cache size configurations:
        - **Small**: L1 = **8KB**, L2 = **32KB**, L3 = **256KB**, Memory = **4MB**
        - **Medium**: L1 = **16KB**, L2 = **64KB**, L3 = **512KB**, Memory = **16MB**
        - **Large**: L1 = **32KB**, L2 = **128KB**, L3 = **1MB**, Memory = **64MB**
2. `-threads <num>`
    - Number of threads used for simulation.
    - Valid range: 1 to 16.
    - Must be an even number if greater than 1.
3. `-policy <replacement>`
    - Cache replacement policy.
    - Must be one of: `FIFO`, `LRU`, or `LFU`.
4. `-assoc <ways>`
    - Cache associativity.
    - Must be one of: `1` (Direct-mapped), `4` (4-Way Set-Associative), or `0` (Fully Associative).
5. `-write_policy <wp>`
    - Defines write policy.
    - Must be one of: `WB` (Write-Back) or `WT` (Write-Through).
6. `-trace <file>`
    - Path to the **memory access trace file**.
    - The file **must** be located in the `examples/` directory.
    - The file **must** be of extension type `.txt`

## Optional Arguments

1. `--verbose`
    - Enables detailed logging of cache operations.