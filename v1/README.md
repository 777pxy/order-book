# Order Book v1 Spec

Oct 25, 2026 · @Praj

## Goal and scope

v1 is a single-threaded C++ program that keeps a limit order book for one stock, reads orders from a file or stdin, matches them by price-time priority, and prints the trades.

- One symbol only, so there is no symbol field anywhere.
- Order types: limit and market. Plus cancel.
- One process, one thread, no networking, no persistence.
- Language: C++20, built with CMake, tested with GoogleTest.

## Core types

Use plain structs and fixed-width integers. Prices are integer ticks, never floats: 100.50 is stored as 10050.

| Type | Fields | Notes |
| --- | --- | --- |
| `Side` | `Buy`, `Sell` | `enum class` |
| `OrderType` | `Limit`, `Market` | `enum class` |
| `Order` | `id` (uint64), `side`, `type`, `price` (int64 ticks), `qty` (uint32), `seq` (uint64) | `seq` is a counter that goes up by one per order, used for time priority |
| `Trade` | `buyId`, `sellId`, `price`, `qty` | Price is always the resting order's price |

Use `using Price = std::int64_t;` style aliases so the types read clearly.

## Data structures

The book is two sorted maps of price levels plus an index for cancels. Standard library only, no custom containers yet.

| Member | Type | Why |
| --- | --- | --- |
| `bids_` | `std::map<Price, std::list<Order>, std::greater<Price>>` | Best bid (highest) is `begin()` |
| `asks_` | `std::map<Price, std::list<Order>>` | Best ask (lowest) is `begin()` |
| `index_` | `std::unordered_map<OrderId, Locator>` | O(1) lookup for cancels |
| `nextSeq_` | `std::uint64_t` | Stamps each incoming order |

`Locator` holds the side, the price, and a `std::list<Order>::iterator`. A `std::list` is used per level because its iterators stay valid when other orders are removed, so a cancel can erase the order directly.

Public interface of `OrderBook`:

- `std::vector<Trade> addOrder(Order o)`: matches, then rests any leftover (limit only).
- `bool cancel(OrderId id)`: returns false if the ID is unknown.
- `std::optional<Price> bestBid() const` and `bestAsk() const`.
- `std::size_t size() const`: number of resting orders.

## Input format

One command per line, space separated. Prices are given in ticks so the parser never touches decimals.

```
LIMIT <id> <BUY|SELL> <price> <qty>
MARKET <id> <BUY|SELL> <qty>
CANCEL <id>
PRINT
```

Example session:

```
LIMIT 1 SELL 10050 10
LIMIT 2 SELL 10060 5
LIMIT 3 BUY 10055 12
PRINT
```

Bad lines (unknown command, missing field, duplicate ID, zero qty) print `ERROR <line number> <reason>` and are skipped. The program never crashes on input.

## Matching rules

Price-time priority: best price first, and at the same price, the lowest `seq` first. An incoming order always trades at the resting order's price.

For an incoming order:

1. Pick the opposite side (a buy matches against asks, a sell against bids).
2. While the incoming order has quantity left and the opposite side is not empty:
   1. Look at the best level and the first order in its queue.
   2. Limit order only: stop if the price does not cross (buy price below best ask, or sell price above best bid).
   3. Trade `min(incoming qty, resting qty)` at the resting price and record a `Trade`.
   4. Reduce both quantities. If the resting order hits zero, remove it from the queue and from `index_`.
   5. If the level's queue is now empty, erase the level from the map.
3. Leftover quantity: a limit order rests at its price at the back of that level's queue. A market order's leftover is discarded.

Cancel removes the order through `index_` and erases its level if that leaves it empty. Cancelling an unknown or already filled ID returns false and changes nothing.

## Output format

Each trade prints one line as it happens. `PRINT` shows the book with asks above bids, best prices closest to the middle.

For the example session above:

```
TRADE buy=3 sell=1 price=10050 qty=10
--- ASKS ---
10060  5   (1 order)
--- BIDS ---
10055  2   (1 order)
```

Keep output deterministic (no timestamps) so tests can compare it exactly.

## Project layout and build

Keep the book as a library so tests and the CLI both link against it, and the parser stays separate from the matching logic.

```
v1/
  README.md          // this spec
  CMakeLists.txt
  include/orderbook/
    types.hpp        // Side, Order, Trade, aliases
    order_book.hpp   // OrderBook class
    parser.hpp       // line -> command
  src/
    order_book.cpp
    parser.cpp
    main.cpp         // reads stdin, prints output
  tests/
    order_book_test.cpp
    parser_test.cpp
```

- Pull GoogleTest in with CMake's `FetchContent`.
- Name the executable `orderbook_v1` and the test target `orderbook_v1_tests`, so they don't clash with later versions.
- Compile with `-Wall -Wextra -Wpedantic`, and add a debug build with `-fsanitize=address,undefined` to catch memory bugs early.
- Run with `./build/v1/orderbook_v1 < orders.txt`.

## Test cases

Write each as a GoogleTest case that builds a fresh book, sends orders, and checks the returned trades plus `bestBid`, `bestAsk` and `size`.

| Case | Setup | Expected |
| --- | --- | --- |
| No cross | Sell 10 @ 101, buy 10 @ 100 | No trades, both rest |
| Exact fill | Sell 10 @ 100, buy 10 @ 100 | 1 trade of 10 @ 100, book empty |
| Partial fill | Sell 4 @ 100, buy 10 @ 100 | 1 trade of 4, buy rests with 6 |
| Sweep levels | Sells 5 @ 100, 5 @ 101, buy 8 @ 101 | Trades 5 @ 100 then 3 @ 101 |
| Time priority | Sell A then sell B, both @ 100, buy 5 | A fills before B |
| Price improvement | Sell @ 100, buy @ 105 | Trades at 100, not 105 |
| Market, thin book | Sell 3 @ 100, market buy 10 | Trade 3, remaining 7 dropped |
| Market, empty book | Market buy 10 | No trades, nothing rests |
| Cancel resting | Rest an order, cancel it | Returns true, level gone |
| Cancel unknown | Cancel ID 999 | Returns false |
| Cancel filled | Fill an order, then cancel it | Returns false |
| Bad input | `LIMIT x BUY` | Parser returns an error, no crash |

## Milestones

Each step should end with green tests and a commit.

- [ ] CMake project builds, GoogleTest runs a trivial test
- [ ] Core types in `types.hpp`
- [ ] Resting limit orders with no matching, plus `bestBid`, `bestAsk`, `size`
- [ ] Matching for limit orders (no cross, exact, partial, sweep, time priority)
- [ ] Cancel via `index_`
- [ ] Market orders
- [ ] Parser and `main.cpp` reading stdin, with error handling
- [ ] Run the shared `benchmarks/orders_1m.txt` input and measure orders per second
- [ ] Fill in the Results section below, then tag `v1.0` in git

## Out of scope for v1

These are deliberately left for later versions:

- Multiple symbols
- Modify or replace orders
- Other order types: IOC, FOK, stop, iceberg
- Networking, binary protocols, market data feeds
- Multithreading or lock-free structures
- Custom allocators and cache-friendly price level arrays
- Persistence, recovery, auctions, trading halts

## Results

Fill this in when v1 is finished.

- **Benchmark:** TBD orders per second on `benchmarks/orders_1m.txt`.
- **Design changes:** anything that ended up different from this spec, and why.
- **What I learned:** the main C++ and market structure lessons from this version.
- **Next:** what v2 should fix, based on what the benchmark and code showed.
