"""Generate CRC-32 for a binary file.

CRC parameters:
- Width: 32
- Polynomial: 0x04C11DB7
- Init: 0xFFFFFFFF
- RefIn: True
- RefOut: True
- XorOut: 0x00000000
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


CRC32_INIT = 0xFFFFFFFF

# Reflected form of 0x04C11DB7 for right-shift processing when RefIn=True.
CRC32_POLY_REFLECTED = 0xEDB88320


def crc32_update(crc: int, data: bytes) -> int:
    """Update CRC-32 state with a chunk of data."""
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ CRC32_POLY_REFLECTED
            else:
                crc >>= 1
    return crc & 0xFFFFFFFF


def crc32_file(path: Path, chunk_size: int = 64 * 1024) -> int:
    """Compute CRC-32 of an input file."""
    crc = CRC32_INIT
    with path.open("rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            crc = crc32_update(crc, chunk)
    return crc


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate CRC-32 from a binary input file.",
    )
    parser.add_argument("inputFile", type=Path, help="Path to input binary file")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        metavar="outputFile",
        help="Path to output binary file (4 bytes). If omitted, prints CRC-32 hex value to stdout.",
    )
    return parser


def main() -> int:
    parser = _build_parser()
    args = parser.parse_args()

    crc = crc32_file(args.inputFile)
    crc_bytes = crc.to_bytes(4, byteorder="little", signed=False)

    if args.output is not None:
        args.output.write_bytes(crc_bytes)
    else:
        sys.stdout.write(f"0x{crc:08X}\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
