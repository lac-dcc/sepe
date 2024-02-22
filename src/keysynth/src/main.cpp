#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm>

// Constant to hold a helper function used within the synthetized functions
static const std::string load_u64_le = "inline static uint64_t load_u64_le(const char* b) {\n\
\tuint64_t Ret;\n\
\t// This is a way for the compiler to optimize this func to a single movq instruction\n\
\tmemcpy(&Ret, b, sizeof(uint64_t));\n\
\treturn Ret;\n}";

/**
 * @brief Converts an integer to a 2 digits hexadecimal string.
 *
 * This function takes a size_t integer as input and returns its hexadecimal representation as a string.
 *
 * @param num The integer to be converted to hexadecimal.
 * @return std::string The hexadecimal representation of the input integer.
 */
static std::string intToHex(size_t num){
    char numStr[17];
    sprintf(numStr, "%02lx", num);
    return std::string(numStr);
}

/**
 * @struct Range
 * @brief A structure representing a range of characters.
 *
 * This structure represents a range of characters, with a start and end character, an offset, a repetition count, and a mask.
 */
struct Range{
    char start; ///< The start character of the range.
    char end; ///< The end character of the range.
    int offset; ///< The offset of the range.
    size_t repetition; ///< The repetition count of the range.
    char mask; ///< The mask associated with the range. Only useful for PEXT.
    float entropy; ///< The associated entropy of the range.

    Range(char _start, char _end, int _offset, size_t _repetition, float _entropy) :
        start(_start),
        end(_end),
        offset(_offset),
        repetition(_repetition),
        entropy(_entropy)
    {

        /**
         * This function calculates a mask that ignores static bits across a start to an end char range. The process is as follows:
         *  We assume masks are 8 bytes for a range, i.e. [a-z] has a mask of 0x1F, and [a-z0-9] has a mask of 0x7F.
         * - `zeroes` contain all bits that are always zero.
         *   First, we mark all positions that contain at least one one, then we negate it to get all bits that are always zero.
         * - `ones` contain all bits that are always one.
         * - Then, we OR `zeroes` and `ones`. The bits that never change are 0, everything else is 1.
         * - Finally, we invert the mask because we want to keep the bits that change across the range.
         *   Prety cool right?
         */
        char zeroes = 0;
        char ones = start;
        for(char ch = start; ch < end ; ch++){
            zeroes |= ch;
            ones &= ch;
        }
        zeroes = ~zeroes;
        mask = zeroes | ones;
        mask = ~mask;
        if(mask == 0){
            mask = 0x7F; // Default
        }
    }

    void print(){
        printf("Range: %c - %c : offset %d repetition %lu\n", start, end, offset, repetition);
    }

};

/**
 * @brief Count the number of zero bits in a size_t value.
 *
 * This function counts and returns the number of zero bits in the binary representation of a size_t value.
 *
 * @param x The size_t value to count zero bits in.
 * @return int The number of zero bits in x.
 */
static inline int countZeros(const size_t x) {
    int zeros = 0;
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
        if ((x & (1L << i)) == 0) {
            ++zeros;
        }
    }
    return zeros;
}

/**
 * @brief Check if the next range is continuous with the current range.
 *
 * This function checks if the next range in the vector is continuous with the current range.
 * It does this by comparing the offset of the next range with the sum of the offset and repetition of the current range.
 *
 * @param ranges The vector of Range objects.
 * @param rangeIdx The index of the current range in the vector.
 * @return bool True if the next range is continuous with the current range, false otherwise.
 */
static bool nextIsContinuous(const std::vector<Range>& ranges, size_t rangeIdx){
    return ((size_t)ranges[rangeIdx+1].offset == ranges[rangeIdx].offset + ranges[rangeIdx].repetition);
}

/**
 * @brief Recursively calculate the mask for a vector of Range objects.
 *
 * This function recursively calculates the mask for a vector of Range objects. It does this by iterating over the ranges and
 * generating a hexadecimal string representation of the mask for each range. The mask size in bytes is (n-1)*8 <= maskSize <= n*8, where n is the range size.
 * So, if the next range is not continuous with the current range, it adds zeroes to the mask string to represent the gap.
 *
 * @param ranges The vector of Range objects.
 * @param rangeIdx The index of the current range in the vector.
 * @param numRepetitions The number of times the current range has been repeated.
 * @return std::string The hexadecimal string representation of the mask for the ranges.
 */
