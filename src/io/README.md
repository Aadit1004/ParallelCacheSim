# Memory Access File Format Guide

This document outlines the proper format for writing memory access files used by the cache simulator. The file contains **read (R)** and **write (W)** operations with hexadecimal addresses and integer values.

## File Location and Naming Requirements
- All memory access files must be located in the `examples/` directory.
- Files must have a `.txt` extension.
- Example valid file path:
```bash
examples/memory_access.txt
```
**Note:** Files placed under `examples/tests/` or any other directory will not be recognized and will cause an exception to be thrown.

## Valid Formats
Each line must follow one of the two valid formats:

<br>
1. Read Operation (R)

```bash
R <hex_address>
```
- Example: `R 0x1A3F`
- Reads the value at the specified memory address.

<br>
2. Write Operation (W)

```bash
W <hex_address> <int_value>
```
- Example `W 0x2B7D 42`
- Writes the specified integer value to the given memory address.

## Invalid Formats

| Invalid Line Format | Issue |
| -------- | ------- |
| `X 0x1000` | Invalid operation (`X` is not R or W). |
| `R` | Missing address for read operation. |
| `R 1000` | Address must be in hexadecimal (`0x...`). |
| `W 0x2000` | Missing integer value for write operation. |
| `W 0x3000 abc` | Value must be an integer. |
| `W 0x4000 2147483648` | Value exceeds 32-bit integer range. |
| `W 0x5000 -2147483649` | Value is below 32-bit integer range. |

<br>
If a file contains invalid syntax, the program will **throw an exception**, stop processing further requests, and will provide an error message indicating the issue. Example:

```bash
[ERROR] Invalid write format: W 0x5000 abc
```

## Additional Rules
- **Hexadecimal addresses** must begin with 0x and contain only valid hex characters (0-9, a-f, A-F).
- **Integer values** for write operations must be within the 32-bit signed integer range
- **Blank lines** are allowed and will be ignored.
- **Extra spaces** between arguments will be trimmed and ignored.
- **Extra arguments** after the required ones are ignored but should be avoided.

## Example of Valid File

```bash
W 0x1000 42
R 0x1000
W 0x2000 99
R 0x2000
W 0x3000 -10153
R 0x3000
W 0x4000 123
R 0x4000
W 0x5000 -10
R 0x5000
```