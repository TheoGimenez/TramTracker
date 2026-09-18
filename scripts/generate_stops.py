#!/usr/bin/env python3
"""Generate a compact stop_id -> stop_headsign lookup for the ESP32.

The source file contains one row per trip and stop. A stop_id can have
multiple stop_headsign values, so this generator stores every distinct pair.
The generated C++ table is sorted by stop_id, then by headsign offset.
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path


DEFAULT_INPUT = Path("config/gtfs/stop_times.txt")
DEFAULT_OUTPUT = Path("src/generated_stops.h")


def read_stop_headings(input_path: Path) -> dict[int, set[str]]:
    """Read distinct headsigns grouped by numeric stop_id."""
    stops: dict[int, set[str]] = defaultdict(set)

    with input_path.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required_columns = {"stop_id", "stop_headsign"}
        missing_columns = required_columns - set(reader.fieldnames or ())
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV columns: {missing}")

        # Ignore duplicate rows while preserving multiple destinations per stop.
        for line_number, row in enumerate(reader, start=2):
            raw_stop_id = (row["stop_id"] or "").strip()
            headsign = (row["stop_headsign"] or "").strip()

            if not raw_stop_id or not headsign:
                continue

            try:
                stop_id = int(raw_stop_id)
            except ValueError as error:
                raise ValueError(
                    f"Invalid numeric stop_id {raw_stop_id!r} at line {line_number}"
                ) from error

            if not 0 <= stop_id <= 0xFFFF:
                raise ValueError(
                    f"stop_id {stop_id} at line {line_number} does not fit uint16_t"
                )

            stops[stop_id].add(headsign)

    return stops


def escape_cpp_string(value: str) -> str:
    """Escape a UTF-8 string for a C++ string literal."""
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
    )


def generate_header(stops: dict[int, set[str]], output_path: Path, input_path: Path) -> None:
    """Write the compact flash-resident lookup header."""
    # Store each headsign once and reference it by a 16-bit offset.
    names: list[str] = []
    offsets: dict[str, int] = {}

    for stop_id in sorted(stops):
        for headsign in sorted(stops[stop_id]):
            if headsign not in offsets:
                offsets[headsign] = sum(len(name.encode("utf-8")) + 1 for name in names)
                names.append(headsign)

    entries = [
        (stop_id, offsets[headsign])
        for stop_id in sorted(stops)
        for headsign in sorted(stops[stop_id])
    ]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("#pragma once\n\n")
        output.write("#include <stddef.h>\n")
        output.write("#include <stdint.h>\n\n")
        output.write("namespace gtfs {\n\n")
        output.write("struct Stop {\n")
        output.write("    uint16_t id;\n")
        output.write("    uint16_t name_offset;\n")
        output.write("};\n\n")
        output.write("inline constexpr char kStopNames[] =\n")
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
    print(f"Unique stop_id/headsign pairs: {len(entries)}")
    print(f"Unique headsign strings: {len(names)}")
    print(f"String pool bytes: {sum(len(name.encode('utf-8')) + 1 for name in names)}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("output", nargs="?", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    stops = read_stop_headings(args.input)
    generate_header(stops, args.output, args.input)


if __name__ == "__main__":
    main()
