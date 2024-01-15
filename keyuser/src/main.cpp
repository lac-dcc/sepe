#include <iostream>
#include <cstring>
#include <vector>
#include <functional>
#include <unordered_map>

#include "benchmarks.hpp"
#include "customHashes.hpp"

#define DECLARE_ONE_BENCH(name, hashname) (Benchmark*)new name<hashname>(#name,#hashname)

// Define Macro to set the benchmarks and HashInfo
#define REGISTER_BENCHMARKS(hashname)   benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedMapBench, hashname)); \
                                        benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedMultiMapBench, hashname)); \
                                        benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedSetBench, hashname)); \
                                        benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedMultisetBench, hashname)); \
                                        hashInfo[#hashname] = HashBenchmarkInfo();

std::string correctBenchUsage(){
    return "Correct Usage: ./benchmarks -i <number> -s <number> -e <number> -n <number>\n"
           "       --hashes <hash0> <hash1> ... <hashN>: list of hashes to run\n "
           "       -i or --insert: integer that represents the percentage of insertion operations\n"
           "       -s or --search: integer that represents the percentage of search operations\n"
           "       -e or --elimination: integer that represents the percentage of elimination operations\n"
           "               The sum of -i, -s, and -e shoud be 100.\n"
           "       -n or --num-operations: integer that represents the number of times to perform a hash operation on the benchmark\n"
           "       -seed: integer that represents the seed for the random number generator\n "
           "       -v or --verbose: print the results of each operation\n"
           "       -h or --help: print this message\n"
           ;
}

BenchmarkParameters parseArgs(int argc, char** argv){
    BenchmarkParameters args;
    args.verbose = false;
    int i = 1;
    while(i<argc){
        if(strcmp(argv[i], "--hashes") == 0){
            i++;
            while(i<argc && argv[i][0] != '-'){
                args.hashesToRun.push_back(argv[i]);
                i++;
            }
        } else if(strcmp(argv[i], "-i") == 0 || 
           strcmp(argv[i], "--insert") == 0){
            args.insert = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-s") == 0 || 
                 strcmp(argv[i], "--search") == 0){
            args.search = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-e") == 0 || 
                 strcmp(argv[i], "--elimination") == 0){
            args.elimination = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-n") == 0 || 
                 strcmp(argv[i], "--num-operations") == 0){
            args.numOperations = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-seed") == 0){
            args.seed = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-v") == 0 || 
                 strcmp(argv[i], "--verbose") == 0){
            args.verbose = true;
            i++;
        }else if(strcmp(argv[i], "-h") == 0 || 
                 strcmp(argv[i], "--help") == 0){
            fprintf(stderr, "%s", correctBenchUsage().c_str());
            exit(0);
        }else{
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            fprintf(stderr, "%s", correctBenchUsage().c_str());
            exit(1);
        }
    }
    int sumArgs = args.insert + args.search + args.elimination;
    if ( sumArgs != 100 ){
        fprintf(stderr, "Missing insert, search, or elimination arguments.\n");
        fprintf(stderr, "%s", correctBenchUsage().c_str());
        exit(1);
    }
    return args;
}

#include <chrono>
int main(int argc, char** argv){
    std::unordered_multiset<std::string, STDHash> mset;

    BenchmarkParameters args = parseArgs(argc, argv);

    // All benchmarks to run
    std::vector<Benchmark*> benchmarks;

    // HashInfo contains all hash functions used in the benchmarks  
    std::unordered_map<std::string,HashBenchmarkInfo> hashInfo;

    // Register Benchmarks
    REGISTER_BENCHMARKS(STDHash);
    REGISTER_BENCHMARKS(FNVHash);

    REGISTER_BENCHMARKS(SSNHashBitOps);

    REGISTER_BENCHMARKS(CPFHashBitOps);
    REGISTER_BENCHMARKS(CPFHashVectorizedMul);

    REGISTER_BENCHMARKS(IPV4HashGeneric);
    REGISTER_BENCHMARKS(IPV4HashUnrolled);
    REGISTER_BENCHMARKS(IPV4HashMove);

    // Load keys from standard input into memory
    std::vector<std::string> keys;
    std::string line;
    while(std::getline(std::cin, line)){
        keys.push_back(line);
    }

    // Fill default hash functions to run
    if(args.hashesToRun.empty()){
        args.hashesToRun.push_back("STDHash");
        args.hashesToRun.push_back("FNVHash");
    }
    // Delete benchmarks that are not in the list of hashes to run
    std::vector<Benchmark*> filteredBenchmarks;
    for(auto& bench : benchmarks){
        // Check if we should run this hash based on arguments
        if(std::find(args.hashesToRun.begin(),
                     args.hashesToRun.end(), 
                     bench->getHashName()) != args.hashesToRun.end())
        {
            filteredBenchmarks.push_back(bench);
        } else {
            delete bench;
            hashInfo.erase(bench->getHashName());
        }
    }

    // Run benchmarks
    benchmarkExecutor(filteredBenchmarks, keys, args, hashInfo);

}
