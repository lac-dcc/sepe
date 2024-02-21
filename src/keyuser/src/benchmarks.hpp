/**
 * @file benchmarks.hpp
 * @brief This file contains functions to execute and manage hash function benchmarks.
 */

#ifndef BENCHMARKS_HPP
#define BENCHMARKS_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "customHashes.hpp"

/**
 * @brief Base class for benchmarking.
 */
class Benchmark{

    const std::string containerName; ///< The name of the container used in the benchmark.
    const std::string hashName; ///< The name of the hash function used in the benchmark.

    protected:

        /**
         * @brief Calculates the number of collisions in the buckets of a given unordered container.
         * 
         * @tparam UnorderedContainer The type of the unordered container.
         * @param container The unordered container to calculate collisions for.
         * @return The number of collisions.
         */
        template <typename UnorderedContainer>
        int internalcalculateCollisionCountBuckets(const UnorderedContainer& container) {
            // STD Containers may have multiple keys inside the same bucket, even though they have different hashes :(
            int colcount = 0;
            int empty = 0;
            for (size_t bucket = 0; bucket < container.bucket_count(); ++bucket) {
                if (container.bucket_size(bucket) > 1) {
                    colcount += container.bucket_size(bucket) - 1;
                } else {
                    ++empty;
                }
            }
            return colcount;
        }

    public:
        /**
         * @brief Construct a new Benchmark object.
         * 
         * @param _containerName The name of the container used in the benchmark.
         * @param _hashName The name of the hash function used in the benchmark.
         */
        Benchmark(const std::string& _containerName, const std::string& _hashName) : 
            containerName(_containerName),
            hashName(_hashName)
            {}

        /**
         * @brief Get the name of the container used in the benchmark.
         * 
         * @return The name of the container.
         */
        std::string getContainerName(){ return containerName; }

        /**
         * @brief Get the name of the hash function used in the benchmark.
         * 
         * @return The name of the hash function.
         */
        std::string getHashName(){ return hashName; }

        /**
         * @brief Virtual destructor.
         */
        virtual ~Benchmark() {}

        /**
         * @brief Insert a key into the container. Must be implemented by derived classes.
         * 
         * @param key The key to insert.
         */
        virtual void insert(const std::string& key) = 0;

        /**
         * @brief Search for a key in the container. Must be implemented by derived classes.
         * 
         * @param key The key to search for.
         * @return true If the key is found.
         * @return false Otherwise.
         */
        virtual bool search(const std::string& key) = 0;

        /**
         * @brief Remove a key from the container. Must be implemented by derived classes.
         * 
         * @param key The key to remove.
         */
        virtual void elimination(const std::string& key) = 0;

        /**
         * @brief Calculate the number of collision buckets in the container. Must be implemented by derived classes.
         * 
         * @return The number of collision buckets.
         */
        virtual int calculateCollisionCountBuckets(void) = 0;

        /**
         * @brief Get the hash function used by the container. Must be implemented by derived classes.
         * 
         * @return The hash function.
         */
        virtual std::function<std::size_t(const std::string&)> getHashFunction(void) = 0;
};

/**
 * @brief A benchmarking class for unordered map with a custom hash function for std::string.
 * 
 * @tparam HashFuncT The type of the hash function.
 */
template <typename HashFuncT>
class UnorderedMapBench : public Benchmark{
    std::unordered_map<std::string, int, HashFuncT> map; ///< The unordered map used for benchmarking.
    HashFuncT hashFunctor; ///< The hash function object.

    public:
        /**
         * @brief Construct a new Unordered Map Bench object.
         * 
         * @param _name The name of the benchmark.
         * @param _hashName The name of the hash function.
         */
        UnorderedMapBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}

        /**
         * @brief Insert a key into the unordered map.
         * 
         * @param key The key to insert.
         */
        void insert(const std::string& key) override {
            map[key] = 0;
        }

        /**
         * @brief Search for a key in the unordered map.
         * 
         * @param key The key to search for.
         * @return true If the key is found.
         * @return false Otherwise.
         */
        bool search(const std::string& key) override {
            return map.find(key) != map.end();
        }

        /**
         * @brief Remove a key from the unordered map.
         * 
         * @param key The key to remove.
         */
        void elimination(const std::string& key) override {
            map.erase(key);
        }

        /**
         * @brief Calculate the number of collision buckets in the unordered map.
         * 
         * @return int The number of collision buckets.
         */
        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(map);
        }

        /**
         * @brief Get the hash function used by the unordered map.
         * 
         * @return std::function<std::size_t(const std::string&)> The hash function.
         */
        std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return map.hash_function();
        }
};

