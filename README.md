# Scorpion

Scorpion is a lightweight C++17 utility library providing common algorithms, concurrent containers, encoding, networking, and system utilities. It targets POSIX environments such as macOS and Linux and currently builds as a static library.

## Features

| Module | Contents |
| --- | --- |
| `algorithm` | Consistent hashing, token bucket, leaky bucket, thread pool, time wheel |
| `concurrent` | Blocking queues, mutex-protected queue and stack, SPSC/MPSC/MPMC lock-free queues, spin lock |
| `encoding` | Base16, Base32, and Base64 encoding and decoding |
| `format` | UTC and local time formatting and parsing |
| `network` | Unix Domain Socket client/server and primary host IP lookup |
| `utilities` | IPv4 filtering, command statistics, asynchronous task pools, call-graph generation |
| `basement` | Singleton, atomic condition variable, and signal handling |
| `experiment` | File-lock-based read/write lock `NRWLock` |

## Requirements

- A C++17 compiler
- CMake 3.16 or newer
- POSIX APIs
- Graphviz with `dot` available in `PATH` when using `DigraphDot`

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

The build produces the static library `libScorpion.a` and standalone example/test executables. There is currently no install target. Another CMake project can link the `Scorpion` target from the build tree and add the `src` directory to its include paths.

```cmake
target_include_directories(app PRIVATE /path/to/Scorpion/src)
target_link_libraries(app PRIVATE Scorpion)
```

All C++ APIs are in the `Scorpion` namespace.

## Quick Start

### Concurrent queues

```cpp
#include "MPMCQueue.h"

Scorpion::MPMCQueue<int> queue(1024);
queue.TryPush(42);

int value = 0;
if (queue.TryPop(value)) {
    // value == 42
}
```

Choose a queue according to the number of producers and consumers:

- `SPSCQueue`: single producer, single consumer
- `MPSCQueue`: multiple producers, single consumer
- `MPMCQueue`: multiple producers, multiple consumers
- `BlockingQueue`: bounded blocking queue
- `ConcurrentQueue`, `ConcurrentStack`: general-purpose mutex-protected containers

`Push`/`Pop` on lock-free queues wait when the queue is full or empty. `TryPush`/`TryPop` return immediately. Queue objects are non-copyable, and element types must satisfy the move, destruction, and assignment requirements of the selected implementation.

### Thread pool

```cpp
#include "ThreadPool.h"

Scorpion::ThreadPool pool(4);
pool.Push([] {
    // Runs on a thread-pool worker
    return 0;
});
```

`Push` returns `false` when the pool is not running or has no workers. The callback return value is currently ignored.

### Time wheel

```cpp
#include "TimeWheel.h"
#include <chrono>

Scorpion::TimeWheel<std::chrono::milliseconds> wheel(true);
wheel.Add([] {
    return 0;
}, 1000, 1);
```

`interval` is expressed in ticks and corresponds to the `TimeWheel` resolution. A positive `loop` value specifies the number of executions; `-1` repeats indefinitely. With `async = true`, callbacks run on an internal thread pool. Use `TimeWheelRaw::Tick()` when driving the wheel manually.

### Encoding

```cpp
#include "BaseX.h"

auto encoded = Scorpion::BaseEncoding::Base64Encode("hello");
auto decoded = Scorpion::BaseEncoding::Base64Decode(encoded);
```

Base16 decoding accepts both cases. Base32 and Base64 use standard padded forms. Decoding errors return an empty string, as does decoding empty input.

### IPv4 filtering

```cpp
#include "IPV4Filter.h"

Scorpion::IPFilter filter;
filter.Add("192.168.1.0/24", Scorpion::IPFilter::BANNED);
filter.Add("192.168.1.10", Scorpion::IPFilter::EXCEPTION);

bool blocked = filter.IsBlocked("192.168.1.10"); // false
```

Rules support individual IPv4 addresses and CIDR ranges. An address is blocked when it matches a blacklist rule and does not match an exception rule.

### Unix Domain Sockets

Server:

```cpp
#include "UnixSocket.h"

Scorpion::UnixServer server("/tmp/scorpion.sock");
server.Create();
server.Listen();
auto client = server.Accept();
```

Client sockets must be created locally before connecting to the server:

```cpp
Scorpion::UnixClient client("/tmp/scorpion-client.sock");
client.Create();
client.Connect("/tmp/scorpion.sock");
```

`Send` and `Recv` process the requested byte count, so both sides must agree on a fixed packet size. Destruction closes the file descriptor and attempts to remove the bound path; applications should avoid reusing an active server path.

## Other utilities

- `ConsistentHash<KEY, VALUE>`: manage consistent-hash nodes with `Add`, `Del`, and `Get`
- `TokenBucket`, `LeakyBucket`: rate limiters based on a monotonic clock
- `TimeHelper`: UTC and local time formatting and parsing
- `CMDStatReport`, `CollectCMDStats`: collect latency statistics by time range
- `SignalHandler`: block selected signals and handle them through `sigwait`
- `AtomicCondition`: combine an atomic value with a condition variable
- `DigraphDot`: read service relationships and generate Graphviz output
- `NRWLock`: cross-process read/write locking backed by stable lock files

## Tests

The build generates standalone test programs:

```bash
./build/Encoding
./build/LockFreeQueue
./build/ThreadPool
./build/IPV4Filter
```

The `TimeWheel` and `CMDStats` examples contain long-running demonstrations and are not quick unit tests. CTest registration is not enabled; run the corresponding executable directly.

## Directory layout

```text
src/
  algorithm/     Algorithms and task scheduling
  basement/      Core infrastructure
  concurrent/    Concurrent containers
  encoding/      Encoding and decoding
  experiment/    Experimental components
  format/        Time formatting
  network/       Network wrappers
  utilities/     General utilities
test/            Examples and regression tests
```

## License

[MIT License](LICENSE)
