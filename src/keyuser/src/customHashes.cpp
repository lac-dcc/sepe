/**
 * @file customHashes.cpp
 * @brief Implementation of custom hash functions.
 *
 * This file includes the implementation of several custom hash functions as
 * well as helper functions for loading and shifting data.
 *
 * Some general function types and their brief implementation strategies are:
 *  - Pext: XORS only relevant bytes after compressing them the PEXT instruction.
 *  - Naive: XORS all bytes.
 *  - OffXor: XORS all relevant bytes.
 *  - Gpt: Uses the GPT generated hash functions.
 *  - Gperf: Uses the GPERF generated hash functions.
 *
 *  NOTE THAT THE ARM IMPLEMENTATIONS ARE INCOMPLETE. In particular, we've only
 *  properly implemented the aes hash functions on x86_64. 
**/

#if defined(__amd64__)  || \
    defined(__amd64)    || \
    defined(__x86_64__) || \
    defined(__x86_64)   || \
    defined(_M_X64)     || \
    defined(_M_AMD64)
    #define x86_64
#elif defined(__arm64__) || \
    defined(__arm64)     || \
    defined(__aarch64__) || \
    defined(__aarch64)
    #define ARM
#else
    #error "ARCHITECTURE NOT SUPPORTED"
#endif


#if defined(x86_64)
    #include <smmintrin.h>
    #include <pmmintrin.h>
    #include <immintrin.h>
    #include <wmmintrin.h>
#elif defined(ARM)
    #include <arm_neon.h>
#endif

#include <cstring>

#include "customHashes.hpp"
#include "google-hashes/city.hpp"
#include "absl/hash/hash.h"
#include "absl/hash/internal/hash.h"
extern "C" {
    #include "gperf-hashes/gperf-hashes.h"
}

std::size_t AbseilHash::operator()(const std::string& key) const{
    return absl::Hash<std::string>{}(key);
}

std::size_t CityHash::operator()(const std::string& key) const{
    return CityHash64(key.c_str(), key.size());
}

inline static uint64_t load_u64_le(const char* b) {
    uint64_t Ret;
    // This is a way for the compiler to optimize this func to a single movq instruction
    memcpy(&Ret, b, sizeof(uint64_t));
    return Ret;
}

// C++ STD_HASH implementation extracted from
//https://github.com/gcc-mirror/gcc/blob/ee0717da1eb5dc5d17dcd0b35c88c99281385280/libstdc%2B%2B-v3/libsupc%2B%2B/hash_bytes.cc#L61
static inline std::size_t unaligned_load(const char* p)
{
    std::size_t result;
    __builtin_memcpy(&result, p, sizeof(result));
    return result;
}
static inline std::size_t shift_mix(std::size_t v)
{ return v ^ (v >> 47);}

static inline std::size_t load_bytes(const char* p, int n)
{
    std::size_t result = 0;
    --n;
    do
      result = (result << 8) + static_cast<unsigned char>(p[n]);
    while (--n >= 0);
    return result;
}

// Implementation of Murmur hash for 64-bit size_t.
static size_t _Hash_bytes(const void* ptr, size_t len, size_t seed)
{
    static const size_t mul = (((size_t) 0xc6a4a793UL) << 32UL)
                    + (size_t) 0x5bd1e995UL;
    const char* const buf = static_cast<const char*>(ptr);

    // Remove the bytes not divisible by the sizeof(size_t).  This
    // allows the main loop to process the data as 64-bit integers.
    const size_t len_aligned = len & ~(size_t)0x7;
    const char* const end = buf + len_aligned;
    size_t hash = seed ^ (len * mul);
    for (const char* p = buf; p != end; p += 8)
    {
        const size_t data = shift_mix(unaligned_load(p) * mul) * mul;
        hash ^= data;
        hash *= mul;
    }
    if ((len & 0x7) != 0)
    {
        const size_t data = load_bytes(end, len & 0x7);
        hash ^= data;
        hash *= mul;
    }
    hash = shift_mix(hash) * mul;
    hash = shift_mix(hash);
    return hash;
}

std::size_t STDHashSrc::operator()(const std::string& key) const{
    constexpr size_t __seed = static_cast<size_t>(0xc70f6907UL);
    return _Hash_bytes(key.c_str(), key.size(), __seed);
}

std::size_t STDHashBin::operator()(const std::string& key) const{
    return std::hash<std::string>{}(key);
}

std::size_t FNVHash::operator()(const std::string& key) const {
    const char* cptr = key.c_str();
    size_t len = key.size();
    size_t hash = 0;
    for (; len; --len)
    {
        hash ^= static_cast<size_t>(*cptr++);
        hash *= static_cast<size_t>(1099511628211ULL);
    }
    return hash;
}

std::size_t IPV4HashUnrolled::operator()(const std::string& key) const {

    std::size_t hash_code = (std::size_t)(key[0] - '0')
    + (std::size_t)(key[1] - '0')*10
    + (std::size_t)(key[2] - '0')*100
    + (std::size_t)(key[4] - '0')*1000
    + (std::size_t)(key[5] - '0')*10000
    + (std::size_t)(key[6] - '0')*100000
    + (std::size_t)(key[8] - '0')*1000000
    + (std::size_t)(key[9] - '0')*10000000
    + (std::size_t)(key[10] - '0')*100000000
    + (std::size_t)(key[12] - '0')*1000000000
    + (std::size_t)(key[13] - '0')*10000000000
    + (std::size_t)(key[14] - '0')*100000000000;

    return hash_code;

}

std::size_t IPV4HashMove::operator()(const std::string& key) const {
    return ((std::size_t*)key.c_str())[0];
}

std::size_t IntSimdHash::operator()(const std::string& key) const {
#ifdef x86_64
    __m128i bits[7] = {
        _mm_loadu_si64(key.c_str()),
        _mm_loadu_si64(key.c_str() + 16),
        _mm_loadu_si64(key.c_str() + 32),
        _mm_loadu_si64(key.c_str() + 48),
        _mm_loadu_si64(key.c_str() + 64),
        _mm_loadu_si64(key.c_str() + 80),
        _mm_loadu_si64(key.c_str() + 84),
    };

    bits[1] = _mm_bslli_si128(bits[1], 4);
    bits[3] = _mm_bslli_si128(bits[3], 4);
    bits[5] = _mm_bslli_si128(bits[5], 4);

    __m128i or1 = _mm_or_si128(bits[0], bits[1]);
    __m128i or2 = _mm_or_si128(bits[2], bits[3]);
    __m128i or3 = _mm_or_si128(bits[4], bits[5]);

    __m128i xor1 = _mm_xor_si128(or1, or2);
    __m128i xor2 = _mm_xor_si128(or3, bits[6]);

    const __m128i xor_final = _mm_xor_si128(xor1, xor2);
    std::size_t const * xor_final_ptr = (std::size_t const *)&xor_final;
    return xor_final_ptr[0] ^ xor_final[1];
#elif defined(ARM)
    // we don't care about the ARM Simd implementation right now
    return load_u64_le(key.c_str());
#endif
}

