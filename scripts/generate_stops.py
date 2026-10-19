#!/usr/bin/env python3
"""Generate compact TAM stop, route-name, and stop-route lookup tables."""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path


DEFAULT_INPUT = Path("data/gtfs/stop_times.txt")
DEFAULT_STOPS = Path("data/gtfs/stops.txt")
DEFAULT_TRIPS = Path("data/gtfs/trips.txt")
DEFAULT_ROUTES = Path("data/gtfs/routes.txt")
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
            if stop_id == 0:
                raise ValueError("stop_id 0 is reserved for the unknown stop")

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


def read_route_names(input_path: Path) -> dict[str, str]:
    """Read TAM route IDs and verify they match the user-facing short names."""
    route_names: dict[str, str] = {}

    with input_path.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required_columns = {"route_id", "route_short_name"}
        missing_columns = required_columns - set(reader.fieldnames or ())
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV columns: {missing}")

        for line_number, row in enumerate(reader, start=2):
            route_id = (row["route_id"] or "").strip()
            short_name = (row["route_short_name"] or "").strip()
            if not route_id or not short_name:
                continue
            if route_id != short_name:
                raise ValueError(
                    f"TAM route_id {route_id!r} differs from route_short_name "
                    f"{short_name!r} at line {line_number}"
                )
            route_names[route_id] = short_name

    return route_names


def read_stop_routes(
    input_path: Path,
    trips_path: Path,
    route_names: dict[str, str],
    stop_ids: set[int],
) -> set[tuple[int, str, int]]:
    """Join stop times to TAM route and direction metadata."""
    trip_routes: dict[str, tuple[str, int]] = {}
    with trips_path.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required_columns = {"trip_id", "route_id", "direction_id"}
        missing_columns = required_columns - set(reader.fieldnames or ())
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV columns: {missing}")

        for line_number, row in enumerate(reader, start=2):
            trip_id = (row["trip_id"] or "").strip()
            route_id = (row["route_id"] or "").strip()
            raw_direction_id = (row["direction_id"] or "").strip()
            if not trip_id or route_id not in route_names:
                continue
            try:
                direction_id = int(raw_direction_id)
            except ValueError as error:
                raise ValueError(
                    f"Invalid direction_id {raw_direction_id!r} at trips.txt line {line_number}"
                ) from error
            if direction_id not in (0, 1):
                raise ValueError(
                    f"TAM direction_id must be 0 or 1 at trips.txt line {line_number}"
                )
            trip_routes[trip_id] = (route_id, direction_id)

    stop_routes: set[tuple[int, str, int]] = set()
    with input_path.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        required_columns = {"trip_id", "stop_id"}
        missing_columns = required_columns - set(reader.fieldnames or ())
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise ValueError(f"Missing required CSV columns: {missing}")

        for line_number, row in enumerate(reader, start=2):
            raw_stop_id = (row["stop_id"] or "").strip()
            trip = trip_routes.get((row["trip_id"] or "").strip())
            if not raw_stop_id or trip is None:
                continue
            try:
                stop_id = int(raw_stop_id)
            except ValueError as error:
                raise ValueError(
                    f"Invalid numeric stop_id {raw_stop_id!r} at stop_times.txt line {line_number}"
                ) from error
            if stop_id in stop_ids:
                route_id, direction_id = trip
                stop_routes.add((stop_id, route_id, direction_id))

    return stop_routes


