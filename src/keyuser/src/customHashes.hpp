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

/**
 * @brief Struct for CityHash.
 */
struct CityHash{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for AbseilHash.
 */
struct AbseilHash{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for STDHashSrc.
 */
struct STDHashSrc{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for STDHashBin.
 */
struct STDHashBin{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for FNVHash.
 */
struct FNVHash {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for IPV4HashUnrolled.
 */
struct IPV4HashUnrolled{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for IPV4HashMove.
 */
struct IPV4HashMove{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for IPV4HashBitOps.
 */
struct IPV4HashBitOps{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for CPFHashVectorizedMul.
 */
struct CPFHashVectorizedMul{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for CPFHashBitOps.
 */
struct CPFHashBitOps{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for SSNHashBitOps.
 */
struct SSNHashBitOps{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for CarPlateHashBitOps.
 */
struct CarPlateHashBitOps{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for MacAddressHashBitOps.
 */
struct MacAddressHashBitOps{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for IntBitHash.
 */
struct IntBitHash{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for IntSimdHash.
 */
struct IntSimdHash{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for UrlCompress.
 */
struct UrlCompress{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for PextMurmurINTS.
 */
struct PextMurmurINTS{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for PextMurmurUrlComplex.
 */
struct PextMurmurUrlComplex{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Macro that declares several structs with the same pattern.
 * 
 * Each struct has a function call operator that calculates the hash of a key.
 * The structs are named by concatenating different prefixes with the NAME argument.
 * 
 * @param NAME The name to append to the struct names.
 */
#define DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(NAME) \
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

// Use the macro to declare several structs.
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(UrlComplex)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(Url)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(Mac)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(CPF)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(SSN)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(IPV4)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(IPV6)
DECLARE_SYNTH_OFFXOR_NAIVE_CHATGPT_AND_GPERF_FUNCTIONS(INTS)

/**
 * @brief Struct for OffXorSimdUrlComplex.
 */
struct OffXorSimdUrlComplex{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for OffXorSimdUrl.
 */
struct OffXorSimdUrl {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for OffXorSimdIPV6.
 */
struct OffXorSimdIPV6 {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
}; 

/**
 * @brief Struct for OffXorSimdINTS.
 */
struct OffXorSimdINTS {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
}; 

/**
 * @brief Struct for NaiveSimdUrlComplex.
 */
struct NaiveSimdUrlComplex{
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for NaiveSimdUrl.
 */
struct NaiveSimdUrl {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
};

/**
 * @brief Struct for NaiveSimdIPV6.
 */
struct NaiveSimdIPV6 {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
}; 

/**
 * @brief Struct for NaiveSimdINTS.
 */
struct NaiveSimdINTS {
    /**
     * @brief Function call operator that calculates the hash of a key.
     * 
     * @param key The fixed lenght std::string key to hash.
     * @return The hash of the key.
     */
    std::size_t operator()(const std::string& key) const;
}; 

struct ExperimentalHash {
    std::size_t operator()(const std::string& key) const;
};

#endif