static std::size_t __pext_hash_url_complex(const char* ptr, size_t len, size_t seed) {
    static const size_t mul = (((size_t) 0xc6a4a793UL) << 32UL)
                    + (size_t) 0x5bd1e995UL;
    const char* const buf = static_cast<const char*>(ptr);

    // Remove the bytes not divisible by the sizeof(size_t).  This
    // allows the main loop to process the data as 64-bit integers.
    const size_t len_aligned = len & ~(size_t)0x7;
    const char* const end = buf + len_aligned;
    size_t hash = seed ^ (len * mul);

        constexpr std::size_t mask0 = 0x1f1f1f1f1f1f1f1f;
        constexpr std::size_t mask1 = 0x0000000000001f1f;
        constexpr std::size_t mask2 = 0x00000000000f0f0f;
        constexpr std::size_t mask3 = 0x0000007f7f7f7f7f;
        constexpr std::size_t mask4 = 0x7f7f7f7f7f7f7f7f;
        constexpr std::size_t mask5 = 0x007f7f7f7f7f7f7f;

#ifdef x86_64
        const std::size_t hashable0 = _pext_u64(load_u64_le(ptr+23), mask0);
        const std::size_t hashable1 = _pext_u64(load_u64_le(ptr+31), mask1);
        const std::size_t hashable2 = _pext_u64(load_u64_le(ptr+41), mask2);
        const std::size_t hashable3 = _pext_u64(load_u64_le(ptr+58), mask3);
        const std::size_t hashable4 = _pext_u64(load_u64_le(ptr+66), mask4);
        const std::size_t hashable5 = _pext_u64(load_u64_le(ptr+74), mask5);
#elif defined(ARM)
        const std::size_t hashable0 = load_u64_le(ptr+23) ^ mask0;
        const std::size_t hashable1 = load_u64_le(ptr+31) ^ mask1;
        const std::size_t hashable2 = load_u64_le(ptr+41) ^ mask2;
        const std::size_t hashable3 = load_u64_le(ptr+58) ^ mask3;
        const std::size_t hashable4 = load_u64_le(ptr+66) ^ mask4;
        const std::size_t hashable5 = load_u64_le(ptr+74) ^ mask5;
#endif

        size_t data = shift_mix(hashable0 * mul) * mul;
        hash ^= data;
        hash *= mul;

        data = shift_mix(hashable1 * mul) * mul;
        hash ^= data;
        hash *= mul;

        data = shift_mix(hashable2 * mul) * mul;
        hash ^= data;
        hash *= mul;

        data = shift_mix(hashable3 * mul) * mul;
        hash ^= data;
        hash *= mul;

        data = shift_mix(hashable4 * mul) * mul;
        hash ^= data;
        hash *= mul;

        data = shift_mix(hashable5 * mul) * mul;
        hash ^= data;
        hash *= mul;

        hash = shift_mix(hash) * mul;
        hash = shift_mix(hash);

    hash = shift_mix(hash) * mul;
    hash = shift_mix(hash);

    return hash;
}

std::size_t PextMurmurUrlComplex::operator()(const std::string& key) const {
    constexpr size_t __seed = static_cast<size_t>(0xc70f6907UL);
    return __pext_hash_url_complex(key.c_str(), key.size(), __seed);
}

std::size_t PextUrlComplex::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x1f1f1f1f1f1f1f1f;
    constexpr std::size_t mask1 = 0x0000000000001f1f;
    constexpr std::size_t mask2 = 0x00000000000f0f0f;
    constexpr std::size_t mask3 = 0x7f7f7f7f7f7f7f7f;
    constexpr std::size_t mask4 = 0x7f7f7f7f7f7f7f7f;
    constexpr std::size_t mask5 = 0x000000007f7f7f7f;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+23), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+31), mask1);
    const std::size_t hashable2 = _pext_u64(load_u64_le(key.c_str()+41), mask2);
    const std::size_t hashable3 = _pext_u64(load_u64_le(key.c_str()+58), mask3);
    const std::size_t hashable4 = _pext_u64(load_u64_le(key.c_str()+66), mask4);
    const std::size_t hashable5 = _pext_u64(load_u64_le(key.c_str()+74), mask5);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+23) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+31) ^ mask1;
    const std::size_t hashable2 = load_u64_le(key.c_str()+41) ^ mask2;
    const std::size_t hashable3 = load_u64_le(key.c_str()+58) ^ mask3;
    const std::size_t hashable4 = load_u64_le(key.c_str()+66) ^ mask4;
    const std::size_t hashable5 = load_u64_le(key.c_str()+74) ^ mask5;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 54;
    size_t shift2 = hashable2;
    size_t shift3 = hashable3 << 8;
    size_t shift4 = hashable4;
    size_t shift5 = hashable5 << 36;
    size_t tmp0 = shift0 ^ shift1;
    size_t tmp1 = shift2 ^ shift3;
    size_t tmp2 = shift4 ^ shift5;
    size_t tmp3 = tmp0 ^ tmp1;
    size_t tmp4 = tmp2 ^ tmp3;
    return tmp4;
}

std::size_t PextUrl::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x7f7f7f7f7f7f7f7f;
    constexpr std::size_t mask1 = 0x7f7f7f7f7f7f7f7f;
    constexpr std::size_t mask2 = 0x000000007f7f7f7f;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+45), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+53), mask1);
    const std::size_t hashable2 = _pext_u64(load_u64_le(key.c_str()+61), mask2);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+45) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+53) ^ mask1;
    const std::size_t hashable2 = load_u64_le(key.c_str()+61) ^ mask2;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 8;
    size_t shift2 = hashable2;
    size_t tmp0 = shift0 ^ shift1;
    size_t tmp1 = shift2 ^ tmp0;
    return tmp1;
}


std::size_t PextMac::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x7f7f007f7f007f7f;
    constexpr std::size_t mask1 = 0x7f7f007f7f007f7f;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+9), mask1);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+9) ^ mask1;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 22;
    size_t tmp0 = shift0 ^ shift1;
    return tmp0;
}

std::size_t PextCPF::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x000f0f0f000f0f0f;
    constexpr std::size_t mask1 = 0x0f0f000f0f0f0000;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+6), mask1);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+6) ^ mask1;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 44;
    size_t tmp0 = shift0 ^ shift1;
    return tmp0;
}

std::size_t PextSSN::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x0f000f0f000f0f0f;
    constexpr std::size_t mask1 = 0x0f0f0f0000000000;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+3), mask1);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+3) ^ mask1;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 52;
    size_t tmp0 = shift0 ^ shift1;
    return tmp0;
}

std::size_t PextIPV4::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x000f0f0f000f0f0f;
    constexpr std::size_t mask1 = 0x0f0f0f000f0f0f00;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+7), mask1);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+7) ^ mask1;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 40;
    size_t tmp0 = shift0 ^ shift1;
    return tmp0;
}

