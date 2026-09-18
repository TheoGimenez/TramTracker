# TramTracker

ESP32 firmware that displays the next arrival time for a selected TAM bus or
tram stop on an LED matrix. The stop is selected through the embedded web
server and realtime data is fetched from the TAM GTFS-Realtime API.

## Requirements

- VS Code with PlatformIO, or PlatformIO CLI
- An ESP32 Dev Module
- Python 3 for the project helper scripts
- Network access during the first build to download the static TAM GTFS feed

PlatformIO installs the C/C++ dependencies declared in `platformio.ini`,
including Nanopb, AsyncTCP, ESPAsyncWebServer, MD_MAX72XX and MD_Parola.

## Project setup

Create the local configuration file from the template:

```bash
cp .env.example .env
```

Then set the Wi-Fi credentials and application values in `.env`. This file is
ignored by Git and must not be committed.

Build the debug firmware:

```bash
pio run -e debug
```

On the first build, the PlatformIO pre-scripts automatically:

1. Generate `include/generated_app_config.h` from `.env`.
2. Generate Nanopb sources from `data/proto/*.proto` when outputs are missing.
3. Download and extract the TAM static GTFS archive into `data/gtfs/` when
	`data/gtfs/stop_times.txt` is missing.
4. Generate `include/generated_stops.h` from `stop_times.txt` and `stops.txt`
	when it is missing.

The generated files and downloaded GTFS data are ignored by Git.

## Build environments

```bash
pio run -e debug
pio run -e release
pio run -t upload -e debug
pio device monitor
```

The debug environment enables `TRAM_DEBUG` and includes debug logs. The
release environment uses optimized release settings. Serial debug messages
should be guarded by the logging macros in `src/utils.h`.

## Directory layout

```text
data/gtfs/          Downloaded TAM static GTFS files
data/proto/         Protocol Buffer schemas used by Nanopb
include/             Project and generated application headers
lib/gtfs_rt_pb/     Generated Nanopb C sources and headers
scripts/             PlatformIO pre-scripts and data generators
src/                 Firmware source code
```

## Manual scripts

The scripts can also be run independently from the project root:

```bash
python3 scripts/fetch_tram_gtfs.py
python3 scripts/generate_stops.py
python3 scripts/generate_nanopb.py
python3 scripts/generate_app_config.py
```

`generate_nanopb.py` uses the Nanopb package installed by PlatformIO. If its
Python dependency is unavailable when launched with the system Python, it
prints a warning; PlatformIO provides the intended build integration.

## GTFS-Realtime

The realtime endpoints are configured in `.env` and generated into
`include/generated_app_config.h`. The current TAM endpoints are public HTTPS
GTFS-Realtime feeds and do not require an API key. TLS certificate validation
should be enabled for production deployments instead of relying on
`setInsecure()`.
