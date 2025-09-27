# MemoryDump

🌍 **[English](README.md) | [简体中文](README-CN.md)**

A lightweight Windows command-line tool to dump memory regions from a running process by PID or executable name.

## Features

- Dump memory by **PID** (`-p`) or **executable name** (`-n`) 
- Specify memory range by:
  - Start address + length (`-a` + `-l`)
  - Start address + end address (`-a` + `-e`)
- Supports both **hexadecimal** (e.g., `0x1000`) and **decimal** input
- Output to a binary file (`-o`)

## Usage

Usage: memdump [OPTIONS]

| OPTIONS | DESCRIPTION |
| -- | -- |
| `-p PID` | Target process by PID (integer) |
| `-n PROCESS_NAME` | Target process by name (e.g., notepad.exe) |
| `-a START_ADDR` | Memory start address (hex or decimal, e.g., 0x1000 or 4096) |
| `-e END_ADDR` | Memory end address (exclusive, hex or decimal) |
| `-l LENGTH` | Number of bytes to dump (hex or decimal) |
| `-o OUTPUT_FILE` | Output dump file path |

## Examples

```shell
# Dump 0x500 bytes starting at 0x1000 from notepad.exe
memdump -n notepad.exe -a 0x1000 -l 0x500 -o dump.bin

# Dump using PID and decimal values
memdump -p 23324 -a 4096 -l 1280 -o dump.bin

# Dump from address 0x1000 up to (but not including) 0x1500
memdump -p 23324 -a 0x1000 -e 0x1500 -o dump.bin

# Mix hex and decimal
memdump -p 12345 -a 0x7FF00000 -l 4096 -o region.bin
```

> Note: When using `-e END_ADDR`, the end address is exclusive - the byte at `END_ADDR` is not included in the dump.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.