std::size_t PextIPV6::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x7f7f7f007f7f7f7f;
    constexpr std::size_t mask1 = 0x7f007f7f7f7f007f;
    constexpr std::size_t mask2 = 0x7f7f7f7f007f7f7f;
    constexpr std::size_t mask3 = 0x7f7f7f007f7f7f7f;
    constexpr std::size_t mask4 = 0x7f7f7f7f007f0000;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+8), mask1);
    const std::size_t hashable2 = _pext_u64(load_u64_le(key.c_str()+16), mask2);
    const std::size_t hashable3 = _pext_u64(load_u64_le(key.c_str()+25), mask3);
    const std::size_t hashable4 = _pext_u64(load_u64_le(key.c_str()+31), mask4);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+8) ^ mask1;
    const std::size_t hashable2 = load_u64_le(key.c_str()+16) ^ mask2;
    const std::size_t hashable3 = load_u64_le(key.c_str()+25) ^ mask3;
    const std::size_t hashable4 = load_u64_le(key.c_str()+31) ^ mask4;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 22;
    size_t shift2 = hashable2;
    size_t shift3 = hashable3 << 15;
    size_t shift4 = hashable4;
    size_t tmp0 = shift0 ^ shift1;
    size_t tmp1 = shift2 ^ shift3;
    size_t tmp2 = shift4 ^ tmp0;
    size_t tmp3 = tmp1 ^ tmp2;
    return tmp3;
}

static std::size_t __pext_hash_ints(const char* ptr, size_t len, size_t seed) {

    static const size_t mul = (((size_t) 0xc6a4a793UL) << 32UL)
                    + (size_t) 0x5bd1e995UL;
    const char* const buf = static_cast<const char*>(ptr);

    // Remove the bytes not divisible by the sizeof(size_t).  This
    // allows the main loop to process the data as 64-bit integers.
    const size_t len_aligned = len & ~(size_t)0x7;
    const char* const end = buf + len_aligned;
    size_t hash = seed ^ (len * mul);

    constexpr std::size_t mask0 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask1 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask2 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask3 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask4 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask5 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask6 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask7 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask8 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask9 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask10 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask11 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask12 = 0x0f0f0f0f00000000;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(ptr+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(ptr+8), mask1);
    const std::size_t hashable2 = _pext_u64(load_u64_le(ptr+16), mask2);
    const std::size_t hashable3 = _pext_u64(load_u64_le(ptr+24), mask3);
    const std::size_t hashable4 = _pext_u64(load_u64_le(ptr+32), mask4);
    const std::size_t hashable5 = _pext_u64(load_u64_le(ptr+40), mask5);
    const std::size_t hashable6 = _pext_u64(load_u64_le(ptr+48), mask6);
    const std::size_t hashable7 = _pext_u64(load_u64_le(ptr+56), mask7);
    const std::size_t hashable8 = _pext_u64(load_u64_le(ptr+64), mask8);
    const std::size_t hashable9 = _pext_u64(load_u64_le(ptr+72), mask9);
    const std::size_t hashable10 = _pext_u64(load_u64_le(ptr+80), mask10);
    const std::size_t hashable11 = _pext_u64(load_u64_le(ptr+88), mask11);
    const std::size_t hashable12 = _pext_u64(load_u64_le(ptr+92), mask12);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(ptr+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(ptr+8) ^ mask1;
    const std::size_t hashable2 = load_u64_le(ptr+16) ^ mask2;
    const std::size_t hashable3 = load_u64_le(ptr+24) ^ mask3;
    const std::size_t hashable4 = load_u64_le(ptr+32) ^ mask4;
    const std::size_t hashable5 = load_u64_le(ptr+40) ^ mask5;
    const std::size_t hashable6 = load_u64_le(ptr+48) ^ mask6;
    const std::size_t hashable7 = load_u64_le(ptr+56) ^ mask7;
    const std::size_t hashable8 = load_u64_le(ptr+64) ^ mask8;
    const std::size_t hashable9 = load_u64_le(ptr+72) ^ mask9;
    const std::size_t hashable10 = load_u64_le(ptr+80) ^ mask10;
    const std::size_t hashable11 = load_u64_le(ptr+88) ^ mask11;
    const std::size_t hashable12 = load_u64_le(ptr+92) ^ mask12;
#endif

    size_t data = shift_mix(hashable0 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable1 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable2 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable3 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable4 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable5 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable6 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable7 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable8 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable9 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable10 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable11 * mul) * mul;
    hash ^= data;
    hash *= mul;

    data = shift_mix(hashable12 * mul) * mul;
    hash ^= data;
    hash *= mul;

    hash = shift_mix(hash) * mul;
    hash = shift_mix(hash);

    return hash;
}

std::size_t PextMurmurINTS::operator()(const std::string& key) const {
    constexpr size_t __seed = static_cast<size_t>(0xc70f6907UL);
    return __pext_hash_ints(key.c_str(), key.size(), __seed);
}

std::size_t PextINTS::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask1 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask2 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask3 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask4 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask5 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask6 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask7 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask8 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask9 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask10 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask11 = 0x0f0f0f0f0f0f0f0f;
    constexpr std::size_t mask12 = 0x0f0f0f0f00000000;
#ifdef x86_64
    const std::size_t hashable0 = _pext_u64(load_u64_le(key.c_str()+0), mask0);
    const std::size_t hashable1 = _pext_u64(load_u64_le(key.c_str()+8), mask1);
    const std::size_t hashable2 = _pext_u64(load_u64_le(key.c_str()+16), mask2);
    const std::size_t hashable3 = _pext_u64(load_u64_le(key.c_str()+24), mask3);
    const std::size_t hashable4 = _pext_u64(load_u64_le(key.c_str()+32), mask4);
    const std::size_t hashable5 = _pext_u64(load_u64_le(key.c_str()+40), mask5);
    const std::size_t hashable6 = _pext_u64(load_u64_le(key.c_str()+48), mask6);
    const std::size_t hashable7 = _pext_u64(load_u64_le(key.c_str()+56), mask7);
    const std::size_t hashable8 = _pext_u64(load_u64_le(key.c_str()+64), mask8);
    const std::size_t hashable9 = _pext_u64(load_u64_le(key.c_str()+72), mask9);
    const std::size_t hashable10 = _pext_u64(load_u64_le(key.c_str()+80), mask10);
    const std::size_t hashable11 = _pext_u64(load_u64_le(key.c_str()+88), mask11);
    const std::size_t hashable12 = _pext_u64(load_u64_le(key.c_str()+92), mask12);
#elif defined(ARM)
    const std::size_t hashable0 = load_u64_le(key.c_str()+0) ^ mask0;
    const std::size_t hashable1 = load_u64_le(key.c_str()+8) ^ mask1;
    const std::size_t hashable2 = load_u64_le(key.c_str()+16) ^ mask2;
    const std::size_t hashable3 = load_u64_le(key.c_str()+24) ^ mask3;
    const std::size_t hashable4 = load_u64_le(key.c_str()+32) ^ mask4;
    const std::size_t hashable5 = load_u64_le(key.c_str()+40) ^ mask5;
    const std::size_t hashable6 = load_u64_le(key.c_str()+48) ^ mask6;
    const std::size_t hashable7 = load_u64_le(key.c_str()+56) ^ mask7;
    const std::size_t hashable8 = load_u64_le(key.c_str()+64) ^ mask8;
    const std::size_t hashable9 = load_u64_le(key.c_str()+72) ^ mask9;
    const std::size_t hashable10 = load_u64_le(key.c_str()+80) ^ mask10;
    const std::size_t hashable11 = load_u64_le(key.c_str()+88) ^ mask11;
    const std::size_t hashable12 = load_u64_le(key.c_str()+92) ^ mask12;
#endif
    size_t shift0 = hashable0;
    size_t shift1 = hashable1 << 32;
    size_t shift2 = hashable2;
    size_t shift3 = hashable3 << 32;
    size_t shift4 = hashable4;
    size_t shift5 = hashable5 << 32;
    size_t shift6 = hashable6;
    size_t shift7 = hashable7 << 32;
    size_t shift8 = hashable8;
    size_t shift9 = hashable9 << 32;
    size_t shift10 = hashable10;
    size_t shift11 = hashable11 << 32;
    size_t shift12 = hashable12;
    size_t tmp0 = shift0 ^ shift1;
    size_t tmp1 = shift2 ^ shift3;
    size_t tmp2 = shift4 ^ shift5;
    size_t tmp3 = shift6 ^ shift7;
    size_t tmp4 = shift8 ^ shift9;
    size_t tmp5 = shift10 ^ shift11;
    size_t tmp6 = shift12 ^ tmp0;
    size_t tmp7 = tmp1 ^ tmp2;
    size_t tmp8 = tmp3 ^ tmp4;
    size_t tmp9 = tmp5 ^ tmp6;
    size_t tmp10 = tmp7 ^ tmp8;
    size_t tmp11 = tmp9 ^ tmp10;
    return tmp11;
}

