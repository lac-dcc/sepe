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
$ ./bench-runner.sh
```

Some important parameters are: NUM_KEYS_TO_GENERATE, NUM_OPERATIONS, KEYGEN_INSERT, KEYGEN_SEARCH, and KEYGEN_ELIMINATION.

## Building 

### Building KeyGen

Requires Rust.

```
cd keygen/ && cargo build --release && cd -
```
*Binary output located at : keygen/target/release/keygen*

### Building KeyUser

Requires C++.

```
cd keyuser/ && make && cd -
```

*Binary output located at : keyuser/keyuser*

## Running

### Running KeyGen

KeyGen generates (stdandard output) n random keys from Regex. *Not all valid regexes are accepeted.*

```
./keygen <REGEX> [number_of_elements] [seed]
```

Example: *Generating 2 random IPV4 keys with seed 223554*

```
./keygen/target/release/keygen "(([0-9]{3})\.){3}[0-9]{3}" -n 2 -s 223554
313.797.178.390
445.982.868.308
```

See all usability with:
```
./keygen/target/release/keygen --help
```

KeyGen is independent from KeyUser.

### Running KeyUser

KeyUser benchmarks custom hash functions with keys received from standard input.

```
<standard_output_keys> | ./keyuser/keyuser <num_operations> <insert> <search> <elimination> [seed] [verbose]
```

Example: *Benchmarking 2 IPV4 Keys with 10 total operations*

```
./keygen/target/release/keygen "(([0-9]{3})\.){3}[0-9]{3}" -n 2 -s 223554 | ./keyuser/keyuser -n 10 -i 50 -s 30 -e 20
 Interweaved execution mode (50% batched inserts):
                ------> IPV4HashGeneric           Average time: 0.000004 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
                ------> STDHash                   Average time: 0.000005 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
        Batch execution mode:
                ------> IPV4HashGeneric           Average time: 0.000004 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
                ------> STDHash                   Average time: 0.000004 (s)    Geomean time: 0.000004 (s)    Total Collision Count (Buckets) 4
```

**For specific Hash Functions, register them in the main.cpp with the REGISTER_BENCHMARKS**

See all usability with:
```
./keyuser/keyuser --help
```