/**
 * @brief A benchmarking class for unordered multimap with a custom hash function for std::string.
 * 
 * @tparam HashFuncT The type of the hash function.
 */
template <typename HashFuncT>
class UnorderedMultiMapBench : public Benchmark{
    std::unordered_multimap<std::string, int, HashFuncT> mmap; ///< The unordered multimap used for benchmarking.

    public:
        /**
         * @brief Construct a new Unordered Multi Map Bench object.
         * 
         * @param _name The name of the benchmark.
         * @param _hashName The name of the hash function.
         */
        UnorderedMultiMapBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}
            
        /**
         * @brief Insert a key into the unordered multimap.
         * 
         * @param key The key to insert.
         */
        void insert(const std::string& key) override {
            mmap.insert(std::make_pair(key, 0));
        }

        /**
         * @brief Search for a key in the unordered multimap.
         * 
         * @param key The key to search for.
         * @return true If the key is found.
         * @return false Otherwise.
         */
        bool search(const std::string& key) override {
            return mmap.find(key) != mmap.end();
        }

        /**
         * @brief Remove a key from the unordered multimap.
         * 
         * @param key The key to remove.
         */
        void elimination(const std::string& key) override {
            mmap.erase(key);
        }

        /**
         * @brief Calculate the number of collision buckets in the unordered multimap.
         * 
         * @return int The number of collision buckets.
         */
        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(mmap);
        }

        /**
         * @brief Get the hash function used by the unordered multimap.
         * 
         * @return std::function<std::size_t(const std::string&)> The hash function.
         */
        std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return mmap.hash_function();
        }
};

/**
 * @brief A benchmarking class for unordered set with a custom hash function for std::string.
 * 
 * @tparam HashFuncT The type of the hash function.
 */
template <typename HashFuncT>
class UnorderedSetBench : public Benchmark{
    std::unordered_set<std::string, HashFuncT> set; ///< The unordered set used for benchmarking.

    public:
        /**
         * @brief Construct a new Unordered Set Bench object.
         * 
         * @param _name The name of the benchmark.
         * @param _hashName The name of the hash function.
         */
        UnorderedSetBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}

        /**
         * @brief Insert a key into the unordered set.
         * 
         * @param key The key to insert.
         */
        void insert(const std::string& key) override {
            set.insert(key);
        }

        /**
         * @brief Search for a key in the unordered set.
         * 
         * @param key The key to search for.
         * @return true If the key is found.
         * @return false Otherwise.
         */
        bool search(const std::string& key) override {
            return set.find(key) != set.end();
        }

        /**
         * @brief Remove a key from the unordered set.
         * 
         * @param key The key to remove.
         */
        void elimination(const std::string& key) override {
            set.erase(key);
        }

        /**
         * @brief Calculate the number of collision buckets in the unordered set.
         * 
         * @return int The number of collision buckets.
         */
        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(set);
        }

        /**
         * @brief Get the hash function used by the unordered set.
         * 
         * @return std::function<std::size_t(const std::string&)> The hash function.
         */
        std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return set.hash_function();
        }
};

/**
 * @brief A benchmarking class for unordered multiset with a custom hash function for std::string.
 * 
 * @tparam HashFuncT The type of the hash function.
 */
template <typename HashFuncT>
class UnorderedMultisetBench : public Benchmark{
    std::unordered_multiset<std::string, HashFuncT> mset; ///< The unordered multiset used for benchmarking.