std::size_t PextINT4::operator()(const std::string& key) const {
    constexpr std::size_t mask0 = 0x0f0f0f0f00000000;
    const std::size_t hashable0 = _pext_u64((unsigned int)load_u64_le(key.c_str()), mask0);
    size_t shift0 = hashable0;
    return shift0;
}

std::size_t OffXorUrlComplex::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+23);
    const std::size_t hashable1 = load_u64_le(key.c_str()+31);
    const std::size_t hashable2 = load_u64_le(key.c_str()+41);
    const std::size_t hashable3 = load_u64_le(key.c_str()+58);
    const std::size_t hashable4 = load_u64_le(key.c_str()+66);
    const std::size_t hashable5 = load_u64_le(key.c_str()+74);
    size_t tmp0 = hashable0 ^ hashable1;
    size_t tmp1 = hashable2 ^ hashable3;
    size_t tmp2 = hashable4 ^ hashable5;
    size_t tmp3 = tmp0 ^ tmp1;
    size_t tmp4 = tmp2 ^ tmp3;
    return tmp4;
}

std::size_t OffXorUrl::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+45);
    const std::size_t hashable1 = load_u64_le(key.c_str()+53);
    const std::size_t hashable2 = load_u64_le(key.c_str()+61);
    size_t tmp0 = hashable0 ^ hashable1;
    size_t tmp1 = hashable2 ^ tmp0;
    return tmp1;

}

std::size_t OffXorMac::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+0);
    const std::size_t hashable1 = load_u64_le(key.c_str()+9);
    size_t tmp0 = hashable0 ^ hashable1;
    return tmp0;
}

std::size_t OffXorCPF::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+0);
    const std::size_t hashable1 = load_u64_le(key.c_str()+6);
    size_t tmp0 = hashable0 ^ hashable1;
    return tmp0;
}

std::size_t OffXorSSN::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+0);
    const std::size_t hashable1 = load_u64_le(key.c_str()+3);
    size_t tmp0 = hashable0 ^ hashable1;
    return tmp0;
}

std::size_t OffXorIPV4::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+0);
    const std::size_t hashable1 = load_u64_le(key.c_str()+7);
    size_t tmp0 = hashable0 ^ hashable1;
    return tmp0;
}

std::size_t OffXorIPV6::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+0);
    const std::size_t hashable1 = load_u64_le(key.c_str()+8);
    const std::size_t hashable2 = load_u64_le(key.c_str()+16);
    const std::size_t hashable3 = load_u64_le(key.c_str()+25);
    const std::size_t hashable4 = load_u64_le(key.c_str()+31);
    size_t tmp0 = hashable0 ^ hashable1;
    size_t tmp1 = hashable2 ^ hashable3;
    size_t tmp2 = hashable4 ^ tmp0;
    size_t tmp3 = tmp1 ^ tmp2;
    return tmp3;
}

std::size_t OffXorINTS::operator()(const std::string& key) const {
    const std::size_t hashable0 = load_u64_le(key.c_str()+0);
    const std::size_t hashable1 = load_u64_le(key.c_str()+8);
    const std::size_t hashable2 = load_u64_le(key.c_str()+16);
    const std::size_t hashable3 = load_u64_le(key.c_str()+24);
    const std::size_t hashable4 = load_u64_le(key.c_str()+32);
    const std::size_t hashable5 = load_u64_le(key.c_str()+40);
    const std::size_t hashable6 = load_u64_le(key.c_str()+48);
    const std::size_t hashable7 = load_u64_le(key.c_str()+56);
    const std::size_t hashable8 = load_u64_le(key.c_str()+64);
    const std::size_t hashable9 = load_u64_le(key.c_str()+72);
    const std::size_t hashable10 = load_u64_le(key.c_str()+80);
    const std::size_t hashable11 = load_u64_le(key.c_str()+88);
    const std::size_t hashable12 = load_u64_le(key.c_str()+92);
    size_t tmp0 = hashable0 ^ hashable1;
    size_t tmp1 = hashable2 ^ hashable3;
    size_t tmp2 = hashable4 ^ hashable5;
    size_t tmp3 = hashable6 ^ hashable7;
    size_t tmp4 = hashable8 ^ hashable9;
    size_t tmp5 = hashable10 ^ hashable11;
    size_t tmp6 = hashable12 ^ tmp0;
    size_t tmp7 = tmp1 ^ tmp2;
    size_t tmp8 = tmp3 ^ tmp4;
    size_t tmp9 = tmp5 ^ tmp6;
    size_t tmp10 = tmp7 ^ tmp8;
    size_t tmp11 = tmp9 ^ tmp10;
    return tmp11;
}

std::size_t OffXorINT4::operator()(const std::string& key) const {
    const std::size_t hashable0 = (unsigned int)load_u64_le(key.c_str());
    return hashable0; 
}

std::size_t NaiveSimdUrlComplex::operator()(const std::string& key) const {
#ifdef x86_64
    __m128i var0 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 0));
    __m128i var1 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 16));
    __m128i var2 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 32));
    __m128i var3 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 48));
    __m128i var4 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 64));
    __m128i var5 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 67));
    __m128i xor0 = _mm_xor_si128(var5, var4);
    __m128i xor1 = _mm_xor_si128(var3, var2);
    __m128i xor2 = _mm_xor_si128(var1, var0);
    __m128i xor3 = _mm_xor_si128(xor0, xor1);
    __m128i xor4 = _mm_xor_si128(xor2, xor3);
    return _mm_extract_epi64(xor4 , 0) ^ _mm_extract_epi64(xor4 , 1);
#elif defined(ARM)
    // we don't care about the ARM Simd implementation right now
    return load_u64_le(key.c_str());
