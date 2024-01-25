<p align="center">
  <img alt="Sepe Banner" src="assets/images/SepeBanner.png" width="95%" height="auto"/></br>
</p>

## Introduction

The goal of this project is to devise, implement, and evaluate techniques for generating optimized hash functions tailored for string keys whose format can be approximated by a regular expression inferred through profiling. These functions will be used to improve the performance of C++'s STL data structures, such as `std::unordered_map`, `std::unordered_set`, `std::unordered_multimap` and `std::unordered_multiset`, in addition to any other `std::hash` specialization for user-defined C++ types.

### Dependencies

These are the most important dependencies for building and running Sepe:

| Dependency | Version   | Installation Link                            |
|------------|-----------|----------------------------------------------|
| clang      | >= 14.0.0 | [llvm.org](https://llvm.org/docs/CMake.html) |
| CMake      | >= 3.20   | [cmake.org](https://cmake.org/install/)      |
| Rust       | >= 1.7    | [rust.org](https://www.rust-lang.org/tools/install)|

## Quick-Start: Building and Running

Building and running with default parameters:

```
make
./bin/bench-runner [REGEX]
```

Valid regexes are listed in the `Regexes.toml` file.

## Building 

`make` will build all binaries and move them to a `bin` file in the top level
directory.

### Building keygen

Requires Rust.

```
cd src/keygen
cargo build --release
```

*Binary output located at : src/keygen/target/release/keygen*

### Building bench-runner

Requires Rust.

```
cd src/bench-runner
cargo build --release
```

*Binary output located at : src/bench-runner/target/release/bench-runner*

### Building keyuser

Requires C++.

```
cd src/keyuser
make
```

*Binary output located at : src/keyuser/keyuser*

## Running

### Running keygen

`keygen` generates (standard output) n random keys from Regex. *Not all valid regexes are accepeted.*

```
./bin/keygen <REGEX> [number_of_elements] [seed]
```

Example: *Generating 2 random IPV4 keys with seed 223554*

```
./bin/keygen "(([0-9]{3})\.){3}[0-9]{3}" -n 2 -s 223554
313.797.178.390
445.982.868.308
```

For more options, do:
```
./bin/keygen --help
```

`keygen` is independent from `keyuser`.

### Running keyuser

`keyuser` benchmarks custom hash functions with keys received from standard input.

```
<standard_output_keys> | ./bin/keyuser [hashes] <num_operations> <insert> <search> <elimination> [seed] [verbose]
```

**If no [hashes] are specified, only generic hash functions are executed**

Example: *Benchmarking 2 IPV4 Keys with 10 total operations using STDHashSrc IPV4HashGeneric hash functions. 50% insertions, 30% search, and 20% elimination operations.*

```
./bin/keygen "(([0-9]{3})\.){3}[0-9]{3}" -n 2 -s 223554 | ./bin/keyuser --hashes STDHashSrc IPV4HashGeneric -n 10 -i 50 -s 30 -e 20
 Interweaved execution mode (50% batched inserts):
                ------> IPV4HashGeneric           Average time: 0.000004 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
                ------> STDHashSrc                   Average time: 0.000005 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
        Batch execution mode:
                ------> IPV4HashGeneric           Average time: 0.000004 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
                ------> STDHashSrc                   Average time: 0.000004 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
```

For more options, do:
```
./bin/keyuser --help
```

### Running bench-runner

`bench-runner` is a helper program that connects `keygen` and `keyuser` together.

```
./bin/bench-runner <Regex entry in Regexes.toml>
```

Example: *Running the IPV4 Benchmark*

```
./bin/bench-runner IPV4
```

For more options, do:
```
./bin/bench-runner --help
```
