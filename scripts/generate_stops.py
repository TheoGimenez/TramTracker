#!/usr/bin/env python3
"""Generate a compact stop_id -> stop_name lookup for the ESP32.

The stop IDs come from stop_times.txt and their names come from stops.txt.
The generated C++ table is sorted by stop_id.
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path


DEFAULT_INPUT = Path("data/gtfs/stop_times.txt")
DEFAULT_STOPS = Path("data/gtfs/stops.txt")
DEFAULT_OUTPUT = Path("include/generated_stops.h")


def read_stop_ids(input_path: Path) -> set[int]:
    """Read the numeric stop IDs used by stop_times.txt."""
    stop_ids: set[int] = set()

    with input_path.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required_columns = {"stop_id"}
        missing_columns = required_columns - set(reader.fieldnames or ())
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV columns: {missing}")

        for line_number, row in enumerate(reader, start=2):
            raw_stop_id = (row["stop_id"] or "").strip()

            if not raw_stop_id:
                continue

            try:
                stop_id = int(raw_stop_id)
            except ValueError as error:
                raise ValueError(
                    f"Invalid numeric stop_id {raw_stop_id!r} at line {line_number}"
                ) from error

            if not 0 <= stop_id <= 0xFFFFFFFF:
                raise ValueError(
                    f"stop_id {stop_id} at line {line_number} does not fit uint32_t"
                )

            stop_ids.add(stop_id)

    return stop_ids


def read_stop_names(input_path: Path, stop_ids: set[int]) -> dict[int, str]:
    """Read names from stops.txt for the IDs used by stop_times.txt."""
    stops: dict[int, str] = {}

    with input_path.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required_columns = {"stop_id", "stop_name"}
        missing_columns = required_columns - set(reader.fieldnames or ())
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV columns: {missing}")

        for line_number, row in enumerate(reader, start=2):
            raw_stop_id = (row["stop_id"] or "").strip()
            stop_name = (row["stop_name"] or "").strip()
            if not raw_stop_id or not stop_name:
                continue

            try:
                stop_id = int(raw_stop_id)
            except ValueError as error:
                raise ValueError(
                    f"Invalid numeric stop_id {raw_stop_id!r} at line {line_number}"
                ) from error

            if stop_id in stop_ids:
                stops[stop_id] = stop_name

    missing_ids = stop_ids - stops.keys()
    if missing_ids:
        print(f"Warning: {len(missing_ids)} stop IDs have no entry in stops.txt")

    return stops


def escape_cpp_string(value: str) -> str:
    """Escape a UTF-8 string for a C++ string literal."""
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
    )


def generate_header(stops: dict[int, str], output_path: Path, input_path: Path) -> None:
    """Write the compact flash-resident lookup header."""
    # Store each stop name once and reference it by a 16-bit offset.
    names: list[str] = []
    offsets: dict[str, int] = {}

    for stop_name in stops.values():
        if stop_name not in offsets:
            offsets[stop_name] = sum(len(name.encode("utf-8")) + 1 for name in names)
            names.append(stop_name)

    entries = [
        (stop_id, offsets[stop_name])
        for stop_id in sorted(stops)
        for stop_name in [stops[stop_id]]
    ]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("#pragma once\n\n")
        output.write("#include <stddef.h>\n")
        output.write("#include <stdint.h>\n\n")
        output.write("namespace gtfs {\n\n")
        output.write("struct Stop {\n")
        output.write("    uint32_t id;\n")
        output.write("    uint16_t name_offset;\n")
        output.write("};\n\n")
        output.write("inline constexpr char const kStopNames[] =\n")
        for name in names:
            output.write(f'    "{escape_cpp_string(name)}\\0"\n')
        output.write("    ;\n\n")
        output.write("inline constexpr Stop kStops[] = {\n")
        for stop_id, offset in entries:
            output.write(f"    {{{stop_id}, {offset}}},\n")
        output.write("};\n\n")
        output.write(
            "inline constexpr size_t kStopCount = "
            "sizeof(kStops) / sizeof(kStops[0]);\n\n"
        )
        output.write("}  // namespace gtfs\n")

    print(f"Generated {output_path}")
    print(f"Input: {input_path}")
    print(f"Unique stop IDs: {len(stops)}")
    print(f"Unique stop_id/name pairs: {len(entries)}")
    print(f"Unique stop name strings: {len(names)}")
    print(f"String pool bytes: {sum(len(name.encode('utf-8')) + 1 for name in names)}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("stops", nargs="?", type=Path, default=DEFAULT_STOPS)
    parser.add_argument("output", nargs="?", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    stop_ids = read_stop_ids(args.input)
    stops = read_stop_names(args.stops, stop_ids)
    generate_header(stops, args.output, args.input)


if __name__ == "__main__":
    main()
