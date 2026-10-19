#!/usr/bin/env python3
"""Generate the application configuration header from the project .env file."""
from __future__ import annotations

import os
from pathlib import Path

# Resolve the project root in both PlatformIO and standalone execution.
ROOT = Path(
    os.environ.get("PROJECT_DIR")
    or os.environ.get("PLATFORMIO_PROJECT_DIR")
    or os.getcwd()
).resolve()
if not ROOT.exists():
    ROOT = Path(".").resolve()
ENV_PATH = ROOT / ".env"
OUT_PATH = ROOT / "include" / "generated_app_config.h"


def parse_env(path: Path) -> dict[str, str]:
    """Read simple KEY=VALUE entries from the environment file."""
    values: dict[str, str] = {}
    if not path.exists():
        raise FileNotFoundError(f"Missing {path}. Copy .env.example to .env and fill the values.")

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = [part.strip() for part in line.split("=", 1)]
        values[key] = value
    return values


def render_header(values: dict[str, str]) -> str:
    """Render configuration values as C++ constants."""
    wifi_ssid = values.get("WIFI_SSID", "")
    wifi_password = values.get("WIFI_PASSWORD", "")
    webserver_port = values.get("WEBSERVER_PORT", "80")
    gtfs_base_url = values.get("GTFS_BASE_URL", "")
    gtfs_trip_update = values.get("GTFS_TRIP_UPDATE_ENDPOINT", "/").replace("\\/", "/")
    gtfs_vehicle_position = values.get("GTFS_VEHICLE_POSITION_ENDPOINT", "/").replace("\\/", "/")
    gtfs_alert = values.get("GTFS_ALERT_ENDPOINT", "/").replace("\\/", "/")
    gtfs_port = values.get("GTFS_PORT", "443")

    lines = [
        "#pragma once",
        "",
        "#include <stdint.h>",
        "",
        "namespace env {",
        f"constexpr const char* k_wifi_ssid = \"{wifi_ssid}\";",
        f"constexpr const char* k_wifi_password = \"{wifi_password}\";",
        f"constexpr uint16_t    k_web_server_port = {int(webserver_port)};",
        f"constexpr const char* k_gtfs_base_url = \"{gtfs_base_url}\";",
        f"constexpr const char* k_gtfs_trip_update_endpoint = \"{gtfs_trip_update}\";",
        f"constexpr const char* k_gtfs_alert_endpoint = \"{gtfs_alert}\";",
        f"constexpr uint16_t    k_gtfs_port = {int(gtfs_port)};",
        "}",
        "",
    ]
    return "\n".join(lines) + "\n"


def main() -> None:
    values = parse_env(ENV_PATH)
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUT_PATH.write_text(render_header(values), encoding="utf-8")
    print(f"Generated {OUT_PATH}")


if __name__ == "__main__":
    main()
