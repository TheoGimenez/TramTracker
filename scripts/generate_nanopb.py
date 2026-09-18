#!/usr/bin/env python3
"""Generate Nanopb C sources from the project's Protocol Buffer schemas."""

from __future__ import annotations

import importlib.util
import os
import shutil
import subprocess
import sys
from pathlib import Path


PROJECT_DIR = Path(
    os.environ.get("PROJECT_DIR")
    or os.environ.get("PLATFORMIO_PROJECT_DIR")
    or Path.cwd()
).resolve()
PROTO_DIRECTORY = PROJECT_DIR / "data" / "proto"
OUTPUT_DIRECTORY = PROJECT_DIR / "lib" / "gtfs_rt_pb"
RED = "\033[31m"
RESET = "\033[0m"


def warning(message: str) -> None:
    """Print a warning in red without stopping the PlatformIO build."""
    print(f"{RED}WARNING: {message}{RESET}", flush=True)


def find_nanopb_generator() -> Path | None:
    """Find the Nanopb generator installed by PlatformIO."""
    candidates = sorted(
        (PROJECT_DIR / ".pio" / "libdeps").glob(
            "*/Nanopb/generator/nanopb_generator.py"
        )
    )
    return candidates[0] if candidates else None


def required_outputs(proto_files: list[Path]) -> list[Path]:
    """Return all generated files expected for the input schemas."""
    return [
        output
        for proto_file in proto_files
        for output in (
            OUTPUT_DIRECTORY / f"{proto_file.stem}.pb.c",
            OUTPUT_DIRECTORY / f"{proto_file.stem}.pb.h",
        )
    ]


def main() -> None:
    proto_files = sorted(PROTO_DIRECTORY.glob("*.proto"))
    if not proto_files:
        warning(f"No .proto files found in {PROTO_DIRECTORY}")
        return

    generator = find_nanopb_generator()
    if generator is None:
        warning("Nanopb is not installed in PlatformIO dependencies")
        return

    try:
        protobuf_installed = importlib.util.find_spec("google.protobuf") is not None
    except ModuleNotFoundError:
        protobuf_installed = False

    if not protobuf_installed:
        warning("Nanopb generator dependency 'google.protobuf' is not installed")
        return

    if shutil.which("protoc") is None:
        warning("'protoc' is not installed or is not available in PATH")
        return

    nanopb_proto_directory = generator.parent / "proto"
    command = [
        sys.executable,
        str(generator),
        "--proto-path",
        str(PROTO_DIRECTORY),
        "--proto-path",
        str(nanopb_proto_directory),
        "--output-dir",
        str(OUTPUT_DIRECTORY),
        *[str(proto_file) for proto_file in proto_files],
    ]

    OUTPUT_DIRECTORY.mkdir(parents=True, exist_ok=True)
    print(f"Generating Nanopb files in {OUTPUT_DIRECTORY}")
    try:
        subprocess.run(command, cwd=PROJECT_DIR, check=True)
    except (OSError, subprocess.CalledProcessError) as error:
        warning(f"Nanopb generation failed: {error}")
        return

    print(f"Generated {len(required_outputs(proto_files))} Nanopb files")


if __name__ == "__main__":
    main()
