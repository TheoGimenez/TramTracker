# TramTracker

TramTracker is an ESP32 firmware project intended to show upcoming TAM public
transport arrivals on an LED matrix. The long-term workflow is to select any
TAM route and one of its stops in an embedded web interface, then display a
countdown to the next arrivals for each direction using TAM GTFS-Realtime data.
The countdown is intended to use Europe/Paris local time and an internet time
service for clock synchronization.

## Project status

This project is under active development. The firmware currently initializes
the display, connects to Wi-Fi in station mode, and starts an embedded web
server. The page currently offers a hard-coded list of tram lines; selecting a
line changes the display text. Stop selection and the regular fetch-and-display
arrival workflow are not yet integrated. The GTFS-Realtime fetch and streaming
Nanopb decoding code is being developed, with arrival correctness as the first
priority.

Planned work, in priority order:

1. Fetch and decode coherent realtime arrivals, matching route, stop, and
	direction correctly.
2. Populate the stop selector dynamically from the selected route and update
	the selected stop, fetch behavior, and display.
3. Add Wi-Fi access-point onboarding so a phone can configure home Wi-Fi.
4. In the longer term, consider migrating from Arduino to the Espressif
	framework, then from PlatformIO to a CMake-based build.

The last two items are future goals, not current build requirements.

## Hardware

- Development board: Diymore ESP32 NodeMCU-style board with USB-C and a CH340
	USB-to-serial chip. PlatformIO currently targets the `esp32dev` board
	definition. Confirm the exact ESP32 module/chip revision before making
	hardware-specific changes or changing the board configuration.
- The current display is one chain of four 8x8 1088AS LED matrix modules.
  The owner may add two modules later; that extension is not assumed by the
  current firmware configuration.
- `src/DisplayController.h` configures FC16-compatible modules, with data on
  GPIO 23, clock on GPIO 18, and chip select on GPIO 5.

Connect the matrix module's DIN, CLK, and CS/LOAD pins to the corresponding
GPIOs above, and connect its power and ground to a supply appropriate for the
module. Verify the display module pin labels, voltage requirements, and board
pinout before wiring; do not infer GPIO numbers from physical header positions.
The current firmware does not configure an RTC module or other external
peripherals.

## Requirements

- VS Code with the PlatformIO extension, or PlatformIO Core CLI (`pio`)
- Python 3 for project helper scripts
- Network access on the first build to download the static TAM GTFS feed
- `protoc` and the Python `protobuf` package when Nanopb sources need to be
  regenerated; the Nanopb generator itself is installed by PlatformIO

PlatformIO installs the C/C++ dependencies declared in `platformio.ini`,
including Nanopb, AsyncTCP, ESPAsyncWebServer, MD_MAX72XX, and MD_Parola.

## Configuration and build

Create the local configuration file from the template:

```bash
cp .env.example .env
```

Set `WIFI_SSID`, `WIFI_PASSWORD`, the TAM feed host/endpoints/port, and
`WEBSERVER_PORT` in `.env`. Replace all template values before building. `.env`
is ignored by Git; do not commit real credentials. The pre-build script
generates `include/generated_app_config.h` from this file. Display hardware
settings such as module count and GPIO pins are currently configured in
`src/DisplayController.h`; changing them requires rebuilding the firmware.

Build the firmware:

```bash
pio run -e debug
pio run -e release
```

The `debug` environment enables `TRAM_DEBUG`; serial logging should use the
macros in `include/utils.h`. The `release` environment uses optimized release
settings. To upload to a connected board and open its serial monitor:

```bash
pio run -t upload -e debug
pio device monitor
```

The serial monitor is configured for 115200 baud. Uploading and validating the
display/network behavior requires the physical ESP32 and attached hardware.

## Build-generated data

The PlatformIO pre-build scripts perform these steps:

1. Generate `include/generated_app_config.h` from `.env`.
2. Generate Nanopb C sources when the schema/generator fingerprint changes or
	an expected output is missing.
3. Download and extract the static TAM GTFS archive into `data/gtfs/` when
	`data/gtfs/stop_times.txt` is missing.
4. Generate `include/generated_stops.h` when it is missing. The lookup is
	derived from `stop_times.txt`, `stops.txt`, `trips.txt`, and `routes.txt`;
	it stores unique stops, deduplicated route names, and stop-route-direction
	associations.

The stop lookup is only regenerated automatically when its output is missing.
After changing or refreshing the static GTFS files, run
`python3 scripts/generate_stops.py` to rebuild it. Generated headers, generated
Nanopb sources, the local Nanopb fingerprint, and downloaded static GTFS files
are ignored by Git; edit their source schemas or generator scripts instead of
hand-editing generated output.

## GTFS data and memory

Static TAM GTFS tables are stored in `data/gtfs/`; Protocol Buffer schemas are
in `data/proto/`. The stop lookup generator joins stop times to trips, routes,
and stop names. It expects numeric stop IDs and verifies that TAM route IDs
match their displayed short names. The intended selector includes all routes
represented in the TAM feed, rather than only the tram lines shown by the
current hard-coded web page.

Two physical stops can share a name while representing opposite directions;
some route-stop combinations occur only during diversions. Use the generated
route/stop/direction association and realtime feed IDs rather than matching on
display names alone. Some stops have only one direction for a given route.

Realtime feeds can exceed 299 KiB, while ESP32 RAM is limited. The decoder uses
Nanopb callbacks to process feed entities as a stream; avoid buffering the
whole response in memory. The public TAM endpoint does not require an API key.
The current HTTPS client disables certificate verification; this weakens
server authentication and should be revisited before treating the connection
as production-secure.

The intended arrival display is a countdown in the `Europe/Paris` timezone.
The ESP32 must first synchronize its clock over the internet (for example via
NTP); Paris alternates between UTC+1 and UTC+2, so a fixed GMT+1 offset is not
correct year-round. Clock synchronization and timezone-aware arrival rendering
are not yet integrated into the normal display workflow.

The planned Wi-Fi onboarding mode would temporarily expose an access point for
initial setup. `admin` / `admin` has been discussed only as a temporary default,
not as a secure credential; onboarding is not currently implemented.

## Manual scripts

Run these from the project root when needed:

```bash
python3 scripts/fetch_tram_gtfs.py
python3 scripts/generate_stops.py
python3 scripts/generate_nanopb.py
python3 scripts/generate_app_config.py
```

`fetch_tram_gtfs.py` downloads and extracts the static feed. `generate_stops.py`
rebuilds the embedded route/stop lookup. `generate_nanopb.py` regenerates code
from the schemas and requires the PlatformIO Nanopb generator, `protoc`, and
Python protobuf tooling. `generate_app_config.py` requires a configured `.env`.
The PlatformIO pre-build integration is the normal build path.

## Repository layout

```text
.github/             Copilot workspace instructions
data/gtfs/           Downloaded static TAM GTFS tables
data/proto/          GTFS-Realtime Protocol Buffer schemas
include/             Project headers and generated lookup/configuration
lib/gtfs_rt_pb/      Generated Nanopb C sources and headers
scripts/             PlatformIO pre-scripts and data generators
src/                 ESP32 firmware source code
test/                PlatformIO test directory (no project tests yet)
```

There is currently no automated test suite. Use the debug/release builds for
compile validation and test runtime behavior on the physical device.
