#include "benchmarks.hpp"

#include <chrono>
#include <algorithm>

double geometricMean(const std::vector<double>& nums) {
    double product = 1.0;
    for (double num : nums) {
        product *= num;
    }
    return std::pow(product, 1.0 / nums.size());
}

void executeInterweaved(Benchmark* bench, 
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args)
{
    // Seed random number generator
    srand(args.seed);

    // Interweaved execution mode parameters
    int numInsert = (args.insert * args.numOperations) / 100;

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

void printVerbose(Benchmark& bench, const std::chrono::duration<double>& elapsed_seconds){
    printf( "\t\t%-25s %25s    Elapsed time: %f (s)    "
            "Collision Count (Buckets): %d\n", 
            bench.getName().c_str(), 
            bench.getHashName().c_str(), 
            elapsed_seconds.count(),
            bench.calculateCollisionCountBuckets());
}

void reportHashMetrics(std::unordered_map<std::string,HashBenchmarkInfo>& hashInfo){
    for(auto& hashBench : hashInfo){
            printf( "\t\t------> %-25s Average time: %f (s)    Geomean time: %f (s)"
                    "    Total Collision Count (Buckets) %d\n", 
                    hashBench.first.c_str(),
                    hashBench.second.averageTime(),
                    hashBench.second.geomeanTime(),
                    hashBench.second.collisionCountBuckets);
        hashBench.second.resetInternalState();
    }
}

void benchmarkExecutor(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys, 
                       const BenchmarkParameters& args,
                       std::unordered_map<std::string,HashBenchmarkInfo>& hashInfo)
{
    // Execution modes are hard coded since we do not expect to add new ones or modify existing ones
    printf("\tInterweaved execution mode (50%% batched inserts):\n");
    for (const auto& bench : benchmarks){

        // Execute benchmark
        auto start = std::chrono::system_clock::now();
        executeInterweaved(bench, keys, args);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;

        // Update Hash Function metrics
        hashInfo[bench->getHashName()].collisionCountBuckets += bench->calculateCollisionCountBuckets();
        hashInfo[bench->getHashName()].samples.push_back(elapsed_seconds.count());

        if(args.verbose){
            printVerbose(*bench, elapsed_seconds);
        }
    }
    reportHashMetrics(hashInfo);

    printf("\tBatch execution mode:\n");
    for (const auto& bench : benchmarks){

        // Execute benchmark
        auto start = std::chrono::system_clock::now();
        executeBatched(bench, keys, args);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;

        // Update Hash Function metrics
        hashInfo[bench->getHashName()].collisionCountBuckets += bench->calculateCollisionCountBuckets();
        hashInfo[bench->getHashName()].samples.push_back(elapsed_seconds.count());

        if(args.verbose){
            printVerbose(*bench, elapsed_seconds);
        }
    }

    reportHashMetrics(hashInfo);

    for(auto bench : benchmarks){
        delete bench;
    }
}