#endif
}

std::size_t NaiveUrlComplex::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 8);
    const std::size_t var2 = load_u64_le(key.c_str() + 16);
    const std::size_t var3 = load_u64_le(key.c_str() + 24);
    const std::size_t var4 = load_u64_le(key.c_str() + 32);
    const std::size_t var5 = load_u64_le(key.c_str() + 40);
    const std::size_t var6 = load_u64_le(key.c_str() + 48);
    const std::size_t var7 = load_u64_le(key.c_str() + 56);
    const std::size_t var8 = load_u64_le(key.c_str() + 64);
    const std::size_t var9 = load_u64_le(key.c_str() + 72);
    const std::size_t var10 = load_u64_le(key.c_str() + 75);
    std::size_t xor0 = var10 ^ var9;
    std::size_t xor1 = var8 ^ var7;
    std::size_t xor2 = var6 ^ var5;
    std::size_t xor3 = var4 ^ var3;
    std::size_t xor4 = var2 ^ var1;
    std::size_t xor5 = var0 ^ xor0;
    std::size_t xor6 = xor1 ^ xor2;
    std::size_t xor7 = xor3 ^ xor4;
    std::size_t xor8 = xor5 ^ xor6;
    std::size_t xor9 = xor7 ^ xor8;
    return xor9;
}

std::size_t NaiveUrl::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 8);
    const std::size_t var2 = load_u64_le(key.c_str() + 16);
    const std::size_t var3 = load_u64_le(key.c_str() + 24);
    const std::size_t var4 = load_u64_le(key.c_str() + 32);
    const std::size_t var5 = load_u64_le(key.c_str() + 40);
    const std::size_t var6 = load_u64_le(key.c_str() + 48);
    const std::size_t var7 = load_u64_le(key.c_str() + 56);
    const std::size_t var8 = load_u64_le(key.c_str() + 62);
    std::size_t xor0 = var8 ^ var7;
    std::size_t xor1 = var6 ^ var5;
    std::size_t xor2 = var4 ^ var3;
    std::size_t xor3 = var2 ^ var1;
    std::size_t xor4 = var0 ^ xor0;
    std::size_t xor5 = xor1 ^ xor2;
    std::size_t xor6 = xor3 ^ xor4;
    std::size_t xor7 = xor5 ^ xor6;
    return xor7 ;
}


std::size_t NaiveMac::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 8);
    const std::size_t var2 = load_u64_le(key.c_str() + 9);
    std::size_t xor0 = var2 ^ var1;
    std::size_t xor1 = var0 ^ xor0;
    return xor1 ;
}

std::size_t NaiveCPF::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 6);
    std::size_t xor0 = var1 ^ var0;
    return xor0 ;
}

std::size_t NaiveSSN::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 3);
    std::size_t xor0 = var1 ^ var0;
    return xor0 ;
}

std::size_t NaiveIPV4::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 7);
    std::size_t xor0 = var1 ^ var0;
    return xor0 ;
}

std::size_t NaiveIPV6::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 8);
    const std::size_t var2 = load_u64_le(key.c_str() + 16);
    const std::size_t var3 = load_u64_le(key.c_str() + 24);
    const std::size_t var4 = load_u64_le(key.c_str() + 31);
    std::size_t xor0 = var4 ^ var3;
    std::size_t xor1 = var2 ^ var1;
    std::size_t xor2 = var0 ^ xor0;
    std::size_t xor3 = xor1 ^ xor2;
    return xor3 ;
}

std::size_t NaiveINTS::operator()(const std::string& key) const {
    const std::size_t var0 = load_u64_le(key.c_str() + 0);
    const std::size_t var1 = load_u64_le(key.c_str() + 8);
    const std::size_t var2 = load_u64_le(key.c_str() + 16);
    const std::size_t var3 = load_u64_le(key.c_str() + 24);
    const std::size_t var4 = load_u64_le(key.c_str() + 32);
    const std::size_t var5 = load_u64_le(key.c_str() + 40);
    const std::size_t var6 = load_u64_le(key.c_str() + 48);
    const std::size_t var7 = load_u64_le(key.c_str() + 56);
    const std::size_t var8 = load_u64_le(key.c_str() + 64);
    const std::size_t var9 = load_u64_le(key.c_str() + 72);
    const std::size_t var10 = load_u64_le(key.c_str() + 80);
    const std::size_t var11 = load_u64_le(key.c_str() + 88);
    const std::size_t var12 = load_u64_le(key.c_str() + 92);
    std::size_t xor0 = var12 ^ var11;
    std::size_t xor1 = var10 ^ var9;
    std::size_t xor2 = var8 ^ var7;
    std::size_t xor3 = var6 ^ var5;
    std::size_t xor4 = var4 ^ var3;
    std::size_t xor5 = var2 ^ var1;
    std::size_t xor6 = var0 ^ xor0;
    std::size_t xor7 = xor1 ^ xor2;
    std::size_t xor8 = xor3 ^ xor4;
    std::size_t xor9 = xor5 ^ xor6;
    std::size_t xor10 = xor7 ^ xor8;
    std::size_t xor11 = xor9 ^ xor10;
    return xor11 ;
}

std::size_t NaiveINT4::operator()(const std::string& key) const {
    const std::size_t hashable0 = (unsigned int)load_u64_le(key.c_str());
    return hashable0; 
}

std::size_t AesUrlComplex::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i mask = _mm_set_epi64x(0xFB6D468E93C391E2 , 0x9c06f0be6f44851b);
    const __m128i hashable0 = _mm_lddqu_si128((const __m128i *)(key.c_str()+23));
    const __m128i hashable1 = _mm_lddqu_si128((const __m128i *)(key.c_str()+58));
    const __m128i hashable2 = _mm_lddqu_si128((const __m128i *)(key.c_str()+67));
    __m128i tmp0 = _mm_aesenc_si128(hashable0, hashable1);
    __m128i tmp1 = _mm_aesenc_si128(hashable2, tmp0);
    return _mm_extract_epi64(tmp1, 0) ^ _mm_extract_epi64(tmp1 , 1);
#elif defined(ARM)
    const uint8x16_t hashable0 = vld1q_u8((const uint8_t *)(key.c_str()+23));
    const uint8x16_t hashable1 = vld1q_u8((const uint8_t *)(key.c_str()+58));
    const uint8x16_t hashable2 = vld1q_u8((const uint8_t *)(key.c_str()+67));
    const uint8x16_t tmp0 = vaeseq_u8(hashable0, hashable1);
    const uint8x16_t tmp1 = vaeseq_u8(hashable2, tmp0);
    const uint64x2_t ret = vreinterpretq_u64_u8(tmp1);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesUrl::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i hashable0 = _mm_lddqu_si128((const __m128i *)(key.c_str()+45));
    const __m128i hashable1 = _mm_lddqu_si128((const __m128i *)(key.c_str()+54));
    __m128i tmp0 = _mm_aesenc_si128(hashable0, hashable1);
    return _mm_extract_epi64(tmp0, 0) ^ _mm_extract_epi64(tmp0 , 1);
