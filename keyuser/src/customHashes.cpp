#include "customHashes.hpp"
#include<chrono>
#include <smmintrin.h>
#include <pmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

double geometricMean(const std::vector<double>& nums) {
    double product = 1.0;
    for (double num : nums) {
        product *= num;
    }
    return std::pow(product, 1.0 / nums.size());
}


std::size_t RandHash::operator()(const std::string& ) const{
    return rand();
}

static inline constexpr std::size_t a = 863;
static inline constexpr std::size_t c = 401;
static inline constexpr std::size_t m = 4294967291;
// Note: this isn't thread-safe; there **WILL** be race conditions. However,
// they ultimately do not matter, since it won't be noticeable for the player
static std::size_t x = std::chrono::system_clock::now().time_since_epoch().count();
std::size_t FastRandHash::operator()(const std::string& ) const {
    x = (a * x + c) % m;
    return x;
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

//   std::size_t hash_code = 0;
//   // Convert ASCII digits to integers
//   unsigned int digit0 = ((unsigned int*)key.c_str())[0];
//   unsigned int digit1 = (unsigned int)key[4];

//   // Vectorized multiplication
//   hash_code = digit1;
//   hash_code <<= 32;
//   hash_code += digit0;

    return ((std::size_t*)key.c_str())[0];
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
	const std::size_t* u64_ptr = (const std::size_t *)(key.c_str());

	// [0-7] inclusive, 
	//we only care about 0, 1, 2, 4, 5, 6
	constexpr std::size_t mask1 = 0x000F0F0F000F0F0F;
	const std::size_t low = _pext_u64(u64_ptr[0], mask1);
	//printf("u64_ptr[0]: 0x%lx, low: 0x%lx\n", u64_ptr[0], low);

	// [8-15] inclusive, 
	//we only care about 8, 9, 10, 12, 13
	constexpr std::size_t mask2 = 0x00000F0F000F0F0F;
	const std::size_t high = _pext_u64(u64_ptr[1], mask2);

	return low | (high << 24);
}

std::size_t SSNHashBitOps::operator()(const std::string& key) const{
	const std::size_t* u64_ptr = (const std::size_t *)(key.c_str());
    
    //SSN FORMAT: 123-45-6789

	// [0-7] inclusive, 
	//we only care about 0, 1, 2, 4, 5, 7
	constexpr std::size_t mask1 = 0x0F000F0F000F0F0F;
	const std::size_t low = _pext_u64(u64_ptr[0], mask1);
	//printf("u64_ptr[0]: 0x%lx, low: 0x%lx\n", u64_ptr[0], low);

	// [8-15] inclusive, 
	//we only care about 8, 9, 10
	constexpr std::size_t mask2 = 0x00000000000F0F0F;
	const std::size_t high = _pext_u64(u64_ptr[1], mask2);

	return low | (high << 24);
}
