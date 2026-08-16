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

Requires Boost.Asio (`libboost-system-dev` on Debian/Ubuntu) in addition to
PostgreSQL client headers:

```sh
sudo apt install libboost-system-dev libpq-dev
```

```sh
cmake -S . -B build
cmake --build build
```

## Usage

```sh
./build/bin/calculator_hw '{"firstValue":2,"secondValue":3,"operation":"+"}'
./build/bin/calculator_hw '{"firstValue":5,"operation":"!"}'
```

Running with no arguments starts the application in network service mode: it
connects to PostgreSQL, warms up the cache, and listens for calculation
requests over TCP (see "Network protocol" below) until it receives
`SIGINT`/`SIGTERM`. This is the mode used by the systemd unit below.

## Network protocol

When started with no arguments (including under systemd), the application
listens on TCP `0.0.0.0:5555` (see [NetworkConfig.h](include/NetworkConfig.h)).
One connection handles one request at a time (concurrent clients are not
required): send a single line of JSON in the same format as the CLI, receive
a single line of JSON back with `result`/`status` filled in, then the
connection is closed by the server.

```sh
echo '{"firstValue":2,"secondValue":3,"operation":"+"}' | nc localhost 5555
# {"firstValue":2,"operation":"+","result":5,"secondValue":3,"status":0}
```

Networking is implemented with Boost.Asio
([NetworkServer](src/NetworkServer.cpp)), running its `io_context` on the
same worker thread that the CLI's single-task computation would otherwise
use. On shutdown, the acceptor is closed from the signal-handling thread via
[ShutdownCoordinator::onStop](include/ShutdownCoordinator.h); any in-flight
request is allowed to finish, but no new connections are accepted, and the
event loop returns once there is no more pending work.

## Running as a systemd service

The unit file is installed to `/etc/systemd/system/calculator_hw.service` as
part of `cmake --install` (see [packaging/systemd/calculator_hw.service](packaging/systemd/calculator_hw.service)).
It runs the binary with no arguments (network service mode) under a
dedicated `calculator` system user and relies on the graceful `SIGTERM`
handling in
[SignalHandler](src/SignalHandler.cpp)/[ShutdownCoordinator](src/ShutdownCoordinator.cpp)
for clean shutdown (closing the database connection and network socket via
RAII).

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

### Verifying the running service

Confirm the port is open, then send a request and check the response:

```sh
ss -ltnp | grep 5555

echo '{"firstValue":6,"secondValue":7,"operation":"*"}' | nc -q1 localhost 5555
# {"firstValue":6,"operation":"*","result":42,"secondValue":7,"status":0}
```

`telnet` works too for an interactive check (type the JSON line, press Enter,
read the reply, then the server closes the connection):

```sh
telnet localhost 5555
{"firstValue":5,"operation":"!"}
```

## Tests

```sh
ctest --test-dir build
```

This also runs the calculator test suite under Valgrind
(`calculator_tests_valgrind`) if Valgrind is installed.
