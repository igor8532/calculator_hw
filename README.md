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

## Tests

```sh
ctest --test-dir build
```

This also runs the calculator test suite under Valgrind
(`calculator_tests_valgrind`) if Valgrind is installed.