static std::string recursivelyCalculateMask(const std::vector<Range>& ranges, size_t rangeIdx, size_t numRepetitions){

    // Stop condition
    if(rangeIdx >= ranges.size()){
        return "";
    }

    // Advance to the next range
    if(ranges[rangeIdx].repetition == numRepetitions){
        if((rangeIdx+1) < ranges.size()
          && !nextIsContinuous(ranges, rangeIdx))
        {
            int numZeroes = ranges[rangeIdx+1].offset - (ranges[rangeIdx].repetition + ranges[rangeIdx].offset);
            std::string zeroes = "";
            if(numZeroes > 0){
                for(int i = 0; i < numZeroes; i++){
                    zeroes += "00";
                }
            }
            return zeroes + recursivelyCalculateMask(ranges, rangeIdx+1, 0);
        } else {
            return recursivelyCalculateMask(ranges, rangeIdx+1, 0);
        }
    }

    // Calculate byte mask
    std::string str = intToHex(ranges[rangeIdx].mask);
    return str + recursivelyCalculateMask(ranges, rangeIdx, numRepetitions+1);

}

// Returns a hashable variable for the pext hash function
static std::string hashablePext(int hashableID, size_t offset){
    return "\t\tconst std::size_t hashable" +
                std::to_string(hashableID) +
                " = _pext_u64(load_u64_le(key.c_str()+" +
                std::to_string(offset) +
                "), mask" +
                std::to_string(hashableID) +
                ");\n";
}

// Returns a hashable variable for the naive hash function
static std::string hashableNaive(int hashableID, size_t offset){
    return "\t\tconst std::size_t hashable" +
                std::to_string(hashableID) +
                " = load_u64_le(key.c_str()+" +
                std::to_string(offset) +
                ");\n";
}

// Returns a hashable variable for the vectorized/SIMD naive hash function
static std::string hashableNaiveSIMD(int hashableID, size_t offset){
    return "\t\tconst __m128i hashable" +
                std::to_string(hashableID) +
                " = _mm_lddqu_si128((const __m128i *)(key.c_str()+" +
                std::to_string(offset) +
                "));\n";
}

/**
 * @brief Calculate the offsets for a vector of Range objects.
 *
 * This function calculates the offsets for a vector of Range objects. It does this by iterating over the ranges and
 * incrementing the current offset based on repetitions and other ranges offsets.
 *
 * @param ranges The vector of Range objects.
 * @param regSize The size of the register. Default is 8.
 * @return std::vector<size_t> The vector of calculated offsets.
 */
static std::vector<size_t> calculateOffsets(std::vector<Range>& ranges, int regSize=8){
    size_t rangesID = 0;
    std::vector<size_t> offsets;
    int currOffset = ranges[rangesID].offset;
    while( rangesID < ranges.size() ){
        offsets.push_back(currOffset);
        currOffset += regSize;
        if( currOffset >= (ranges[rangesID].offset + (int)ranges[rangesID].repetition) ){
            ++rangesID;
            if(rangesID >= ranges.size()){
                continue;
            } else if ( currOffset >= ranges[rangesID].offset ){
                while( rangesID < ranges.size() && currOffset >= (ranges[rangesID].offset + (int)ranges[rangesID].repetition)){
                    ++rangesID;
                }
            }
            if (ranges[rangesID].offset > currOffset ){
                currOffset = ranges[rangesID].offset;
            }
        }
    }
    return offsets;
}

/**
 * @brief Calculate the ranges for a given regular expression string.
 *
 * This function calculates the ranges for a given regular expression string. It does this by iterating over the characters in the
 * string and creating a Range object for each range found in the string. The offset is incremented for each character in the string
 * that is not part of a range. If no ranges are found in the string, the function prints a default function and exits.
 *
 * @param regex The regular expression string.
 * @param charEntropy The vector of character entropy values.
 * @return std::pair<std::vector<Range>,size_t> A pair containing the vector of Range objects and the final offset.
 */
static std::pair<std::vector<Range>,size_t>
calculateRanges(std::string& regex, std::vector<float>& charEntropy){
    std::vector<Range> ranges;
    size_t offset = 0;
    for(size_t i = 0; i < regex.size(); i++){
        if(regex[i] == '['){
            if(regex[i+5] == '{'){
                size_t closeBracketPos = regex.find('}', i+6);
                size_t repetition = std::stoi(regex.substr(i+6, closeBracketPos));
                ranges.push_back(Range(regex[i+1],regex[i+3],offset,repetition,charEntropy[i+1]));
                offset += repetition;
                i = closeBracketPos;
            } else {
                offset++;
                ranges.push_back(Range(regex[i+1],regex[i+3],offset,1,charEntropy[i+1]));
                i += 4;
            }
        } else if(regex[i] == '\\') {
            i++;
            offset++;
        } else {
            offset++;
        }
    }

    if(ranges.size() == 0){
        std::cout << "// No regex ranges in the key. Using default Function. \n\
            struct synthesizedHashFunc{\n\
                std::size_t operator()(const std::string& key) const {\n\
                    \treturn std::hash<std::string>{}(key);\n\
                }\n\
            }\n";
            exit(0);
    }

    return std::make_pair(ranges,offset);
}

