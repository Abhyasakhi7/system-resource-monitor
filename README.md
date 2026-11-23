# System Resource Monitor

A real-time Linux system monitor and all-in-one utility built with C++20, Qt 6 Widgets, Qt Charts, and SQLite.

## Features

### Implemented (v2 MVP)

- Real-time CPU, memory, disk, and network monitoring from `/proc` and `/sys`
- Temperature sensors from thermal zones and hwmon, plus battery capacity/status from sysfs
- Dark/light dashboard with metric cards and live charts (60 samples)
- Detailed monitoring page with CPU, memory, disk, network, temperature, and battery history
- Process manager: search, sort, detail view, SIGTERM/SIGKILL with confirmation and
  PID-reuse protection
- Hardware information: processor, system, memory, graphics, storage, motherboard, and
  battery with availability status and data source
- SQLite performance history with configurable retention (default 7 days)
- Analytics page: 1h/6h/12h/24h/7d/custom ranges, min/max/avg statistics, CSV/JSON export
- Settings stored in SQLite (`app_settings`), collector enable/disable (including temperature
  and battery), update interval, theme, and network speed unit
- Background worker threads for monitoring, process scanning, and database access

### Planned

- GPU utilization/VRAM details, system management (systemd/startup), benchmark,
  smart alerts, and PDF reporting

## Requirements

Tested on Ubuntu/Debian-based systems:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git \
    qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
    libqt6charts6-dev libgtest-dev libsqlite3-dev
```

- C++20 compiler (GCC or Clang)
- CMake >= 3.21
- Qt 6 Widgets and Charts
- GoogleTest
- SQLite 3
- Linux `/proc` and `/sys`

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Run

```bash
./build/system-resource-monitor
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

The tests cover `/proc` parsers, metric math, hardware parsers, process stat parsing,
temperature/battery parsers and sysfs discovery, SQLite schema/queries/retention/export,
and settings round-trips.

## Database

The database is created at:

```
~/.local/share/<organization>/<application>/metrics.sqlite
```

Tables:

- `metric_samples` — timestamped CPU, memory, disk, and network samples
- `app_settings` — key/value application preferences

History retention defaults to 7 days and is configurable in Settings → History.

## Project Layout

```
include/
  core/       metrics model, ring buffer, collector, worker
  monitors/   /proc and /sys parsers and collectors
  database/   SQLite repository and asynchronous database manager
  hardware/   hardware information model and collector
  process/    process parsing, service, and worker
  services/   config, history, and application service container
  ui/         main window, dashboard, pages, charts, settings
src/          implementations
tests/        GoogleTest unit tests
```
