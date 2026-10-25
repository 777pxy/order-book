# Order Book

A limit order book and matching engine written in C++, built to learn C++ properly and to understand how exchanges work under the hood.

## Versions

The project is built in stages. Each version lives in its own directory and is a frozen snapshot once finished, so you can compare how the design evolved. Later versions are planned and may change as I learn.

| Version | Focus | What it adds | Status |
| --- | --- | --- | --- |
| [v1](v1/) | Basic order book | One stock, single-threaded. Limit orders, market orders and cancels, matched by price-time priority using standard library containers. Orders read from a file or stdin. | In progress |
| [v2](v2/) | Performance | Benchmarking, then faster data structures (such as array-based price levels and object pools) and fewer allocations. | Planned |
| [v3](v3/) | More order types | Multiple stocks, order modification, and IOC and FOK orders. | Planned |
| [v4](v4/) | Networking | The engine runs as a server, taking orders over TCP with a simple binary protocol and publishing market data. | Planned |
| [v5](v5/) | Distributed simulation | Trader bots written in Go, matching engines in Docker containers, and graphs of the messages sent between them. | Planned |
| [v6](v6/) | Derivatives | Futures and options built on top of the matching engine. | Planned |

## Repository Layout

```
orderbook/
  README.md
  CMakeLists.txt          # top-level, builds every version
  benchmarks/
    orders_1m.txt         # shared input so versions can be compared fairly
  v1/
    README.md             # design notes, decisions, benchmark results
    CMakeLists.txt
    include/orderbook/
    src/
    tests/
  v2/
    ...
```

How versions work:

- **Self-contained.** Each version has its own source, tests and CMake file. Versions never share code, so changing one can't break another.
- **Start from the last one.** A new version begins as a copy of the previous directory, then changes from there.
- **Frozen when done.** Once a version is finished, it only gets bug fixes. A git tag (`v1.0`, `v2.0`, ...) marks each finished version.
- **Own README.** Each version's README covers what changed, why, and its benchmark results on the shared input.
- **Separate binaries.** Each version builds its own executable (`orderbook_v1`, `orderbook_v2`, ...) and test target.

## AI Usage

All code in this project is handwritten by me. No AI was used to write any of the code.

AI tools were used only for:

- **The spec:** helping draft the design specification for v1.
- **Debugging:** helping me understand compiler errors, crashes and bugs. Fixes were written by me.
- **Learning:** explaining C++ concepts, data structures and how order books and exchanges work.

## Build

Build every version from the repository root:

```
cmake -B build
cmake --build build
```

Or build a single version:

```
cmake -S v1 -B build/v1
cmake --build build/v1
```

## Run

```
./build/v1/orderbook_v1 < orders.txt
```

## Test

Run all versions' tests:

```
ctest --test-dir build
```

Or just one version's:

```
ctest --test-dir build/v1
```

## Benchmarks

Every version is run against `benchmarks/orders_1m.txt` so results are directly comparable.

| Version | Orders per second | Notes |
| --- | --- | --- |
| v1 | TBD | |