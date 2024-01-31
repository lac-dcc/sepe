/**
 * @file benchmarks.hpp
 * @brief This file contains functions to execute and manage benchmarks.
 */

#ifndef BENCHMARKS_HPP
#define BENCHMARKS_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "customHashes.hpp"

/**
 * @class Benchmark
 * @brief This class is used for benchmarking purposes.
 *
 * It contains methods and variables that are used to measure
 * the performance of certain operations or algorithms.
 *
 * @var Benchmark::name
 * The name of the benchmark.
 *
 * @var Benchmark::hashName
 * The hashed name of the benchmark.
 *
 * @fn int Benchmark::internalcalculateCollisionCountBuckets(const UnorderedContainer& container)
 * @brief Calculates the number of collisions in the buckets of a given unordered container.
 * @param container The unordered container to calculate collisions for.
 * @return The number of collisions.
 */
class Benchmark{

    /* Begin declaring class variables */
    const std::string containerName;
    const std::string hashName;

    protected:

    /* Begin declaring class methods */

        template <typename UnorderedContainer>
        int internalcalculateCollisionCountBuckets(const UnorderedContainer& container) {
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
        Benchmark(const std::string& _containerName, const std::string& _hashName) : 
            containerName(_containerName),
            hashName(_hashName)
            {}
        std::string getContainerName(){ return containerName; }
        std::string getHashName(){ return hashName; }

        virtual ~Benchmark() {}
        virtual void insert(const std::string& key) = 0;
        virtual bool search(const std::string& key) = 0;
        virtual void elimination(const std::string& key) = 0;
        virtual int calculateCollisionCountBuckets(void) = 0;
        virtual std::function<std::size_t(const std::string&)> getHashFunction(void) = 0;
};


/**
 * @class UnorderedMapBench
 * @brief This class is used for benchmarking operations on an unordered map.
 *
 * It contains methods and variables that are used to measure
 * the performance of certain operations or algorithms on an unordered map.
 *
 * @var UnorderedMapBench::name
 * The name of the benchmark.
 *
 * @var UnorderedMapBench::hashName
 * The hashed name of the benchmark.
 *
 * @fn int UnorderedMapBench::calculateCollisionCountBuckets(const UnorderedMap& map)
 * @brief Calculates the number of collisions in the buckets of a given unordered map.
 * @param map The unordered map to calculate collisions for.
 * @return The number of collisions.
 */
template <typename HashFuncT>
class UnorderedMapBench : public Benchmark{
    std::unordered_map<std::string, int, HashFuncT> map;
    HashFuncT hashFunctor;

    public:
        UnorderedMapBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}

        void insert(const std::string& key) override {
            map[key] = 0;
        }

        bool search(const std::string& key) override {
            return map.find(key) != map.end();
        }

        void elimination(const std::string& key) override {
            map.erase(key);
        }

        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(map);
        }

        virtual std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return map.hash_function();
        }
};

/**
 * @class UnorderedMultiMapBench
 * @brief This class is used for benchmarking operations on an unordered multimap.
 *
 * It contains methods and variables that are used to measure
 * the performance of certain operations or algorithms on an unordered multimap.
 *
 * @var UnorderedMultiMapBench::name
 * The name of the benchmark.
 *
 * @var UnorderedMultiMapBench::hashName
 * The hashed name of the benchmark.
 *
 * @fn int UnorderedMultiMapBench::calculateCollisionCountBuckets(const UnorderedMultiMap& map)
 * @brief Calculates the number of collisions in the buckets of a given unordered multimap.
 * @param map The unordered multimap to calculate collisions for.
 * @return The number of collisions.
 */
template <typename HashFuncT>
class UnorderedMultiMapBench : public Benchmark{
    std::unordered_multimap<std::string, int, HashFuncT> mmap;

    public:
        UnorderedMultiMapBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}
            
        void insert(const std::string& key) override {
            mmap.insert(std::make_pair(key, 0));
        }

        bool search(const std::string& key) override {
            return mmap.find(key) != mmap.end();
        }

        void elimination(const std::string& key) override {
            mmap.erase(key);
        }

        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(mmap);
        }

        virtual std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return mmap.hash_function();
        }
};

/**
 * @class UnorderedSetBench
 * @brief This class is used for benchmarking operations on an unordered set.
 *
 * It contains methods and variables that are used to measure
 * the performance of certain operations or algorithms on an unordered set.
 *
 * @var UnorderedSetBench::name
 * The name of the benchmark.
 *
 * @var UnorderedSetBench::hashName
 * The hashed name of the benchmark.
 *
 * @fn int UnorderedSetBench::calculateCollisionCountBuckets(const UnorderedSet& set)
 * @brief Calculates the number of collisions in the buckets of a given unordered set.
 * @param set The unordered set to calculate collisions for.
 * @return The number of collisions.
 */
