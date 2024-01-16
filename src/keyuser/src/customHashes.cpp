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

std::size_t UrlGenericHashBitOps::operator()(const std::string& key) const{
	constexpr std::size_t mask1 = 0x3F3F3F3F3F3F3F3F;
	const std::size_t low = _pext_u64(load_u64_le(key.c_str()), mask1);

	constexpr std::size_t mask2 = 0x3F3F3F3F3F3F3F3F;
	const std::size_t high = _pext_u64(load_u64_le(key.c_str() + 8), mask2);

	constexpr std::size_t mask3 = 0x3F3F3F3F00000000;
	const std::size_t high2 = _pext_u64(load_u64_le(key.c_str() + 12), mask3);

	return (low ^ (high2 << 36)) ^ (high << 2);
}

std::size_t IntHash::operator()(const std::string& key) const {
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
