#ifndef CUSTOM_HASHES_HPP
#define CUSTOM_HASHES_HPP

#include <cstdint>
#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>

double geometricMean(const std::vector<double>& nums);

struct HashBenchmarkInfo{
    std::vector<double> samples;
    int collisionCountBenchInternal;
    int collisionCountBuckets;

    HashBenchmarkInfo() : collisionCountBenchInternal(0), collisionCountBuckets(0) {}

    void resetInternalState(){
        samples.clear();
        collisionCountBenchInternal = 0;
        collisionCountBuckets = 0;
    }

    double averageTime(){
        return accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
    }
    double geomeanTime(){
        return geometricMean(samples);
    }
};

struct RandHash{
    std::size_t operator()(const std::string& key) const;  
};

struct FastRandHash{
    std::size_t operator()(const std::string& key) const;  
};

struct STDHash{
    std::size_t operator()(const std::string& key) const;
};

struct FNVHash {
    std::size_t operator()(const std::string& key) const;
};

struct IPV4HashGeneric{
    std::size_t operator()(const std::string& key) const;
};

struct IPV4HashUnrolled{
    std::size_t operator()(const std::string& key) const;
};

struct IPV4HashMove{
    std::size_t operator()(const std::string& key) const;
};

struct CPFHashVectorizedMul{
    std::size_t operator()(const std::string& key) const;
};

struct CPFHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

struct SSNHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

#endif