    public:
        /**
         * @brief Construct a new Unordered Multiset Bench object.
         * 
         * @param _name The name of the benchmark.
         * @param _hashName The name of the hash function.
         */
        UnorderedMultisetBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}

        /**
         * @brief Insert a key into the unordered multiset.
         * 
         * @param key The key to insert.
         */
        void insert(const std::string& key) override {
            mset.insert(key);
        }

        /**
         * @brief Search for a key in the unordered multiset.
         * 
         * @param key The key to search for.
         * @return true If the key is found.
         * @return false Otherwise.
         */
        bool search(const std::string& key) override {
            return mset.find(key) != mset.end();
        }

        /**
         * @brief Remove a key from the unordered multiset.
         * 
         * @param key The key to remove.
         */
        void elimination(const std::string& key) override {
            mset.erase(key);
        }

        /**
         * @brief Calculate the number of collision buckets in the unordered multiset.
         * 
         * @return int The number of collision buckets.
         */
        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(mset);
        }

        /**
         * @brief Get the hash function used by the unordered multiset.
         * 
         * @return std::function<std::size_t(const std::string&)> The hash function.
         */
        std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return mset.hash_function();
        }
};

/**
 * @brief Struct to hold parameters for benchmarking.
 */
struct BenchmarkParameters{
    std::vector<std::string> hashesToRun; ///< Vector of hash functions to run.
    int insert          = -1; ///< Number of insert operations.
    int search          = -1; ///< Number of search operations.
    int elimination     = -1; ///< Number of elimination operations.
    int numOperations   = -1; ///< Total number of operations.
    int seed            = 223554; ///< Seed for random number generation. Chosen by a fair dice roll.
    int repetitions     = 1; ///< Number of repetitions for each benchmark.
    bool verbose        = false; ///< Verbose output flag.
    bool testDistribution = false; ///< Flag to test distribution.
    bool hashPerformance = false; ///< Flag to test hash performance.
    std::string distribution = "normal"; ///< Distribution to use for testing.
};

/**
 * @brief Execute a benchmark with interweaved operations with 50% insertions warm-up.
 * 
 * @param bench The benchmark to execute.
 * @param keys The keys to use in the benchmark.
 * @param args The parameters for the benchmark.
 */
void executeInterweaved(Benchmark* bench,
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args);

/**
 * @brief Execute a benchmark with batched operations.
 * 
 * @param bench The benchmark to execute.
 * @param keys The keys to use in the benchmark.
 * @param args The parameters for the benchmark.
 */
void executeBatched(Benchmark* bench, 
                    const std::vector<std::string>& keys,
                    const BenchmarkParameters& args);

/**
 * @brief Execute a set of benchmarks and prints to standard output the performance and collision results in csv format.
 * 
 * @param benchmarks The benchmarks to execute.
 * @param keys The keys to use in the benchmarks.
 * @param args The parameters for the benchmarks.
 */
void benchmarkExecutor(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys, 
                       const BenchmarkParameters& args);

/**
 * @brief Tests the distribution of benchmarks. 
 * 
 * This function takes a vector of benchmarks and a vector of keys. It tests the distribution of the benchmarks
 * according to the keys provided.
 * 
 * Prints to standard output a python numpy array containing all sorted hashed values.
 *
 * @param benchmarks A vector of pointers to Benchmark objects to be tested.
 * @param keys A vector of keys according to which the benchmarks are to be distributed.
 */
void testDistribution(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys);

/**
 * @brief Tests the performance of a hash function.
 * 
 * This function takes a vector of keys and benchmark parameters as input, 
 * and prints to standard output the elapsed time in seconds to hash all informed keys.
 *
 * @param keys A constant reference to a vector of strings that represent the keys to be hashed.
 * @param args A constant reference to the BenchmarkParameters object that contains the parameters for the benchmark.
 */
void testHashPerformance(const std::vector<Benchmark*>& benchmarks, 
                         const std::vector<std::string>& keys);

/**
 * @brief Frees the memory allocated for the benchmarks.
 * 
 * This function takes a vector of benchmarks and deallocates the memory used by these benchmarks.
 *
 * @param benchmarks A vector of pointers to Benchmark objects to be freed.
 */
void freeBenchmarks(std::vector<Benchmark*>& benchmarks);

#endif