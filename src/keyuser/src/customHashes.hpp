/**
 * @file customHashes.hpp
 * @brief This file contains a collection of custom hash functions.
 */

#ifndef CUSTOM_HASHES_HPP
#define CUSTOM_HASHES_HPP

#include <cstdint>
#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>

struct CityHash{
    std::size_t operator()(const std::string& key) const;
};

struct AbseilHash{
    std::size_t operator()(const std::string& key) const;
};

struct STDHashSrc{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct STDHashBin
 * @brief This struct provides a hash function using the standard library's hash function.
 */
struct STDHashBin{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct FNVHash
 * @brief This struct provides a hash function using the Fowler–Noll–Vo hash function.
 */
struct FNVHash {
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct IPV4HashUnrolled
 * @brief This struct provides an unrolled hash function for IPV4 addresses.
 */
struct IPV4HashUnrolled{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct IPV4HashMove
 * @brief This struct provides a hash function for IPV4 addresses that uses move semantics.
 */
struct IPV4HashMove{
    std::size_t operator()(const std::string& key) const;
};


/**
 * @struct IPV4HashBitOps
 * @brief This struct provides a hash function for IPV4 addresses that uses data compression..
 */
struct IPV4HashBitOps{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct CPFHashVectorizedMul
 * @brief This struct provides a hash function for CPF numbers that uses vectorized multiplication.
 */
struct CPFHashVectorizedMul{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct CPFHashBitOps
 * @brief This struct provides a hash function for CPF numbers that uses data compression..
 */
struct CPFHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct SSNHashBitOps
 * @brief This struct provides a hash function for Social Security numbers that uses data compression..
 */
struct SSNHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct CarPlateHashBitOps
 * @brief This struct provides a hash function for car plate numbers that uses data compression..
 */
struct CarPlateHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct MacAddressHashBitOps
 * @brief This struct provides a hash function for MAC addresses that uses data compression..
 */
struct MacAddressHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

struct IntBitHash{
    std::size_t operator()(const std::string& key) const;
};

struct IntSimdHash{
    std::size_t operator()(const std::string& key) const;
};

struct UrlCompress{
    std::size_t operator()(const std::string& key) const;
};

#define DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(NAME) \
    struct Pext ## NAME { \
        std::size_t operator()(const std::string& key) const; \
    }; \
    struct OffXor ## NAME { \
        std::size_t operator()(const std::string& key) const; \
    }; \
    struct Naive ## NAME { \
        std::size_t operator()(const std::string& key) const; \
    }; \
    struct Gpt ## NAME { \
        std::size_t operator()(const std::string& key) const; \
    }; \
    struct Gperf ## NAME { \
        std::size_t operator()(const std::string& key) const; \
    };

DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(UrlComplex)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(Url)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(Mac)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(CPF)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(SSN)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(IPV4)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(IPV6)
DECLARE_SYNTH_OFFXOR_AND_NAIVE_FUNCTIONS(INTS)

struct OffXorSimdUrlComplex{
    std::size_t operator()(const std::string& key) const;
};

struct OffXorSimdUrl {
    std::size_t operator()(const std::string& key) const;
};

struct OffXorSimdIPV6 {
    std::size_t operator()(const std::string& key) const;
}; 

struct OffXorSimdINTS {
    std::size_t operator()(const std::string& key) const;
}; 

struct NaiveSimdUrlComplex{
    std::size_t operator()(const std::string& key) const;
};

struct NaiveSimdUrl {
    std::size_t operator()(const std::string& key) const;
};

struct NaiveSimdIPV6 {
    std::size_t operator()(const std::string& key) const;
}; 

struct NaiveSimdINTS {
    std::size_t operator()(const std::string& key) const;
}; 

#endif