#elif defined(ARM)
    const uint8x16_t hashable0 = vld1q_u8((const uint8_t *)(key.c_str()+45));
    const uint8x16_t hashable1 = vld1q_u8((const uint8_t *)(key.c_str()+54));
    const uint8x16_t tmp0 = vaeseq_u8(hashable0, hashable1);
    const uint64x2_t ret = vreinterpretq_u64_u8(tmp0);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesMac::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i roundkey = _mm_set_epi64x(0xFB6D468E93C391E2 , 0x9c06f0be6f44851b);
    const __m128i load = _mm_lddqu_si128((const __m128i *)(key.c_str()));
    const __m128i hash = _mm_aesenc_si128(load, roundkey);
    return _mm_extract_epi64(hash , 0) ^ _mm_extract_epi64(hash, 1);
#elif defined(ARM)
    const uint64_t roundkey_arr[2] = { 0xFB6D468E93C391E2 , 0x9c06f0be6f44851b };
    const uint8x16_t roundkey = vld1q_u8((const uint8_t *)roundkey_arr);
    const uint8x16_t load = vld1q_u8((const uint8_t *)(key.c_str()));
    const uint8x16_t hash = vaeseq_u8(load, roundkey);
    const uint64x2_t ret = vreinterpretq_u64_u8(hash);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesCPF::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i roundkey = _mm_set_epi64x(0xFB6D468E93C391E2 , 0x9c06f0be6f44851b);
    const __m128i load = _mm_set_epi8(key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],key[9],key[10],key[11],key[12],key[13],0,0);
    const __m128i hash = _mm_aesenc_si128(load, roundkey);
    return _mm_extract_epi64(hash , 0) ^ _mm_extract_epi64(hash, 1);
#elif defined(ARM)
    const uint64_t roundkey_arr[2] = { 0xFB6D468E93C391E2 , 0x9c06f0be6f44851b };
    const uint8x16_t roundkey = vld1q_u8((const uint8_t *)roundkey_arr);
    const uint8_t arr[16] = { key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],key[9],key[10],key[11],key[12],key[13],0,0 };
    const uint8x16_t load = vld1q_u8(arr);
    const uint8x16_t hash = vaeseq_u8(load, roundkey);
    const uint64x2_t ret = vreinterpretq_u64_u8(hash);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesSSN::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i roundkey = _mm_set_epi64x(0xFB6D468E93C391E2 , 0x9c06f0be6f44851b);
    const __m128i load = _mm_set_epi8(key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],key[9],key[10],0,0,0,0,0);
    const __m128i hash = _mm_aesenc_si128(load, roundkey);
    return _mm_extract_epi64(hash , 0) ^ _mm_extract_epi64(hash, 1);
#elif defined(ARM)
    const uint64_t roundkey_arr[2] = { 0xFB6D468E93C391E2 , 0x9c06f0be6f44851b };
    const uint8x16_t roundkey = vld1q_u8((const uint8_t *)roundkey_arr);
    const uint8_t arr[16] = { key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],key[9],key[10],0,0,0,0,0 };
    const uint8x16_t load = vld1q_u8(arr);
    const uint8x16_t hash = vaeseq_u8(load, roundkey);
    const uint64x2_t ret = vreinterpretq_u64_u8(hash);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesIPV4::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i roundkey = _mm_set_epi64x(0xFB6D468E93C391E2 , 0x9c06f0be6f44851b);
    const __m128i load = _mm_set_epi8(key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],key[9],key[10],key[11],key[12],key[13],key[14],0);
    const __m128i hash = _mm_aesenc_si128(load, roundkey);
    return _mm_extract_epi64(hash , 0) ^ _mm_extract_epi64(hash, 1);
#elif defined(ARM)
    const uint64_t roundkey_arr[2] = { 0xFB6D468E93C391E2 , 0x9c06f0be6f44851b };
    const uint8x16_t roundkey = vld1q_u8((const uint8_t *)roundkey_arr);
    const uint8_t arr[16] = { key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],key[9],key[10],key[11],key[12],key[13],key[14],0 };
    const uint8x16_t load = vld1q_u8(arr);
    const uint8x16_t hash = vaeseq_u8(load, roundkey);
    const uint64x2_t ret = vreinterpretq_u64_u8(hash);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesIPV6::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i hashable0 = _mm_lddqu_si128((const __m128i *)(key.c_str()+0));
    const __m128i hashable1 = _mm_lddqu_si128((const __m128i *)(key.c_str()+16));
    const __m128i hashable2 = _mm_lddqu_si128((const __m128i *)(key.c_str()+23));
    __m128i tmp0 = _mm_aesenc_si128(hashable0, hashable1);
    __m128i tmp1 = _mm_aesenc_si128(hashable2, tmp0);
    return _mm_extract_epi64(tmp1, 0) ^ _mm_extract_epi64(tmp1 , 1);
#elif defined(ARM)
    const uint8x16_t hashable0 = vld1q_u8((const uint8_t *)(key.c_str()+0));
    const uint8x16_t hashable1 = vld1q_u8((const uint8_t *)(key.c_str()+16));
    const uint8x16_t hashable2 = vld1q_u8((const uint8_t *)(key.c_str()+23));
    const uint8x16_t tmp0 = vaeseq_u8(hashable0, hashable1);
    const uint8x16_t tmp1 = vaeseq_u8(hashable2, tmp0);
    const uint64x2_t ret = vreinterpretq_u64_u8(tmp1);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesINTS::operator()(const std::string& key) const{
#ifdef x86_64
    const __m128i hashable0 = _mm_lddqu_si128((const __m128i *)(key.c_str()+0));
    const __m128i hashable1 = _mm_lddqu_si128((const __m128i *)(key.c_str()+16));
    const __m128i hashable2 = _mm_lddqu_si128((const __m128i *)(key.c_str()+32));
    const __m128i hashable3 = _mm_lddqu_si128((const __m128i *)(key.c_str()+48));
    const __m128i hashable4 = _mm_lddqu_si128((const __m128i *)(key.c_str()+64));
    const __m128i hashable5 = _mm_lddqu_si128((const __m128i *)(key.c_str()+80));
    const __m128i hashable6 = _mm_lddqu_si128((const __m128i *)(key.c_str()+84));
    __m128i tmp0 = _mm_aesenc_si128(hashable0, hashable1);
    __m128i tmp1 = _mm_aesenc_si128(hashable2, hashable3);
    __m128i tmp2 = _mm_aesenc_si128(hashable4, hashable5);
    __m128i tmp3 = _mm_aesenc_si128(hashable6, tmp0);
    __m128i tmp4 = _mm_aesenc_si128(tmp1, tmp2);
    __m128i tmp5 = _mm_aesenc_si128(tmp3, tmp4);
    return _mm_extract_epi64(tmp5, 0) ^ _mm_extract_epi64(tmp5 , 1);
#elif defined(ARM)
    const uint8x16_t hashable0 = vld1q_u8((const uint8_t *)(key.c_str()+0));
    const uint8x16_t hashable1 = vld1q_u8((const uint8_t *)(key.c_str()+16));
    const uint8x16_t hashable2 = vld1q_u8((const uint8_t *)(key.c_str()+32));
    const uint8x16_t hashable3 = vld1q_u8((const uint8_t *)(key.c_str()+48));
    const uint8x16_t hashable4 = vld1q_u8((const uint8_t *)(key.c_str()+64));
    const uint8x16_t hashable5 = vld1q_u8((const uint8_t *)(key.c_str()+80));
    const uint8x16_t hashable6 = vld1q_u8((const uint8_t *)(key.c_str()+84));
    const uint8x16_t tmp0 = vaeseq_u8(hashable0, hashable1);
    const uint8x16_t tmp1 = vaeseq_u8(hashable2, hashable3);
    const uint8x16_t tmp2 = vaeseq_u8(hashable4, hashable5);
    const uint8x16_t tmp3 = vaeseq_u8(hashable6, tmp0);
    const uint8x16_t tmp4 = vaeseq_u8(tmp1, tmp2);
    const uint8x16_t tmp5 = vaeseq_u8(tmp3, tmp4);
    const uint64x2_t ret = vreinterpretq_u64_u8(tmp5);
    return vgetq_lane_u64(ret, 0) ^ vgetq_lane_u64(ret, 1);
#endif
}

