<p align="center">
  <img alt="Sepe Banner" src="assets/images/SepeBanner.png" width="95%" height="auto"/></br>
</p>

## Introduction

This project's goal is to devise, implement, and evaluate techniques for generating optimized hash functions tailored for string keys whose format can be approximated by a regular expression inferred through profiling. These functions will be used to improve the performance of C++'s STL data structures, such as `std::unordered_map`, `std::unordered_set`, `std::unordered_multimap` and `std::unordered_multiset`, in addition to any other `std::hash` specialization for user-defined C++ types.

### Dependencies

These are the most important dependencies for building and running all Sepe programs:

| Dependency | Version   | Installation Link                            |
|------------|-----------|----------------------------------------------|
| clang      | >= 14.0.0 | [llvm.org](https://llvm.org/docs/CMake.html) |
| CMake      | >= 3.20   | [cmake.org](https://cmake.org/install/)      |
| Rust       | >= 1.7    | [rust.org](https://www.rust-lang.org/tools/install)|
| Python     | >= 3.10   | [python.org](https://wiki.python.org/moin/BeginnersGuide/Download)|

Rust is only necessary if you want to run the experiments. If you are only interested in the hash functions generation, only `clang` is necessary.

## Quick-Start: Synthesizing functions

You can follow these two steps to use optimized hash functions generated from this project:

1. Obtain your synthesized hash function in one of the two ways: 
   1. Using a set of [key examples](#synthesizing-from-key-examples).
   2. Using the [regular expression of the keys](#synthesizing-from-regular-expression).
2. [Integrate](#integrating-the-synthesized-function-into-your-project) the optimized hash function into your code .

### Synthesizing from Key Examples

To synthesize hash functions from key examples, you only need to create a file containing a non-exhaustive but representative key set. 

Supposing your key strings are saved in the `txt-file-with-strings` file, you can run the following command:

```sh
./bin/keysynth "$(./bin/keybuilder < txt-file-with-strings)"
```

### Synthesizing from Regular Expression

To build the hash function from the regular expression of your keys, use:

```sh
make
./scripts/make_hash_from_regex.sh [REGEX]
```

Example: *Generating a custom hash function for IPV4 keys*
```sh
./scripts/make_hash_from_regex.sh "(([0-9]{3})\.){3}[0-9]{3}" #or single quotes in zshell
```

See more about regular expressions in the [keygen](#keygen) section.

### Integrating the Synthesized function into your project

Suppose your code has a C++ STL std::unordered_map with IPV4 std::string as keys and int as values.

```cpp
void yourCode(void){
        std::unordered_map<std::string, int, synthesizedOffXorHash> map;
        map["255.255.255.255"] = 42;
        // more code that uses map object
}
```

After running, `./scripts/make_hash_from_regex.sh "(([0-9]{3})\.){3}[0-9]{3}"`, you should get the following output with two function options:

```cpp
// Helper function, include in your codebase:
inline static uint64_t load_u64_le(const char* b) {
        uint64_t Ret;
        // This is a way for the compiler to optimize this func to a single movq instruction
        memcpy(&Ret, b, sizeof(uint64_t));
        return Ret;
}
// Pext Hash Function:
struct synthesizedPextHash {
    // Omitted for brevity in this code snippet
};
// OffXor Hash Function:
struct synthesizedOffXorHash {
        std::size_t operator()(const std::string& key) const {
                const std::size_t hashable0 = load_u64_le(key.c_str()+0);
                const std::size_t hashable1 = load_u64_le(key.c_str()+7);
                size_t tmp0 = hashable0 ^ hashable1;
                return tmp0;
        }
};
```

*If in doubt, we always recommend using the synthesizedOffXorHash variant, according to our benchmarks.*
Copy and paste the desired hash function, in this example, `synthesizedOffXorHash`, into your codebase and then add its name as the third argument in the std::unordered_map template.

```cpp
inline static uint64_t load_u64_le(const char* b) {
        uint64_t Ret;
        // This is a way for the compiler to optimize this func to a single movq instruction
        memcpy(&Ret, b, sizeof(uint64_t));
        return Ret;
}

struct synthesizedOffXorHash {
        std::size_t operator()(const std::string& key) const {
                const std::size_t hashable0 = load_u64_le(key.c_str()+0);
                const std::size_t hashable1 = load_u64_le(key.c_str()+7);
                size_t tmp0 = hashable0 ^ hashable1;
                return tmp0;
        }
};

void yourCode(void){
        std::unordered_map<std::string, int, synthesizedOffXorHash> map;
        map["255.255.255.255"] = 42;
        // more code that uses map object
}
```

## Quick-Start: Benchmarking

Building and running with default parameters:

```sh
./scripts/install_abseil.sh # necessary for keyuser
make && make benchmark
./bin/sepe-runner [REGEXES]
```
Valid regexes are listed in the `Regexes.toml` file.

Example: *Benchmarking all IPV4 hash functions with default parameters*
```
./bin/sepe-runner IPV4
./scripts/keyuser_interpreter.py -p IPV4_performance.csv
```

For more options, see [sepe-runner](#sepe-runner) section: 

## Sepe Components

### keygen

`keygen` generates (standard output) n random keys from Regex. 

Not all valid regexes are accepted since we did not implement the `OR` (`|`), `Kleene Star` (`*`),  `Plus` (`+`), and `DOT` (`.`)  operators.

```sh
./bin/keygen REGEX [number_of_elements] [seed]
```

Example: *Generating 2 random IPV4 keys with seed 223554*

```sh
./bin/keygen "(([0-9]{3})\.){3}[0-9]{3}" -n 2 -s 223554
313.797.178.390
445.982.868.308
```

For more options, do:
```sh
./bin/keygen --help
```

### keyuser

*We recommend using keyuser via [sepe-runner](#sepe-runner)*

`keyuser` benchmarks custom hash functions with keys received from standard input.

```sh
<standard_output_keys> | ./bin/keyuser [hashes] <num_operations> <insert> <search> <elimination> [seed] [verbose]
```

**If no [hashes] are specified, only generic hash functions are executed**

Example: *Benchmarking 2 IPV4 Keys with 10 total operations using STDHashBin PextIPV4 hash functions. 50% insertions, 30% search, and 20% elimination operations.*

```sh
./bin/keygen "(([0-9]{3})\.){3}[0-9]{3}" -n 2 -s 223554 | ./bin/keyuser --hashes STDHashBin PextIPV4 -n 10 -i 50 -s 30 -e 20
```

For more options, do:
```sh
./bin/keyuser --help
```

### keybuilder

`keybuilder` creates a regex from a series of strings passed through standard input, separated by a new line.

```sh
./bin/keybuilder < txt-file-with-strings
```

### keysynth

`keysynth` synthesizes the hash functions based on the regex generated by the `keybuilder`. It is picky about the regex's format, so it is not recommended to hand-write it. Use `keybuilder` instead.

```sh
./bin/keysynth "$(./bin/keybuilder < txt-file-with-strings)"
```

### sepe-runner

`sepe-runner` is a helper program that connects the other programs together as needed.

`Regexes.toml` is a configuration file containing all accepted `sepe-runner` regular expressions and their associated Hash Functions. *Changing this file also requires changing `keyuser`.*

```sh
./bin/sepe-runner Regex-entry-in-Regexes.toml
```
Some relevant parameters are:
- `-k, --keys`: Number of keys to generate
- `-o, --operations`: Number of operations to run
- `-i, --insert`: Percentage of insertion operations
- `-s, --search`: Percentage of search operations
- `-e, --elimination`: Percentage of elimination operations
- `--histogram`: Generate the distribution histogram for the given regex, do not run experiments

Example: *Running the IPV4 benchmark*

```sh
./bin/sepe-runner IPV4
```

For more options, do:
```sh
./bin/sepe-runner --help
```

## Helper Scripts

The `scripts` folder contains some helper scripts that may be useful for some people:

  * `align_csv.sh` - pretty prints `keyuser`'s generated `.csv` files for easier analysis
  * `benchmark.sh` - helper to run many benchmarks at once
  * `install_abseil.sh` - installs the abseil library locally. Necessary for `keyuser`
  * `make_hash_from_regex.sh` - creates a hash function from a user-defined regex
  * `keyuser_interpreter.py` - interprets the results generated from `keyuser`'s benchmarks

### Using `keyuser_interpreter.py`

  This script is used to help interpret the output of `keyuser`. It can plot graphs, generate tables, and perform statistical analysis.

  The most relevant configurations are:

```
-d DISTRIBUTION, --distribution DISTRIBUTION
                      Name of the distribution file to interpret. Exclusive with -p option.
-p [PERFORMANCE ...], --performance [PERFORMANCE ...]
                      Name of the CSV performance files to interpret. Exclusive with -d option.
-pg, --plot-graph     Option to plot the results in graphs.
-hf [HASH_FUNCTIONS ...], --hash-functions [HASH_FUNCTIONS ...]
                      Name of the hash functions to analyze.
```

Example for interpreting performance using IPV4 keys:
```sh
./bin/sepe-runner IPV4 && ./scripts/keyuser_interpreter.py -p IPV4_performance.csv
```

Example for interpreting hash distribution using IPV4 keys:
```sh
./bin/sepe-runner --histogram IPV4 && ./scripts/keyuser_interpreter.py -d IPV4_distribution.py
```
