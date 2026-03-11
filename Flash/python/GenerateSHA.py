"""Generate SHA-256 for a binary file.

Algorithm notes:
- SHA-256 produces a 256-bit (32-byte) digest.
- SHA-256 operates on 512-bit blocks internally.

References:
- FIPS PUB 180-4 (Secure Hash Standard)
- FIPS PUB 197 (AES)
- NIST SP 800-38D (GCM)
"""

from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path


def sha256_file(path: Path, chunk_size: int = 64 * 1024) -> bytes:
    """Compute SHA-256 digest of an input file."""
    digest = hashlib.sha256()
    with path.open("rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            digest.update(chunk)
    return digest.digest()


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate SHA-256 from a binary input file.",
    )
    parser.add_argument("inputFile", type=Path, help="Path to input binary file")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        metavar="outputFile",
        help="Path to output binary file (32 bytes). If omitted, prints SHA-256 hex value to stdout.",
    )
    return parser


def main() -> int:
    parser = _build_parser()
    args = parser.parse_args()

    sha = sha256_file(args.inputFile)

    if args.output is not None:
        args.output.write_bytes(sha)
    else:
        sys.stdout.write(f"{sha.hex().upper()}\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