/**
 * @brief Cascade XOR operations on variables.
 *
 * This function cascades XOR operations on variables. It does this by dequeuing two variables from the queue at a time,
 * performing an XOR operation on them, and storing the result in a temporary variable. The temporary variable is then
 * enqueued. This process continues until there is only one variable left in the queue.
 *
 * @param queue The queue of variable names.
 * @return std::string The string representation of the XOR cascade.
 */
static std::string cascadeXorVars(std::queue<std::string>& queue){
    std::string xorCascade;
    int tmpID = 0;
    while (queue.size() > 1) {
        std::string id1 = queue.front();
        queue.pop();
        std::string id2 = queue.front();
        queue.pop();
        std::string tmpVar = "tmp" + std::to_string(tmpID++);
        xorCascade += "\t\tsize_t " + tmpVar + " = " + id1 + " ^ " +id2 + ";\n";
        queue.push(tmpVar);
    }
    return xorCascade;
}

/**
 * @brief Cascade XOR operations on variables using SIMD instructions.
 *
 * This function cascades XOR operations on variables using SIMD instructions. It does this by dequeuing two variables from the queue at a time,
 * performing an XOR operation on them using the _mm_xor_si128 function, and storing the result in a temporary variable. The temporary variable is then
 * enqueued. This process continues until there is only one variable left in the queue.
 *
 * @param queue The queue of variable names.
 * @return std::string The string representation of the XOR cascade.
 */
static std::string cascadeXorVarsSIMD(std::queue<std::string>& queue){
    std::string xorCascade;
    int tmpID = 0;
    while (queue.size() > 1) {
        std::string id1 = queue.front();
        queue.pop();
        std::string id2 = queue.front();
        queue.pop();
        std::string tmpVar = "tmp" + std::to_string(tmpID++);
        xorCascade += "\t\t__m128i " + tmpVar + " = _mm_aesenc_si128(" + id1 + ", " +id2 + ");\n";
        queue.push(tmpVar);
    }
    return xorCascade;
}

/**
 * @brief Unroll masks and calculate shifts.
 *
 * This function unrolls masks and calculates shifts. It does this by iterating over the mask string in chunks of 16 characters.
 * Each chunk is converted to little-endian, shifted until there are no zeroes on the right, and then adjusted to avoid out of bounds memory access.
 * The number of zeroes in the mask is counted and stored in a vector. The mask code string is also constructed for each mask.
 *
 * @param mask The mask string.
 * @param lastMaskShift The shift for the last mask.
 * @return std::pair<std::vector<int>, std::string> A pair containing the vector of shifts and the resulting mask code strings.
 */
static std::pair<std::vector<int>, std::string>
unrollMasks_calculateShift(std::string& mask, size_t lastMaskShift){
    int maskID = 0;
    std::vector<int> shifts;
    std::string masksStr;
    for(size_t i = 0; i < mask.size(); i+=16){
        std::string currMask = mask.substr(i,16);

        // Ignore full 0 masks
        if(currMask.compare("0000000000000000") == 0){
            continue;
        }

        // Swap to little-endian
        std::swap(currMask[0], currMask[14]);
        std::swap(currMask[1], currMask[15]);
        std::swap(currMask[2], currMask[12]);
        std::swap(currMask[3], currMask[13]);
        std::swap(currMask[4], currMask[10]);
        std::swap(currMask[5], currMask[11]);
        std::swap(currMask[6], currMask[8]);
        std::swap(currMask[7], currMask[9]);

        // Shift until no zeroes on the right
        while(currMask[currMask.size()-1] == '0' && currMask[currMask.size()-2] == '0'){
            currMask = currMask.substr(0,currMask.size()-2);
            currMask = "00" + currMask;
        }

        // Fix last mask to avoid out of bounds memory access
        if(i + 16 >= mask.size()){
            for( size_t j = 0; j < lastMaskShift; j++ ) {
                currMask = currMask + "00";
            }
            currMask = currMask.substr(lastMaskShift*2,currMask.size());
        }

        long maskInt = std::stold("0x" + currMask);
        shifts.push_back(countZeros(maskInt));

        masksStr += "\t\tconstexpr std::size_t mask" +
                    std::to_string(maskID) +
                    " = 0x" +
                    currMask +
                    ";\n";
        maskID++;
    }
    return std::make_pair(shifts, masksStr);
}

