// #ifndef ABSL_BENCHMARKS_HPP
// #define ABSL_BENCHMARKS_HPP

// // #include <iostream>

// #include "customHashes.hpp"

// #include "absl/container/flat_hash_map.h"
// #include "absl/container/flat_hash_set.h"
// #include "absl/container/node_hash_map.h"
// #include "absl/container/node_hash_set.h"


// template <typename HashFuncT>
// class AbslFlatHashMap : public Benchmark{
//     absl::flat_hash_map<std::string, int, HashFuncT> map;

//     public:
//         AbslFlatHashMap(std::string _name, std::string _hashName) : 
//             Benchmark(_name, _hashName)
//             {}

//         void insert(const std::string& key) override {
//             map[key] = 0;
//         }

//         bool search(const std::string& key) override {
//             return map.find(key) != map.end();
//         }

//         void elimination(const std::string& key) override {
//             map.erase(key);
//         }

//         int calculateCollisionCountBuckets(void) override {
//             return internalcalculateCollisionCountBuckets(map);
//         }
// };

// template <typename HashFuncT>
// class AbslFlatHashSet : public Benchmark{
//     absl::flat_hash_set<std::string, int, HashFuncT> map;

//     public:
//         AbslFlatHashSet(std::string _name, std::string _hashName) : 
//             Benchmark(_name, _hashName)
//             {}

//         void insert(const std::string& key) override {
//             map[key] = 0;
//         }

//         bool search(const std::string& key) override {
//             return map.find(key) != map.end();
//         }

//         void elimination(const std::string& key) override {
//             map.erase(key);
//         }

//         int calculateCollisionCountBuckets(void) override {
//             return internalcalculateCollisionCountBuckets(map);
//         }
// };

// template <typename HashFuncT>
// class AbslNodeHashMap : public Benchmark{
//     absl::node_hash_map<std::string, int, HashFuncT> map;

//     public:
//         AbslNodeHashMap(std::string _name, std::string _hashName) : 
//             Benchmark(_name, _hashName)
//             {}

//         void insert(const std::string& key) override {
//             map[key] = 0;
//         }

//         bool search(const std::string& key) override {
//             return map.find(key) != map.end();
//         }

//         void elimination(const std::string& key) override {
//             map.erase(key);
//         }

//         int calculateCollisionCountBuckets(void) override {
//             return internalcalculateCollisionCountBuckets(map);
//         }
// };

// template <typename HashFuncT>
// class AbslNodeHashSet : public Benchmark{
//     absl::node_hash_set<std::string, int, HashFuncT> map;

//     public:
//         AbslNodeHashSet(std::string _name, std::string _hashName) : 
//             Benchmark(_name, _hashName)
//             {}

//         void insert(const std::string& key) override {
//             map[key] = 0;
//         }

//         bool search(const std::string& key) override {
//             return map.find(key) != map.end();
//         }

//         void elimination(const std::string& key) override {
//             map.erase(key);
//         }

//         int calculateCollisionCountBuckets(void) override {
//             return internalcalculateCollisionCountBuckets(map);
//         }
// };

// #endif