std::size_t AesINT4::operator()(const std::string& key) const{
     // chosen by a fair roll of the dice
    const __m128i roundkey = _mm_set_epi64x(0xFB6D468E93C391E2 , 0x9c06f0be6f44851b);
    const __m128i load = _mm_set_epi8(key[0],key[1],key[2],key[3],0,0,0,0,0,0,0,0,0,0,0,0);
    const __m128i hash = _mm_aesenc_si128(load, roundkey);
    return _mm_extract_epi64(hash , 0) ^ _mm_extract_epi64(hash, 1);
}

std::size_t NaiveSimdUrl::operator()(const std::string& key) const {
#ifdef x86_64
    __m128i var0 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 0));
    __m128i var1 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 16));
    __m128i var2 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 32));
    __m128i var3 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 48));
    __m128i var4 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 54));
    __m128i xor0 = _mm_xor_si128(var4, var3);
    __m128i xor1 = _mm_xor_si128(var2, var1);
    __m128i xor2 = _mm_xor_si128(var0, xor0);
    __m128i xor3 = _mm_xor_si128(xor1, xor2);
    return _mm_extract_epi64(xor3 , 0) ^ _mm_extract_epi64(xor3 , 1);
#elif defined(ARM)
    // we don't care about the ARM Simd implementation right now
    return load_u64_le(key.c_str());
#endif
}

std::size_t NaiveSimdINTS::operator()(const std::string& key) const {
#ifdef x86_64
    __m128i var0 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 0));
    __m128i var1 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 16));
    __m128i var2 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 32));
    __m128i var3 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 48));
    __m128i var4 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 64));
    __m128i var5 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 80));
    __m128i var6 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 84));
    __m128i xor0 = _mm_xor_si128(var6, var5);
    __m128i xor1 = _mm_xor_si128(var4, var3);
    __m128i xor2 = _mm_xor_si128(var2, var1);
    __m128i xor3 = _mm_xor_si128(var0, xor0);
    __m128i xor4 = _mm_xor_si128(xor1, xor2);
    __m128i xor5 = _mm_xor_si128(xor3, xor4);
    return _mm_extract_epi64(xor5 , 0) ^ _mm_extract_epi64(xor5 , 1);
#elif defined(ARM)
    // we don't care about the ARM Simd implementation right now
    return load_u64_le(key.c_str());
#endif
}

std::size_t NaiveSimdIPV6::operator()(const std::string& key) const {
#ifdef x86_64
    __m128i var0 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 0));
    __m128i var1 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 16));
    __m128i var2 = _mm_lddqu_si128((const __m128i *)(key.c_str() + 23));
    __m128i xor0 = _mm_xor_si128(var2, var1);
    __m128i xor1 = _mm_xor_si128(var0, xor0);
    return _mm_extract_epi64(xor1 , 0) ^ _mm_extract_epi64(xor1 , 1);
#elif defined(ARM)
    // we don't care about the ARM Simd implementation right now
    return load_u64_le(key.c_str());
#endif
}

std::size_t GptCPF::operator()(const std::string& key) const {
    const std::size_t prime = 31; // A prime number for hashing

    // Unrolled loop for calculating the hash
    std::size_t hashValue = 0;
    hashValue += key[0]  * 1000000000000ULL;
    hashValue += key[1]  * 100000000000ULL;
    hashValue += key[2]  * 10000000000ULL;
    hashValue += key[4]  * 1000000000ULL;
    hashValue += key[5]  * 100000000ULL;
    hashValue += key[6]  * 10000000ULL;
    hashValue += key[8]  * 1000000ULL;
    hashValue += key[9]  * 100000ULL;
    hashValue += key[10] * 10000ULL;
    hashValue += key[11] * 1000ULL;
    hashValue += key[12] * 100ULL;
    hashValue += key[13] * 10ULL;
    hashValue += key[14] * 1ULL;

    // Mixing with a prime number
    hashValue = (hashValue * prime);

    return hashValue;
}

std::size_t GptINTS::operator()(const std::string& key) const {
    constexpr std::size_t keySize = 100;
    constexpr std::size_t unrollFactor = 10; // Unroll the loop by a factor of 10 for optimization

    std::size_t hashValue = 0;

    // Unrolled loop for better performance
    for (std::size_t i = 0; i < keySize; i += unrollFactor) {
        hashValue ^= (static_cast<std::size_t>(key[i]) << 7) ^ (static_cast<std::size_t>(key[i + 1]) << 14)
                   ^ (static_cast<std::size_t>(key[i + 2]) << 21) ^ (static_cast<std::size_t>(key[i + 3]) << 28)
                   ^ (static_cast<std::size_t>(key[i + 4]) << 35) ^ (static_cast<std::size_t>(key[i + 5]) << 42)
                   ^ (static_cast<std::size_t>(key[i + 6]) << 49) ^ (static_cast<std::size_t>(key[i + 7]) << 56)
                   ^ (static_cast<std::size_t>(key[i + 8]) << 63) ^ (static_cast<std::size_t>(key[i + 9]) << 70);
    }

    // Handle the remaining characters in the key (if any)
    for (std::size_t i = (keySize / unrollFactor) * unrollFactor; i < keySize; ++i) {
        hashValue ^= static_cast<std::size_t>(key[i]) << ((i % unrollFactor) * 7);
    }

    return hashValue;
}