/**
 * @brief Synthesize a PEXT hash function.
 *
 * This function synthesizes a PEXT hash function. It does this by taking a vector of Range objects and an offset as input.
 * The general idea of the implementation is to compress the input key into relevant bytes and only hashing them.
 *
 * @param ranges The vector of Range objects.
 * @param offset The offset.
 * @return std::string The synthesized PEXT hash function as a string.
 */
std::string synthetizePextHashFunc(std::vector<Range>& ranges, size_t offset){

    // Calculate offsets
    std::vector<size_t> offsets = calculateOffsets(ranges);

    // Avoid out of bounds memory access on the last mask/offset
    size_t lastMaskShift = 0;
    if (offsets[offsets.size()-1] + 8 >= offset){
        lastMaskShift = offsets[offsets.size()-1] - (offset - 8);
        offsets[offsets.size()-1] = offset - 8;
    }

    std::string synthesizedHashFunc = "struct synthesizedPextHash {\n\tstd::size_t operator()(const std::string& key) const {\n";

    // Calculate all concatenated masks based on ranges
    std::string _mask = recursivelyCalculateMask(ranges, 0, 0);

    std::string mask;
    for (int i = _mask.size() - 2; i >= 0; i -= 2) {
        std::string s = _mask.substr(i, 2);
        if (s == "00" && i % 16 == 0) {
            continue;
        }
        mask = s + mask;
    }


    // Add zeroes to fill 8 bytes
        // (number 16 because we are using Hex numbers, so two hex = 1 byte)
    int numZeroes = mask.size() % 16;
    if(numZeroes > 0){
        for(int i = 0; i < (16-numZeroes); i++){
            mask += "0";
        }
    }

    // Unroll Masks and calculate shifts
    std::pair<std::vector<int>, std::string> res_thank_you_cpp_compiler = unrollMasks_calculateShift(mask, lastMaskShift);
    std::vector<int> shifts = res_thank_you_cpp_compiler.first;
    synthesizedHashFunc += res_thank_you_cpp_compiler.second;

    // Create hashables
    int hashableID = 0;
    for(const auto& off : offsets){
        synthesizedHashFunc += hashablePext(hashableID++, off);
    }

    // Create hashable variables and left shift them as much as possible for better collision
    for (int i = 0; i < hashableID; ++i) {
        if (i % 2 == 0) {
            synthesizedHashFunc += "\t\tsize_t shift" + std::to_string(i) + " = " + "hashable" + std::to_string(i) + ";\n";
        } else {
            synthesizedHashFunc += "\t\tsize_t shift" + std::to_string(i) + " = " + "hashable" + std::to_string(i) + " << " + std::to_string(shifts[i]) + ";\n";
        }
    }

    // Create queue of "XORable" variables
    std::queue<std::string> queue;
    for (int i = 0; i < hashableID; ++i) {
        queue.push("shift" + std::to_string(i));
    }

    // Cascade XOR variables
    synthesizedHashFunc += cascadeXorVars(queue);

    synthesizedHashFunc += "\t\treturn " + queue.front() + "; \n";
    synthesizedHashFunc += "\t}\n};\n";

    return synthesizedHashFunc;
}

/**
 * @brief Synthesize an Offset XOR hash function.
 *
 * This function synthesizes an Offset XOR hash function. It does this by taking a vector of Range objects and an offset as input.
 * This is a naive implementation that just XORs bytes at the given offsets.
 *
 * @param ranges The vector of Range objects.
 * @param offset The offset.
 * @return std::string The synthesized Offset XOR hash function as a string.
 */
