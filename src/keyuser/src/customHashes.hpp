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
 * @struct STDHash
 * @brief This struct provides a hash function using the standard library's hash function.
 */
struct STDHash{
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
 * @struct IPV4HashGeneric
 * @brief This struct provides a generic hash function for IPV4 addresses.
 */
struct IPV4HashGeneric{
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

/**
 * @struct UrlHashBitOps
 * @brief This struct provides a hash function for URLs that uses data compression..
 */
struct UrlGenericHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

/**
 * @struct UrlHashBitOps
 * @brief This struct provides a hash function for URLs that uses data compression..
 */
struct IntHash{
    std::size_t operator()(const std::string& key) const;
};

#endif