std::size_t GptIPV6::operator()(const std::string& key) const {
    // Unrolled loop for optimized performance
    std::size_t hashValue = 0;

    // Process each segment (assuming 8 segments separated by ':')
    hashValue ^= (std::size_t(key[0]) << 24) | (std::size_t(key[1]) << 16) | (std::size_t(key[2]) << 8) | std::size_t(key[3]);
    hashValue ^= (std::size_t(key[5]) << 24) | (std::size_t(key[6]) << 16) | (std::size_t(key[7]) << 8) | std::size_t(key[8]);
    hashValue ^= (std::size_t(key[10]) << 24) | (std::size_t(key[11]) << 16) | (std::size_t(key[12]) << 8) | std::size_t(key[13]);
    hashValue ^= (std::size_t(key[15]) << 24) | (std::size_t(key[16]) << 16) | (std::size_t(key[17]) << 8) | std::size_t(key[18]);
    hashValue ^= (std::size_t(key[20]) << 24) | (std::size_t(key[21]) << 16) | (std::size_t(key[22]) << 8) | std::size_t(key[23]);
    hashValue ^= (std::size_t(key[25]) << 24) | (std::size_t(key[26]) << 16) | (std::size_t(key[27]) << 8) | std::size_t(key[28]);
    hashValue ^= (std::size_t(key[30]) << 24) | (std::size_t(key[31]) << 16) | (std::size_t(key[32]) << 8) | std::size_t(key[33]);
    hashValue ^= (std::size_t(key[35]) << 24) | (std::size_t(key[36]) << 16) | (std::size_t(key[37]) << 8) | std::size_t(key[38]);

    return hashValue;
}

std::size_t GptIPV4::operator()(const std::string& key) const {
    // Assuming key.size() is always 15
    const char constantChar = '.'; // Constant character

    // Unrolled for loop for better performance
    std::size_t hashValue = 0;
    hashValue += static_cast<std::size_t>(key[0]) * 31;
    hashValue += static_cast<std::size_t>(key[1]) * 37;
    hashValue += static_cast<std::size_t>(key[2]) * 41;
    hashValue += static_cast<std::size_t>(key[3]) * 43;
    hashValue += static_cast<std::size_t>(key[4]) * 47;
    hashValue += static_cast<std::size_t>(key[5]) * 53;
    hashValue += static_cast<std::size_t>(key[6]) * 59;
    hashValue += static_cast<std::size_t>(key[7]) * 61;
    hashValue += static_cast<std::size_t>(key[8]) * 67;
    hashValue += static_cast<std::size_t>(key[9]) * 71;
    hashValue += static_cast<std::size_t>(key[10]) * 73;
    hashValue += static_cast<std::size_t>(key[11]) * 79;
    hashValue += static_cast<std::size_t>(key[12]) * 83;
    hashValue += static_cast<std::size_t>(key[13]) * 89;
    hashValue += static_cast<std::size_t>(key[14]) * 97;

    return hashValue;
}

std::size_t GptSSN::operator()(const std::string& key) const {
    constexpr size_t keySize = 11;
    size_t hashValue = 0;

    // Unrolled for loop for better performance
    for (size_t i = 0; i < keySize; i += 3) {
        hashValue = 37 * hashValue + static_cast<size_t>(key[i] - '0');
        hashValue = 37 * hashValue + static_cast<size_t>(key[i + 1] - '0');
        hashValue = 37 * hashValue + static_cast<size_t>(key[i + 2] - '0');
    }

    return hashValue;
}

std::size_t GptMac::operator()(const std::string& key) const {
    // Assuming the key is always in the format 'XX:XX:XX:XX:XX:XX'
    // where the ':' character is at positions 2, 5, 8, 11, 14
    // and ignoring those positions while hashing

    const char constantChar = ':'; // The constant character

    // Unrolled loop for hash calculation
    std::size_t hashValue = 0;

    hashValue ^= static_cast<std::size_t>(key[0]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= static_cast<std::size_t>(key[1]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    hashValue ^= static_cast<std::size_t>(key[3]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= static_cast<std::size_t>(key[4]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    hashValue ^= static_cast<std::size_t>(key[6]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= static_cast<std::size_t>(key[7]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    hashValue ^= static_cast<std::size_t>(key[9]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= static_cast<std::size_t>(key[10]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    hashValue ^= static_cast<std::size_t>(key[12]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= static_cast<std::size_t>(key[13]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    hashValue ^= static_cast<std::size_t>(key[15]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
    hashValue ^= static_cast<std::size_t>(key[16]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    // Combine with constant character
    hashValue ^= static_cast<std::size_t>(constantChar) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

    return hashValue;
}

std::size_t GptUrl::operator()(const std::string& key) const {
    constexpr std::size_t keySize = 70;
    constexpr std::size_t unrollFactor = 5; // Adjust as needed

    std::size_t hashValue = 0;

    // Unrolled loop for better performance
    for (std::size_t i = 0; i < keySize; i += unrollFactor) {
        hashValue ^= (static_cast<std::size_t>(key[i]) << 0) |
                     (static_cast<std::size_t>(key[i + 1]) << 8) |
                     (static_cast<std::size_t>(key[i + 2]) << 16) |
                     (static_cast<std::size_t>(key[i + 3]) << 24) |
                     (static_cast<std::size_t>(key[i + 4]) << 32);
    }

    return hashValue;
}

std::size_t GptUrlComplex::operator()(const std::string& key) const {
    // Constants
    const char constantChar = '/';

    // Unrolled loop for hash calculation
    uint64_t hash = 0;

    // Process [a-z]{10}
    hash ^= (key[7] << 56) | (key[8] << 48) | (key[9] << 40) | (key[10] << 32) |
            (key[11] << 24) | (key[12] << 16) | (key[13] << 8) | key[14];

    // Process [0-9]{3}
    hash ^= (key[27] << 56) | (key[28] << 48) | (key[29] << 40);

    // Process [a-z0-9]{20}
    hash ^= (key[46] << 56) | (key[47] << 48) | (key[48] << 40) | (key[49] << 32) |
            (key[50] << 24) | (key[51] << 16) | (key[52] << 8) | key[53];

    hash ^= (key[54] << 56) | (key[55] << 48) | (key[56] << 40) | (key[57] << 32) |
            (key[58] << 24) | (key[59] << 16) | (key[60] << 8) | key[61];

    hash ^= (key[62] << 56) | (key[63] << 48) | (key[64] << 40) | (key[65] << 32) |
            (key[66] << 24) | (key[67] << 16) | (key[68] << 8) | key[69];

    hash ^= (key[70] << 56) | (key[71] << 48) | (key[72] << 40) | (key[73] << 32) |
            (key[74] << 24) | (key[75] << 16) | (key[76] << 8) | key[77];

    hash ^= (key[78] << 56) | (key[79] << 48) | (key[80] << 40) | (key[81] << 32);

    return hash;
}

std::size_t GperfCPF::operator()(const std::string& key) const {
    return GperfCPFHash(key.c_str(), key.size());
}

std::size_t GperfINTS::operator()(const std::string& key) const {
    return GperfINTSHash(key.c_str(), key.size());
}

std::size_t GperfIPV6::operator()(const std::string& key) const {
    return GperfIPV6Hash(key.c_str(), key.size());
}

std::size_t GperfIPV4::operator()(const std::string& key) const {
    return GperfIPV4Hash(key.c_str(), key.size());
}

std::size_t GperfSSN::operator()(const std::string& key) const {
    return GperfSSNHash(key.c_str(), key.size());
}

std::size_t GperfMac::operator()(const std::string& key) const {
    return GperfMACHash(key.c_str(), key.size());
}

std::size_t GperfUrl::operator()(const std::string& key) const {
    return GperfUrlHash(key.c_str(), key.size());
}

std::size_t GperfUrlComplex::operator()(const std::string& key) const {
    return GperfUrlComplexHash(key.c_str(), key.size());
}
