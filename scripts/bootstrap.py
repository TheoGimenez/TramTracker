"""Generate the static GTFS lookup files once before a PlatformIO build."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


# PlatformIO provides the project path when this file is used as an extra script.
PROJECT_DIR = Path(
    os.environ.get("PROJECT_DIR")
    or os.environ.get("PLATFORMIO_PROJECT_DIR")
    or Path.cwd()
).resolve()

GTFS_DIRECTORY = PROJECT_DIR / "data" / "gtfs"
STOP_TIMES = GTFS_DIRECTORY / "stop_times.txt"
GENERATED_STOPS = PROJECT_DIR / "include" / "generated_stops.h"
GENERATE_CONFIG_SCRIPT = PROJECT_DIR / "scripts" / "generate_app_config.py"
FETCH_SCRIPT = PROJECT_DIR / "scripts" / "fetch_tram_gtfs.py"
GENERATE_STOP_SCRIPT = PROJECT_DIR / "scripts" / "generate_stops.py"
GENERATE_NANOPB_SCRIPT = PROJECT_DIR / "scripts" / "generate_nanopb.py"
PROTO_DIRECTORY = PROJECT_DIR / "data" / "proto"
NANOPB_OUTPUT_DIRECTORY = PROJECT_DIR / "lib" / "gtfs_rt_pb"
DEFAULT_GTFS_STATIC_URL = "https://gtfsproxy.e-tam.fr/COMMON/GTFS.zip"


def run_script(script: Path, *arguments: str) -> None:
    """Run a project script with the same Python interpreter as PlatformIO."""
    subprocess.run(
        [sys.executable, str(script), *arguments],
        cwd=PROJECT_DIR,
        env=os.environ.copy(),
        check=True,
    )


def main() -> None:
    run_script(GENERATE_CONFIG_SCRIPT)

    proto_files = sorted(PROTO_DIRECTORY.glob("*.proto"))
    nanopb_outputs_missing = any(
        not output.exists()
        for proto_file in proto_files
        for output in (
            NANOPB_OUTPUT_DIRECTORY / f"{proto_file.stem}.pb.c",
            NANOPB_OUTPUT_DIRECTORY / f"{proto_file.stem}.pb.h",
        )
    )
    if nanopb_outputs_missing:
        run_script(GENERATE_NANOPB_SCRIPT)

    # Download the static feed only when the local source file is missing.
    if not STOP_TIMES.exists():
        print("data/gtfs/stop_times.txt is missing; fetching the TAM GTFS feed")
        run_script(FETCH_SCRIPT, "--url", DEFAULT_GTFS_STATIC_URL)
    else:
        print("stop_times.txt already exists; skipping GTFS download")

    # Generate the embedded lookup only when its output is missing.
    if not GENERATED_STOPS.exists():
        print("generated_stops.h is missing; generating the stop lookup")
        run_script(GENERATE_STOP_SCRIPT)
    else:
        print("generated_stops.h already exists; skipping stop generation")


main()
