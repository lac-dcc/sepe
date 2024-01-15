#ifndef CUSTOM_HASHES_HPP
#define CUSTOM_HASHES_HPP

#include <cstdint>
#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>

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

struct CarPlateHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

struct MacAddressHashBitOps{
    std::size_t operator()(const std::string& key) const;
};

#endif