std::string synthetizeOffXorHashFunc(std::vector<Range>& ranges, size_t offset){

    // Calculate offsets
    std::vector<size_t> offsets = calculateOffsets(ranges);

    // Avoid out of bounds memory access on the last mask/offset
    if (offsets[offsets.size()-1] + 8 >= offset){
        offsets[offsets.size()-1] = offset - 8;
    }

    std::string synthesizedHashFunc = "struct synthesizedOffXorHash {\n\tstd::size_t operator()(const std::string& key) const {\n";

    // Create hashables
    int hashableID = 0;
    for(const auto& off : offsets){
        synthesizedHashFunc += hashableNaive(hashableID++, off);
    }

    // Create queue of "XORable" variables
    std::queue<std::string> queue;
    for (int i = 0; i < hashableID; ++i) {
        queue.push("hashable" + std::to_string(i));
    }

    // Cascade XOR variables
    synthesizedHashFunc += cascadeXorVars(queue);

    synthesizedHashFunc += "\t\treturn " + queue.front() + "; \n";
    synthesizedHashFunc += "\t}\n};\n";

    return synthesizedHashFunc;
}

/**
 * @brief Synthesize an Offset XOR SIMD hash function.
 *
 * This function synthesizes an Offset XOR SIMD hash function. It does this by taking a vector of Range objects and an offset as input.
 * This is a Naive implementation that uses SIMD with the _mm_xor_si128 intrinsic to XOR bytes at the given offsets.
 *
 * @param ranges The vector of Range objects.
 * @param offset The offset.
 * @return std::string The synthesized Offset XOR SIMD hash function as a string.
 */
std::string synthetizeAesHashFunc(std::vector<Range>& ranges, size_t offset) {
    // Calculate offsets
    std::vector<size_t> offsets = calculateOffsets(ranges, 16);

    // Avoid out of bounds memory access on the last mask/offset
    if (offsets[offsets.size()-1] + 16 >= offset){
        offsets[offsets.size()-1] = offset - 16;
    }

    std::string synthesizedHashFunc = "struct synthesizeAesHash {\n\tstd::size_t operator()(const std::string& key) const {\n";

    // Create hashables
    int hashableID = 0;
    for(const auto& off : offsets){
        synthesizedHashFunc += hashableNaiveSIMD(hashableID++, off);
    }

    // Create queue of "XORable" variables
    std::queue<std::string> queue;
    for (int i = 0; i < hashableID; ++i) {
        queue.push("hashable" + std::to_string(i));
    }

    // Cascade XOR variables
    synthesizedHashFunc += cascadeXorVarsSIMD(queue);

    synthesizedHashFunc += "\t\treturn _mm_extract_epi64("+queue.front()+", 0) ^ _mm_extract_epi64("+queue.front()+" , 1); \n";
    synthesizedHashFunc += "\t}\n};\n";
    synthesizedHashFunc = "#include <immintrin.h>\n#include <wmmintrin.h>\n" + synthesizedHashFunc;

    return synthesizedHashFunc;
}

/**
 * @brief Entry point of the program.
 *
 * This is the main function of the program. It processes command line arguments, which are expected to be a regular expression representing the key format.
 * The regular expression should be generated by the keybuilder component of this project.
 * The function then initiates the appropriate processes based on the provided arguments.
 *
 * @param argc The count of command line arguments.
 * @param argv An array of command line arguments.
 * @return int Returns zero upon successful program execution, non-zero values indicate an error.
 */
int main(int argc, char** argv){

    std::string regexStr = std::string(argv[1]);
    std::vector<float> charEntropy;
    for(int i = 2; i < argc; i++){
        charEntropy.push_back(std::stof(argv[i]));
    }

    // Create ranges
    size_t offset;
    std::vector<Range> ranges;
    std::pair<std::vector<Range>,size_t> res = calculateRanges(regexStr,charEntropy);
    ranges = res.first;
    offset = res.second;

    constexpr float ENTROPY_THRESHOLD = 4.0;
    // Remove all ranges that have lower than ENTROPY_THRESHOLD entropy
    ranges.erase(std::remove_if(ranges.begin(), ranges.end(), [ENTROPY_THRESHOLD](const Range& range) {
        return range.entropy < ENTROPY_THRESHOLD;
    }), ranges.end());

    size_t regexSize = 0;
    for(const auto& range : ranges){
        regexSize += range.repetition;
        regexSize += range.offset;
    }

    // load_u64_le function header
    printf("// Helper function, include in your codebase:\n");
    printf("%s\n", load_u64_le.c_str());

    printf("// Pext Hash Function:\n");
    printf("%s\n", synthetizePextHashFunc(ranges,offset).c_str());
    printf("// (Recommended) OffXor Hash Function:\n");
    printf("%s", synthetizeOffXorHashFunc(ranges,offset).c_str());
    if(regexSize > 32){
        printf("// Aes Hash Function:\n");
        printf("%s", synthetizeAesHashFunc(ranges,offset).c_str());
    }

    return 0;
}
