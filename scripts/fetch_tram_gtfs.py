#!/usr/bin/env python3
"""Download and extract the TAM tram GTFS archive."""

from __future__ import annotations

import argparse
import shutil
import tempfile
import urllib.request
import zipfile
from pathlib import Path

DEFAULT_URL = "https://gtfsproxy.e-tam.fr/COMMON/GTFS.zip"
DEFAULT_OUTPUT = Path("data/gtfs")


def download(url: str, destination: Path) -> None:
    """Download the GTFS archive to a temporary file."""
    request = urllib.request.Request(
        url,
        headers={"User-Agent": "TramTracker GTFS updater/1.0"},
    )
    with urllib.request.urlopen(request, timeout=60) as response:
        with destination.open("wb") as output:
            shutil.copyfileobj(response, output, length=1024 * 64)


def extract_archive(archive_path: Path, output_directory: Path) -> None:
    """Extract all GTFS files into the project data directory."""
    output_directory.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive_path) as archive:
        archive.extractall(output_directory)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default=DEFAULT_URL, help="TAM GTFS ZIP URL")
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help=f"Extraction directory (default: {DEFAULT_OUTPUT})",
    )
    args = parser.parse_args()

    # Keep the archive outside the project and remove it after extraction.
    with tempfile.TemporaryDirectory(prefix="tramtracker-gtfs-") as temporary_directory:
        archive_path = Path(temporary_directory) / "GTFS.zip"
        print(f"Downloading {args.url}")
        download(args.url, archive_path)

        print(f"Extracting GTFS files to {args.output}")
        extract_archive(archive_path, args.output)

    # stop_times.txt is required by the stop lookup generator.
    stop_times_path = args.output / "stop_times.txt"
    if not stop_times_path.is_file():
        print(
            f"ERROR: stop_times.txt was not found after extraction in {args.output}",
            flush=True,
        )
        raise SystemExit(1)

    print(f"GTFS extraction complete: {stop_times_path}")


if __name__ == "__main__":
    main()
