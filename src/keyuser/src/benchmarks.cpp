/**
 * @file benchmarks.cpp
 * @brief Implementation of benchmarking functions.
 * 
 * This file includes the implementation of functions for executing benchmarks.
 * The benchmarks are performed on a set of keys and according to a set of parameters.
 */

#include "benchmarks.hpp"

#include <chrono>
#include <algorithm>
#include <random>

void executeInterweaved(Benchmark* bench, 
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args)
{
    // Seed random number generator
    srand(args.seed);

    // Interweaved execution mode parameters
    int numInsert = (args.insert * args.numOperations) / 100;


    if ( args.distribution == "normal" ) {
        // Create a binomial distribution with parameters n=10 and p=0.5
        std::default_random_engine generator;
        std::binomial_distribution<int> distribution(keys.size(), 0.5);

        // First, insert 50% of the numInserts
        for(int j = 0; j < numInsert/2; j++){
            int randomKey = distribution(generator) % keys.size();
            bench->insert(keys[randomKey]);
        }
        for(int j = 0; j < (args.numOperations-(numInsert/2)); j++){
            int randomKey = distribution(generator) % keys.size();
            int randomOp = rand() % 100;
            if(randomOp < args.insert){
                bench->insert(keys[randomKey]);
            }else if(randomOp < args.insert + args.search){
                bench->search(keys[randomKey]);
            }else{
                bench->elimination(keys[randomKey]);
            }
        }
    } else if ( args.distribution == "uniform" ) {
        // First, insert 50% of the numInserts
        for(int j = 0; j < numInsert/2; j++){
            int randomKey = rand() % keys.size();
            bench->insert(keys[randomKey]);
        }
        for(int j = 0; j < (args.numOperations-(numInsert/2)); j++){
            int randomKey = rand() % keys.size();
            int randomOp = rand() % 100;
            if(randomOp < args.insert){
                bench->insert(keys[randomKey]);
            }else if(randomOp < args.insert + args.search){
                bench->search(keys[randomKey]);
            }else{
                bench->elimination(keys[randomKey]);
            }
        }
    }

}

void executeBatched(Benchmark* bench, 
                    const std::vector<std::string>& keys,
                    const BenchmarkParameters& args)
{
    // Seed random number generator
    srand(args.seed);

    // Batch execution mode parameters
    int numInsert = (args.insert * args.numOperations) / 100;
    int numSearch = (args.search * args.numOperations) / 100;
    int numElimination = (args.elimination * args.numOperations) / 100;

    if ( args.distribution == "normal" ) {
        // Create a binomial distribution with parameters n=10 and p=0.5
        std::default_random_engine generator;
        std::binomial_distribution<int> distribution(keys.size(), 0.5);
        for(int j = 0; j < numInsert; j++){
            int randomKey = distribution(generator) % keys.size();
            bench->insert(keys[randomKey]);
        }
        for(int j = 0; j < numSearch; j++){
            int randomKey = distribution(generator) % keys.size();
            bench->search(keys[randomKey]);
        }
        for(int j = 0; j < numElimination; j++){
            int randomKey = distribution(generator) % keys.size();
            bench->elimination(keys[randomKey]);
        }
    } else if ( args.distribution == "uniform" ) {
        for(int j = 0; j < numInsert; j++){
            int randomKey = rand() % keys.size();
            bench->insert(keys[randomKey]);
        }
        for(int j = 0; j < numSearch; j++){
            int randomKey = rand() % keys.size();
            bench->search(keys[randomKey]);
        }
        for(int j = 0; j < numElimination; j++){
            int randomKey = rand() % keys.size();
            bench->elimination(keys[randomKey]);
        }
    }
}

void printVerbose(Benchmark& bench, const std::chrono::duration<double>& elapsed_seconds){
    printf( "\t\t%-25s %25s    Elapsed time: %f (s)    "
            "Collision Count (Buckets): %d\n", 
            bench.getContainerName().c_str(), 
            bench.getHashName().c_str(), 
            elapsed_seconds.count(),
            bench.calculateCollisionCountBuckets());
}

static void reportMetricsCSV(
                                 const char* execMode,
                                 const char* argsString,
                                 const char* containerName,
                                 const char* hashFuncName,
                                 const float execTime,
                                 const int collisions)
{
    printf( "%s,%s,%s,%s,%f,%d\n",
            execMode,
            argsString,
            containerName,
            hashFuncName,
            execTime,
            collisions);
}

