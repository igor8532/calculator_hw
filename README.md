# calculator_hw

A console calculator that performs basic mathematical operations, with
results persisted in PostgreSQL and served from an in-memory cache on
repeat requests.

## Database setup

The application expects a PostgreSQL database and user matching
[include/DataBaseConfig.h](include/DataBaseConfig.h) (defaults:
`calculator`/`calculator`@`localhost:5432`). Create the schema with:

```sh
psql -h localhost -U calculator -d calculator -f sql/schema.sql
```

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Usage

```sh
./build/bin/calculator_hw '{"firstValue":2,"secondValue":3,"operation":"+"}'
./build/bin/calculator_hw '{"firstValue":5,"operation":"!"}'
```

Running with no arguments starts the application in idle service mode: it
connects to PostgreSQL, warms up the cache, and waits for `SIGINT`/`SIGTERM`
without performing a calculation. This is the mode used by the systemd unit
below.

## Running as a systemd service

The unit file is installed to `/etc/systemd/system/calculator_hw.service` as
part of `cmake --install` (see [packaging/systemd/calculator_hw.service](packaging/systemd/calculator_hw.service)).
It runs the binary with no arguments (idle service mode) under a dedicated
`calculator` system user and relies on the graceful `SIGTERM` handling in
[SignalHandler](src/SignalHandler.cpp)/[ShutdownCoordinator](src/ShutdownCoordinator.cpp)
for clean shutdown (closing the database connection via RAII).

```sh
# one-time setup
sudo useradd --system --no-create-home --shell /usr/sbin/nologin calculator
sudo cmake --install build
sudo systemctl daemon-reload

# service management
sudo systemctl start calculator_hw
sudo systemctl status calculator_hw
sudo journalctl -u calculator_hw -f
sudo systemctl restart calculator_hw
sudo systemctl stop calculator_hw

# optional: start on boot
sudo systemctl enable calculator_hw
```

## Tests

```sh
ctest --test-dir build
```

This also runs the calculator test suite under Valgrind
(`calculator_tests_valgrind`) if Valgrind is installed.
