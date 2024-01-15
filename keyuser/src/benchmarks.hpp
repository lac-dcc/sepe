#ifndef BENCHMARKS_HPP
#define BENCHMARKS_HPP

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <any>

#include "customHashes.hpp"

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

class Benchmark{

    /* Begin declaring class variables */
    const std::string name;
    const std::string hashName;

    protected:
        // std::unordered_multimap<size_t,std::string> liveHashes;
        // int collisionCount;

    /* Begin declaring class methods */

        template <typename UnorderedContainer>
        int internalcalculateCollisionCountBuckets(const UnorderedContainer& container) {
            int colcount = 0;
            for (size_t bucket = 0; bucket < container.bucket_count(); ++bucket) {
                if (container.bucket_size(bucket) > 1) {
                    colcount += container.bucket_size(bucket) - 1;
                }
            }
            return colcount;
        }

    public:
        Benchmark(const std::string& _name, const std::string& _hashName) : 
            name(_name),
            hashName(_hashName)/* ,
            collisionCount(0) */
            {}
        std::string getName(){ return name; }
        std::string getHashName(){ return hashName; }
        // int getCollisionCountInternal(void){ return collisionCount; }
        // void resetCollisionCount(void){ collisionCount = 0; liveHashes.clear(); }

        virtual ~Benchmark() {}
        virtual void insert(const std::string& key) = 0;
        virtual bool search(const std::string& key) = 0;
        virtual void elimination(const std::string& key) = 0;
        // virtual void insertCollisionCount(const std::string& key) = 0;
        // virtual void eliminationCollisionCount(const std::string& key) = 0;
        virtual int calculateCollisionCountBuckets(void) = 0;
};

template <typename HashFuncT>
class UnorderedMapBench : public Benchmark{
    std::unordered_map<std::string, int, HashFuncT> map;

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
        // void insertCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     auto range = liveHashes.equal_range(hash); // Get all keys that have the same hash
        //     int count=0;
        //     for(auto it = range.first; it != range.second; ++it){
        //         count++;
        //         if(it->second != key){ // Its only a collision if the key is different, otherwise its swaping elements
        //             collisionCount += 1;
        //         }
        //     }
        //     liveHashes.insert(std::make_pair(hash, key));
        //     this->insert(key);
        // }
        // void eliminationCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     auto range = liveHashes.equal_range(hash);
        //     for (auto it = range.first; it != range.second; ++it) {
        //         if (it->second == key) { // Only eliminate the right key
        //             it = liveHashes.erase(it);
        //             break;
        //         }
        //     }
        //     this->elimination(key);
        // }
        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(map);
        }
};

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
        // void insertCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     if(liveHashes.find(hash) != liveHashes.end()){
        //         collisionCount += 1;
        //     }
        //     liveHashes.insert(hash);
        //     this->insert(key);
        // }
        // void eliminationCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     liveHashes.erase(hash);
        //     this->elimination(key);
        // }

        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(mmap);
        }
};

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
        // void insertCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     if(liveHashes.find(hash) != liveHashes.end()){
        //         collisionCount += 1;
        //     }
        //     liveHashes.insert(hash);
        //     this->insert(key);
        // }
        // void eliminationCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     liveHashes.erase(hash);
        //     this->elimination(key);
        // }

        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(set);
        }

};


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
        // void insertCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     if(liveHashes.find(hash) != liveHashes.end()){
        //         collisionCount += 1;
        //     }
        //     liveHashes.insert(hash);
        //     this->insert(key);
        // }
        // void eliminationCollisionCount(const std::string& key) override {
        //     size_t hash = HashFuncT{}(key);
        //     liveHashes.erase(hash);
        //     this->elimination(key);
        // }
        int calculateCollisionCountBuckets(void) override {
            return internalcalculateCollisionCountBuckets(mset);
        }
};

struct BenchmarkParameters{
    std::vector<std::string> hashesToRun;
    int insert          = -1;
    int search          = -1;
    int elimination     = -1;
    int numOperations   = -1;
    int seed            = 223554; // Chosen by a fair dice roll
    bool verbose        = false;
};

void executeInterweaved(Benchmark* bench,
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args);

void executeInterweavedCollisionCount(Benchmark* bench,
                        const std::vector<std::string>& keys, 
                        const BenchmarkParameters& args);

void executeBatched(Benchmark* bench, 
                    const std::vector<std::string>& keys,
                    const BenchmarkParameters& args);

void executeBatchedCollisionCount(Benchmark* bench, 
                    const std::vector<std::string>& keys,
                    const BenchmarkParameters& args);

void benchmarkExecutor(const std::vector<Benchmark*>& benchmarks, 
                       const std::vector<std::string>& keys, 
                       const BenchmarkParameters& args,
                       std::unordered_map<std::string,HashBenchmarkInfo>& hashInfo);

#endif