template <typename HashFuncT>
class UnorderedSetBench : public Benchmark{
    std::unordered_set<std::string, HashFuncT> set;

    public:
        UnorderedSetBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}

        void insert(const std::string& key) override {
            set.insert(key);
        }

        bool search(const std::string& key) override {
            return set.find(key) != set.end();
        }

        void elimination(const std::string& key) override {
            set.erase(key);
        }

        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(set);
        }

        virtual std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return set.hash_function();
        }

};

/**
 * @class UnorderedMultisetBench
 * @brief This class is used for benchmarking operations on an unordered multiset.
 *
 * It contains methods and variables that are used to measure
 * the performance of certain operations or algorithms on an unordered multiset.
 *
 * @var UnorderedMultisetBench::name
 * The name of the benchmark.
 *
 * @var UnorderedMultisetBench::hashName
 * The hashed name of the benchmark.
 *
 * @fn int UnorderedMultisetBench::calculateCollisionCountBuckets(const UnorderedMultiset& set)
 * @brief Calculates the number of collisions in the buckets of a given unordered multiset.
 * @param set The unordered multiset to calculate collisions for.
 * @return The number of collisions.
 */
template <typename HashFuncT>
class UnorderedMultisetBench : public Benchmark{
    std::unordered_multiset<std::string, HashFuncT> mset;

    public:
        UnorderedMultisetBench(std::string _name, std::string _hashName) : 
            Benchmark(_name, _hashName)
            {}

        void insert(const std::string& key) override {
            mset.insert(key);
        }

        bool search(const std::string& key) override {
            return mset.find(key) != mset.end();
        }

        void elimination(const std::string& key) override {
            mset.erase(key);
        }

        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(mset);
        }

        virtual std::function<std::size_t(const std::string&)> getHashFunction(void) override {
            return mset.hash_function();
        }
};

/**
 * @struct BenchmarkParameters
 * @brief This struct is used to store parameters for a benchmark.
 *
 * It contains variables that hold specific details about the parameters used in a benchmark.
 *
 * @var BenchmarkParameters::name
 * The name of the benchmark.
 *
 * @var BenchmarkParameters::hashName
 * The hashed name of the benchmark.
 *
 * @var BenchmarkParameters::collisionCount
 * The number of collisions that occurred during the benchmark.
 *
 * @var BenchmarkParameters::executionTime
 * The time it took to execute the benchmark.
 */
struct BenchmarkParameters{
    std::vector<std::string> hashesToRun;
    int insert          = -1;
    int search          = -1;
    int elimination     = -1;
    int numOperations   = -1;
    int seed            = 223554; // Chosen by a fair dice roll
    int repetitions     = 1;
    bool verbose        = false;
    bool testDistribution = false;
};

/**
 * @brief Executes a benchmark with interweaved operations.
 * @param bench The benchmark to execute.
 * @param keys The keys to use in the benchmark.
 * @param args The parameters for the benchmark.
 */
void executeInterweaved(Benchmark* bench,
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args);


/**
 * @brief Executes a benchmark with interweaved operations and counts collisions.
 * @param bench The benchmark to execute.
 * @param keys The keys to use in the benchmark.
 * @param args The parameters for the benchmark.
 */
void executeInterweavedCollisionCount(Benchmark* bench,
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args);


/**
 * @brief Executes a benchmark with batched operations.
 * @param bench The benchmark to execute.
 * @param keys The keys to use in the benchmark.
 * @param args The parameters for the benchmark.
 */
void executeBatched(Benchmark* bench, 
                    const std::vector<std::string>& keys,
                    const BenchmarkParameters& args);


/**
 * @brief Executes a benchmark with batched operations and counts collisions.
 * @param bench The benchmark to execute.
 * @param keys The keys to use in the benchmark.
 * @param args The parameters for the benchmark.
 */
void executeBatchedCollisionCount(Benchmark* bench, 
                    const std::vector<std::string>& keys,
                    const BenchmarkParameters& args);


/**
 * @brief Executes a set of benchmarks.
 * @param benchmarks The benchmarks to execute.
 * @param keys The keys to use in the benchmarks.
 * @param args The parameters for the benchmarks.
 * @param hashInfo A map to store information about the benchmarks.
 */
void benchmarkExecutor(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys, 
                       const BenchmarkParameters& args);


void testDistribution(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys);

void freeBenchmarks(std::vector<Benchmark*>& benchmarks);

#endif