def generate_header(
    stops: dict[int, str],
    stop_routes: set[tuple[int, str, int]],
    route_names: dict[str, str],
    output_path: Path,
    input_path: Path,
) -> None:
    """Write compact flash-resident stop and many-to-many route lookup tables."""
    # Store each stop name once and reference it by a 16-bit offset.
    names: list[str] = ["unknown"]
    offsets: dict[str, int] = {"unknown": 0}

    for stop_name in stops.values():
        if stop_name not in offsets:
            offsets[stop_name] = sum(len(name.encode("utf-8")) + 1 for name in names)
            names.append(stop_name)

    entries = [(0, 0)] + [
        (stop_id, offsets[stop_name])
        for stop_id in sorted(stops)
        for stop_name in [stops[stop_id]]
    ]
    stop_indexes = {stop_id: index for index, (stop_id, _) in enumerate(entries)}

    ordered_routes = sorted(
        route_names,
        key=lambda route_name: (not route_name.isdigit(), int(route_name) if route_name.isdigit() else route_name),
    )
    route_offsets: dict[str, int] = {}
    route_pool_bytes = len("unknown".encode("utf-8")) + 1
    for route_name in ordered_routes:
        route_offsets[route_name] = route_pool_bytes
        route_pool_bytes += len(route_names[route_name].encode("utf-8")) + 1
    route_indexes = {
        route_name: index + 1 for index, route_name in enumerate(ordered_routes)
    }
    stop_route_entries = sorted(
        (stop_indexes[stop_id], route_indexes[route_id], direction_id)
        for stop_id, route_id, direction_id in stop_routes
        if stop_id in stop_indexes and route_id in route_indexes
    )

    if len(entries) > 0xFFFF or len(ordered_routes) + 1 > 0xFF:
        raise ValueError("Generated stop or route table exceeds its compact index capacity")
    if sum(len(name.encode("utf-8")) + 1 for name in names) > 0xFFFF:
        raise ValueError("Stop-name pool exceeds uint16_t offset capacity")
    if route_pool_bytes > 0xFFFF:
        raise ValueError("Route-name pool exceeds uint16_t offset capacity")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("#pragma once\n\n")
        output.write("#include <cstddef>\n")
        output.write("#include <cstdint>\n\n")
        output.write("namespace gtfs {\n\n")
        output.write("// One unique numeric stop and its offset in the shared stop-name pool.\n")
        output.write("struct Stop {\n")
        output.write("    uint32_t    id;          // Numeric stop_id used by the TAM feed.\n")
        output.write("    uint16_t    name_offset; // Byte offset into k_stop_names.\n")
        output.write("    const char* name() const;// Ptr from packet k_stop_names string\n")
        output.write("};\n\n")
        output.write("// One dense route-table entry; index 0 is reserved for unknown.\n")
        output.write("struct Route {\n")
        output.write("    uint16_t name_offset; // Byte offset into k_route_names.\n")
        output.write("    const char* name() const;\n")
        output.write("};\n\n")
        output.write("// Many-to-many link from a stop to a route and one TAM travel direction.\n")
        output.write("struct StopRoute {\n")
        output.write("    uint16_t stop_index;  // Index into k_stops.\n")
        output.write("    uint8_t  route_index; // Index into k_routes.\n")
        output.write("    uint8_t  direction_id;// TAM direction slot: 0 or 1.\n")
        output.write("};\n\n")
        output.write("// Deduplicated UTF-8 stop names; each string is NUL-terminated.\n")
        output.write("inline constexpr char const k_stop_names[] =\n")
        for name in names:
            output.write(f'    "{escape_cpp_string(name)}\\0"\n')
        output.write("    ;\n\n")
        output.write("inline const char* Stop::name() const {\n")
        output.write("    return k_stop_names + name_offset;\n")
        output.write("}\n\n")
        output.write("// Deduplicated TAM route names shared by display and realtime lookup.\n")
        output.write("inline constexpr char const k_route_names[] =\n")
        output.write('    "unknown\\0"\n')
        for route_name in ordered_routes:
            output.write(f'    "{escape_cpp_string(route_names[route_name])}\\0"\n')
        output.write("    ;\n\n")
        output.write("inline const char* Route::name() const {\n")
        output.write("    return k_route_names + name_offset;\n")
        output.write("}\n\n")
        output.write("// Route records point into k_route_names; each route is stored once.\n")
        output.write("inline constexpr Route k_routes[] = {\n")
        output.write("    {0}, // Unknown route; route index 0 is reserved.\n")
        for route_name in ordered_routes:
            output.write(f"    {{{route_offsets[route_name]}}},\n")
        output.write("};\n\n")
        output.write("// Stops are sorted by numeric ID so existing binary-search lookups remain valid.\n")
        output.write("inline constexpr Stop k_stops[] = {\n")
        output.write("    {0, 0}, // Unknown stop; stop ID 0 is reserved.\n")
        for stop_id, offset in entries[1:]:
            output.write(f"    {{{stop_id}, {offset}}},\n")
        output.write("};\n\n")
        output.write("// Route memberships preserve every route and direction serving each stop.\n")
        output.write("inline constexpr StopRoute k_stop_routes[] = {\n")
        for stop_index, route_index, direction_id in stop_route_entries:
            output.write(f"    {{{stop_index}, {route_index}, {direction_id}}},\n")
        output.write("};\n\n")
        output.write("// Number of unique stops stored in k_stops.\n")
        output.write(
            "inline constexpr size_t k_stop_count = "
            "sizeof(k_stops) / sizeof(k_stops[0]);\n\n"
        )
        output.write("// Number of unique routes stored in k_routes.\n")
        output.write(
            "inline constexpr size_t k_route_count = "
            "sizeof(k_routes) / sizeof(k_routes[0]);\n\n"
        )
        output.write("// Number of stop-route-direction links stored in k_stop_routes.\n")
        output.write(
            "inline constexpr size_t k_stop_route_count = "
            "sizeof(k_stop_routes) / sizeof(k_stop_routes[0]);\n\n"
        )
        output.write("}  // namespace gtfs\n")

    print(f"Generated {output_path}")
    print(f"Input: {input_path}")
    print(f"Unique stop IDs: {len(stops)}")
    print(f"Unique stop_id/name pairs: {len(entries)}")
    print(f"Unique stop name strings: {len(names)}")
    print(f"Unique TAM route names: {len(ordered_routes)}")
    print(f"Unique route-stop-direction associations: {len(stop_route_entries)}")
    print(f"String pool bytes: {sum(len(name.encode('utf-8')) + 1 for name in names)}")
    print(f"Route-name pool bytes: {route_pool_bytes}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("stops", nargs="?", type=Path, default=DEFAULT_STOPS)
    parser.add_argument("output", nargs="?", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--trips", type=Path, default=DEFAULT_TRIPS)
    parser.add_argument("--routes", type=Path, default=DEFAULT_ROUTES)
    args = parser.parse_args()

    stop_ids = read_stop_ids(args.input)
    stops = read_stop_names(args.stops, stop_ids)
    route_names = read_route_names(args.routes)
    stop_routes = read_stop_routes(args.input, args.trips, route_names, stop_ids)
    generate_header(stops, stop_routes, route_names, args.output, args.input)


if __name__ == "__main__":
    main()
