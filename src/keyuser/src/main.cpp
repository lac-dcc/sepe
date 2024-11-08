/**
 * @file main.cpp
 * @brief Main file for the benchmarking program.
 * 
 * This file includes the main function and other helper functions for the benchmarking program.
 * It parses command line arguments and runs the benchmarks accordingly.
 */

#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>
#include <functional>
#include <unordered_map>

#include "benchmarks.hpp"
#include "customHashes.hpp"
#include "registry.hpp"

std::string correctBenchUsage(){
    return "Correct Usage: ./benchmarks -i <number> -s <number> -e <number> -n <number>\n"
           "       --hashes <hash0> <hash1> ... <hashN>: list of hashes to run\n "
           "       -i or --insert: integer that represents the percentage of insertion operations\n"
           "       -s or --search: integer that represents the percentage of search operations\n"
           "       -e or --elimination: integer that represents the percentage of elimination operations\n"
           "               The sum of -i, -s, and -e should be 100.\n"
           "       -n or --num-operations: integer that represents the number of 'times to perform a hash operation on the benchmark\n"
           "       -r or --repetitions: number of times to repeat the benchmark\n"
           "       -seed: integer that represents the seed for the random number generator\n "
           "       --test-distribution: test the distribution of '--hashes' specified hash functions\n"
           "       --distribution: specify the randon distribution of the keys to be used in the benchmark\n"
           "       --hash-performance: test the execution time of the hash functions over a set of keys\n"  
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
        }else if(strcmp(argv[i], "-r") == 0 || 
                 strcmp(argv[i], "--repetitions") == 0){
            args.repetitions = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-seed") == 0){
            args.seed = atoi(argv[i+1]);
            i+=2;
        }else if(strcmp(argv[i], "-v") == 0 || 
                 strcmp(argv[i], "--verbose") == 0){
            args.verbose = true;
            i++;
        }else if(strcmp(argv[i], "--test-distribution") == 0){
            args.testDistribution = true;
            i++;
        }else if(strcmp(argv[i], "--hash-performance") == 0){
            args.hashPerformance = true;
            i++;
        }else if(strcmp(argv[i], "--distribution") == 0){
            i++;
            std::string distribution = argv[i];
            if(distribution == "normal"){
                args.distribution = "normal";
            }else if(distribution == "uniform"){
                args.distribution = "uniform";
            }else{
                fprintf(stderr,"Invalid distribution: %s! Defaulting to normal.\n", distribution.c_str());
                args.distribution = "normal";
            }
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

int main(int argc, char** argv){

    BenchmarkParameters args = parseArgs(argc, argv);

    // All benchmarks to run
    std::vector<Benchmark*> benchmarks;

    // Register Benchmarks with a macro for beter legibility
    REGISTER_ALL_BENCHMARKS

    // Load keys from standard input into memory
    std::vector<std::string> keys;
    std::string line;
    while(std::getline(std::cin, line)){
        keys.push_back(line);
    }

    // Fill default hash functions to run
    if(args.hashesToRun.empty()){
        args.hashesToRun.push_back("STDHashBin");
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
        }
    }

    // Run benchmarks
    if(args.hashPerformance){
        testHashPerformance(filteredBenchmarks, keys, args);
        freeBenchmarks(filteredBenchmarks);
        return 0;
    }
    if(args.testDistribution){
        testDistribution(filteredBenchmarks, keys);
        freeBenchmarks(filteredBenchmarks);
        return 0;
    }
    benchmarkExecutor(filteredBenchmarks, keys, args);
    freeBenchmarks(filteredBenchmarks);

}