void benchmarkExecutor(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys, 
                       const BenchmarkParameters& args)
{

    // Init CSV File
    printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
        "Execution Mode",
        "Key Distribution",
        "Num Operations",
        "Num Keys",
        "Insertions (%)",
        "Searches (%)",
        "Eliminatons(%)",
        "Hash Container",
        "Hash Function",
        "Execution Time (s)",
        "Collision Count");
    
    char* argsString = (char*)malloc(sizeof(char)*100);
    sprintf(argsString, "%s,%d,%ld,%d,%d,%d",
                        args.distribution.c_str(),
                        args.numOperations,
                        keys.size(),
                        args.insert,
                        args.search,
                        args.elimination);

    // Execution modes are hard coded since we do not expect to add new ones or modify existing ones
    for(int r=0; r < args.repetitions; ++r){
        for (const auto& bench : benchmarks){

            // Execute benchmark
            auto start = std::chrono::system_clock::now();
            executeInterweaved(bench, keys, args);
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;

            reportMetricsCSV("Interweaved",
                             argsString,
                             bench->getContainerName().c_str(),
                             bench->getHashName().c_str(),
                             elapsed_seconds.count(),
                             bench->calculateCollisionCountBuckets());

        }
    }

    for(int r=0; r < args.repetitions; ++r){
        for (const auto& bench : benchmarks){

            // Execute benchmark
            auto start = std::chrono::system_clock::now();
            executeBatched(bench, keys, args);
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;

            reportMetricsCSV("Batched",
                             argsString,
                             bench->getContainerName().c_str(),
                             bench->getHashName().c_str(),
                             elapsed_seconds.count(),
                             bench->calculateCollisionCountBuckets());

        }
    }
    
    free(argsString);
    
}

void testDistribution(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys){

    std::unordered_set<std::string> hashFuncExecuted;
    std::unordered_map<std::string,size_t> hashFuncCollisions;
    printf("import numpy as np\n");
    printf("distributions = {}\n");
    for (const auto& bench : benchmarks){
        if(hashFuncExecuted.find(bench->getHashName()) != hashFuncExecuted.end()){
            continue;
        }
        
        hashFuncExecuted.insert(bench->getHashName());

        std::vector<size_t> buckets;
        auto hashFunc = bench->getHashFunction();

        for(const auto& key : keys){
            size_t hashID = hashFunc(key);
            buckets.push_back(hashID);
        }

        std::sort(buckets.begin(), buckets.end());
        printf("distributions['array_%s'] = np.array([", bench->getHashName().c_str());
        for (size_t i = 0; i < buckets.size(); ++i) {
            printf("%lu,",buckets[i]);
        }
        printf("])\n");
        // Count number of replicated values in the buckets
        auto newEnd = std::unique(buckets.begin(), buckets.end());
        size_t numRepeated = buckets.end() - newEnd;
        hashFuncCollisions[bench->getHashName()] = numRepeated;
    }

    for ( const auto & [hashName, collisions] : hashFuncCollisions ) {
        printf("# Hash Function: %s, Collisions: %lu\n", hashName.c_str(), collisions);
    }
}

void testHashPerformance(const std::vector<Benchmark*>& benchmarks, 
                         const std::vector<std::string>& keys,
                         const BenchmarkParameters& args){

    std::unordered_set<std::string> hashFuncExecuted;
    printf("Hash Function,Elapsed Time (seconds)\n");
    for (const auto& bench : benchmarks){
        if(hashFuncExecuted.find(bench->getHashName()) != hashFuncExecuted.end()){
            continue;
        }

        for(int r=0; r < args.repetitions; ++r){
            hashFuncExecuted.insert(bench->getHashName());
            auto hashFunc = bench->getHashFunction();

            auto start = std::chrono::system_clock::now();
            for(const auto& key : keys){
                size_t hashID = hashFunc(key);
            }
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            printf("%s,%f\n", bench->getHashName().c_str(), elapsed_seconds.count());
        }
    }
}

void freeBenchmarks(std::vector<Benchmark*>& benchmarks){
    for(auto bench : benchmarks){
        delete bench;
    }
}
