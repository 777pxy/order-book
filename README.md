# Order Book

A limit order book and matching engine written in C++, built to learn C++ properly and to understand how exchanges work under the hood.

## Versions

The project is built in stages, with each version adding to the last. Later versions are planned and may change as I learn.

| Version | Focus | What it adds | Status |
| --- | --- | --- | --- |
| v1 | Basic order book | One stock, single-threaded. Limit orders, market orders and cancels, matched by price-time priority using standard library containers. Orders read from a file or stdin. | In progress |
| v2 | Performance | Benchmarking, then faster data structures (such as array-based price levels and object pools) and fewer allocations. | Planned |
| v3 | More order types | Multiple stocks, order modification, and IOC and FOK orders. | Planned |
| v4 | Networking | The engine runs as a server, taking orders over TCP with a simple binary protocol and publishing market data. | Planned |
| v5 | Distributed simulation | Trader bots written in Go, matching engines in Docker containers, and graphs of the messages sent between them. | Planned |
| v6 | Derivatives | Futures and options built on top of the matching engine. | Planned |

## AI Usage

All code in this project is handwritten by me. No AI was used to write any of the code.

AI tools were used only for:

- **The spec:** helping draft the design specification for v1.
- **Debugging:** helping me understand compiler errors, crashes and bugs. Fixes were written by me.
- **Learning:** explaining C++ concepts, data structures and how order books and exchanges work.

## Build

```
cmake -B build
cmake --build build
```

## Run

```
./build/orderbook < orders.txt
```

## Test

```
ctest --test-dir build
```