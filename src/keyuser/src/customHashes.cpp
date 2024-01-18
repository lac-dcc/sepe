#include "customHashes.hpp"
#include <cstring>
#include <smmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

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

std::size_t STDHashMurmur::operator()(const std::string& key) const{
	size_t __seed = static_cast<size_t>(0xc70f6907UL);
    return _Hash_bytes(key.c_str(), key.size(), __seed);
}

std::size_t STDHash::operator()(const std::string& key) const{
    return std::hash<std::string>{}(key);
}

std::size_t FNVHash::operator()(const std::string& key) const {
    std::size_t hash = 0x811c9dc5;  // FNV offset basis
    std::size_t prime = 0x01000193; // FNV prime

    for (char c : key) {
        hash = hash ^ static_cast<std::size_t>(c);
        hash *= prime;
    }

    return hash;
}

std::size_t IPV4HashGeneric::operator()(const std::string& key) const {
    std::size_t hash_code = 0;
    for(const auto ch : key){
        hash_code *= 10;
        hash_code += ch - '0';
    }
    return hash_code;
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

std::size_t IPV4HashBitOps::operator()(const std::string& key) const{

	constexpr std::size_t mask1 = 0x000F0F0F000F0F0F;
	const std::size_t low = _pext_u64(load_u64_le(key.c_str()), mask1);

	constexpr std::size_t mask2 = 0x000F0F0F000F0F0F;
	const std::size_t high = _pext_u64(load_u64_le(key.c_str() + 7), mask2);

	return low | (high << 24);
}

std::size_t CPFHashVectorizedMul::operator()(const std::string& key) const {
	__m128i vector = _mm_lddqu_si128((const __m128i *)key.c_str());
	const __m128i zeros = _mm_set1_epi8('0');
	__m128i sub = _mm_sub_epi8(vector, zeros);

	char* subptr = (char *)(&sub);
	std::size_t hash_code = (std::size_t)subptr[0] 
		+ (std::size_t)subptr[1] *  10 
		+ (std::size_t)subptr[2] *  100
		+ (std::size_t)subptr[4] *  1000 
		+ (std::size_t)subptr[5] *  10000 
		+ (std::size_t)subptr[6] *  100000 
		+ (std::size_t)subptr[8] *  1000000 
		+ (std::size_t)subptr[9] *  10000000 
		+ (std::size_t)subptr[10] * 100000000 
		+ (std::size_t)subptr[12] * 1000000000 
		+ (std::size_t)subptr[13] * 10000000000;
	return hash_code;
}

std::size_t CPFHashBitOps::operator()(const std::string& key) const{

	constexpr std::size_t mask1 = 0x000F0F0F000F0F0F;
	const std::size_t low = _pext_u64(load_u64_le(key.c_str()), mask1);

	constexpr std::size_t mask2 = 0x0F0F000F0F0F0000;
	const std::size_t high = _pext_u64(load_u64_le(key.c_str() + 6), mask2);

	return low | (high << 24);
}

std::size_t SSNHashBitOps::operator()(const std::string& key) const{

	constexpr std::size_t mask1 = 0x0F000F0F000F0F0F;
	const std::size_t low = _pext_u64(load_u64_le(key.c_str()), mask1);

	constexpr std::size_t mask2 = 0x0F0F0F0000000000;
	const std::size_t high = _pext_u64(load_u64_le(key.c_str() + 3), mask2);

	return low | (high << 24);
}


std::size_t CarPlateHashBitOps::operator()(const std::string& key) const{
	return load_u64_le(key.c_str());
}

std::size_t MacAddressHashBitOps::operator()(const std::string& key) const{

	constexpr std::size_t mask1 = 0x7F7F007F7F007F7F;
	const std::size_t low = _pext_u64(load_u64_le(key.c_str()), mask1);

	constexpr std::size_t mask2 = 0x7F7F007F7F007F7F;
	const std::size_t high = _pext_u64(load_u64_le(key.c_str() + 9), mask2);

	return low ^ (high << 21);
}

std::size_t UrlCompress::operator()(const std::string& key) const {
// http:/google.github.io/[a-z]{10}/version[0-9]{2}/doxygen/html/[a-z0-9]{20}.html

//[a-z] 1F -- 10 bytes
	constexpr std::size_t mask1 = 0x1F1F1F1F1F1F1F1F;
	const std::size_t low1 = _pext_u64(load_u64_le(key.c_str()+23), mask1);
	constexpr std::size_t mask2 = 0x0000000000001F1F;
	const std::size_t high1 = _pext_u64(load_u64_le(key.c_str()+31), mask2);

//[0-9] 0F -- 2 bytes
	constexpr std::size_t mask3 = 0x0000000000000F0F;
	const std::size_t low2 = _pext_u64(load_u64_le(key.c_str()+41), mask3);

//[a-z0-9] 7F -- 16 bytes
	constexpr std::size_t mask4 = 0x7F7F7F7F7F7F7F7F;
	const std::size_t high2 = _pext_u64(load_u64_le(key.c_str()+58), mask4);
	constexpr std::size_t mask5 = 0x7F7F7F7F7F7F7F7F;
	const std::size_t low3 = _pext_u64(load_u64_le(key.c_str()+66), mask5);
	constexpr std::size_t mask6 = 0x000000007F7F7F7F;
	const std::size_t high3 = _pext_u64(load_u64_le(key.c_str()+74), mask6);

	return (low1 ^ ( high1 << 24 )) ^ (low2 ^ ( high2 << 8 )) ^ (low3 ^ ( high3 << 24 ));
}

std::size_t UrlGenericHashBitOps::operator()(const std::string& key) const{
	constexpr std::size_t mask1 = 0x3F3F3F3F3F3F3F3F;
	const std::size_t low = _pext_u64(load_u64_le(key.c_str()), mask1);

	constexpr std::size_t mask2 = 0x3F3F3F3F3F3F3F3F;
	const std::size_t high = _pext_u64(load_u64_le(key.c_str() + 8), mask2);

	constexpr std::size_t mask3 = 0x000000003F3F3F3F;
	const std::size_t high2 = _pext_u64(load_u64_le(key.c_str() + 12), mask3);

	return (low ^ (high2 << 36)) ^ (high << 2);
}

std::size_t IntSimdHash::operator()(const std::string& key) const {
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
}

std::size_t IntBitHash::operator()(const std::string& key) const {
    constexpr std::size_t mask = 0x0F0F0F0F0F0F0F0F;

	std::size_t bits1 = _pext_u64(load_u64_le(key.c_str()), mask);
    bits1 |= _pext_u64(load_u64_le(key.c_str() + 8), mask) << 32;

	std::size_t bits2 = _pext_u64(load_u64_le(key.c_str() + 16), mask);
	bits2 |= _pext_u64(load_u64_le(key.c_str() + 24), mask) << 32;

	std::size_t bits3 = _pext_u64(load_u64_le(key.c_str() + 32), mask);
	bits3 |= _pext_u64(load_u64_le(key.c_str() + 40), mask) << 32;

	std::size_t bits4 = _pext_u64(load_u64_le(key.c_str() + 48), mask);
	bits4 |= _pext_u64(load_u64_le(key.c_str() + 56), mask) << 32;

	std::size_t bits5 = _pext_u64(load_u64_le(key.c_str() + 64), mask);
	bits5 |= _pext_u64(load_u64_le(key.c_str() + 72), mask) << 32;

	std::size_t bits6 = _pext_u64(load_u64_le(key.c_str() + 80), mask);
	bits6 |= _pext_u64(load_u64_le(key.c_str() + 88), mask) << 32;

	std::size_t bits7 = _pext_u64(load_u64_le(key.c_str() + 92), mask);

	return bits1 ^ bits2 ^ bits3 ^ bits4 ^ bits5 ^ bits6 ^ bits7;
}