# Scorpion

Scorpion is a C++17 static utility library for POSIX systems. It includes concurrent containers, rate limiters, task scheduling, encoding, time formatting, Unix sockets, IPv4 filtering, and small system utilities.

## Requirements

- CMake 3.16 or later
- A C++17 compiler
- Graphviz when using `DigraphDot`

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The build creates `libScorpion.a` and standalone examples such as `Encoding`, `LockFreeQueue`, and `ThreadPool`.

## Use

All public APIs are in the `Scorpion` namespace. Add the relevant `src` subdirectory to the include path and link the `Scorpion` target from a CMake parent project.
