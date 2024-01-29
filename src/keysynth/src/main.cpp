#include <iostream>
#include <vector>
#include <queue>
#include <utility>

static const std::string load_u64_le = R"(inline static uint64_t load_u64_le(const char* b) {
    uint64_t Ret;
    // This is a way for the compiler to optimize this func to a single movq instruction
    memcpy(&Ret, b, sizeof(uint64_t)); 
    return Ret;
})";

static std::string intToHex(size_t num){
    char numStr[17];
    sprintf(numStr, "%02lx", num);
    return std::string(numStr);
}

struct Range{
    char start;
    char end;
    int offset;
    size_t repetition;
    char mask;

    Range(char _start, char _end, int _offset, size_t _repetition) : 
        start(_start), 
        end(_end), 
        offset(_offset),
        repetition(_repetition)
    {
        // Calculate mask
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
            mask = 0x7F;
        }
    }

    void print(){
        printf("Range: %c - %c : offset %d repetition %lu\n", start, end, offset, repetition);
    }
    
};

static inline int countZeros(const size_t x) {
	int zeros = 0;
	for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
		if ((x & (1L << i)) == 0) {
			++zeros;
		}
	}
	return zeros;
}

static bool nextIsContinuous(const std::vector<Range>& ranges, size_t rangeIdx){
    return ((size_t)ranges[rangeIdx+1].offset == ranges[rangeIdx].offset + ranges[rangeIdx].repetition);
}

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

static std::string hashablePext(int hashableID, size_t offset){
    return "\tconst std::size_t hashable" + 
                std::to_string(hashableID) + 
                " = _pext_u64(load_u64_le(key.c_str()+" + 
                std::to_string(offset) + 
                "), mask" + 
                std::to_string(hashableID) + 
                ");\n";
}

static std::string hashableNaive(int hashableID, size_t offset){
    return "\tconst std::size_t hashable" + 
                std::to_string(hashableID) + 
                " = load_u64_le(key.c_str()+" + 
                std::to_string(offset) + 
                ");\n";
}

static std::string hashableNaiveSIMD(int hashableID, size_t offset){
    return "\tconst __m128i hashable" + 
                std::to_string(hashableID) + 
                " = _mm_lddqu_si128((const __m128i *)(key.c_str()+" + 
                std::to_string(offset) + 
                "));\n";
}

static std::vector<size_t> calculateOffsets(std::vector<Range>& ranges, int regSize=8){
    size_t rangesID = 0;
    std::vector<size_t> offsets;
    int currOffset = ranges[rangesID].offset;
    while( rangesID < ranges.size() ){
        // synthesizedHashFunc += hashablePext(hashableID++, currOffset);
        offsets.push_back(currOffset);
        currOffset += regSize;
        if( currOffset >= (ranges[rangesID].offset + (int)ranges[rangesID].repetition) ){
            rangesID++;
            if(rangesID >= ranges.size()){
                continue;
            }
            if( currOffset >= ranges[rangesID].offset ){
                while( rangesID < ranges.size() && currOffset >= (ranges[rangesID].offset + (int)ranges[rangesID].repetition)){
                    ++rangesID;
                }
                continue;
            } else {
                currOffset = ranges[rangesID].offset;
            }
        }
    }
    return offsets;
}

static std::pair<std::vector<Range>,size_t> 
calculateRanges(std::string& regex){
    std::vector<Range> ranges;
    size_t offset = 0;
    for(size_t i = 0; i < regex.size(); i++){
        if(regex[i] == '['){
            if(regex[i+5] == '{'){
                size_t closeBracketPos = regex.find('}', i+6);
                size_t repetition = std::stoi(regex.substr(i+6, closeBracketPos));
                ranges.push_back(Range(regex[i+1],regex[i+3],offset,repetition));
                offset += repetition;
                i = closeBracketPos;
            } else {
                offset++;
                ranges.push_back(Range(regex[i+1],regex[i+3],offset,1));
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
			std::size_t synthesizedHashFunc::operator()(const std::string& key) const {\n\
    			\treturn std::hash<std::string>{}(key);\n\
			}";
            exit(0);
    }

    return std::make_pair(ranges,offset);
}


static std::string cascadeXorVars(std::queue<std::string>& queue){
    std::string xorCascade;
	int tmpID = 0;
	while (queue.size() > 1) {
		std::string id1 = queue.front();
		queue.pop();
		std::string id2 = queue.front();
		queue.pop();
		std::string tmpVar = "tmp" + std::to_string(tmpID++);
		xorCascade += "\tsize_t " + tmpVar + " = " + id1 + " ^ " +id2 + ";\n";
		queue.push(tmpVar);
	}
    return xorCascade;
}

static std::string cascadeXorVarsSIMD(std::queue<std::string>& queue){
    std::string xorCascade;
	int tmpID = 0;
	while (queue.size() > 1) {
		std::string id1 = queue.front();
		queue.pop();
		std::string id2 = queue.front();
		queue.pop();
		std::string tmpVar = "tmp" + std::to_string(tmpID++);
		xorCascade += "\t__m128i " + tmpVar + " = _mm_xor_si128(" + id1 + ", " +id2 + ");\n";
		queue.push(tmpVar);
	}
    return xorCascade;
}

// Yes, this functions does two things. How about that clean coders?
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
		shifts.push_back(countZeros(maskInt)-1);

        masksStr += "\tconstexpr std::size_t mask" + 
                    std::to_string(maskID) + 
                    " = 0x" + 
                    currMask + 
                    ";\n";
        maskID++;
    }
    return std::make_pair(shifts, masksStr);
}

std::string synthethisePextHashFunc(std::vector<Range>& ranges, size_t offset){

    // Calculate offsets
    std::vector<size_t> offsets = calculateOffsets(ranges);

    // Avoid out of bounds memory access on the last mask/offset
    size_t lastMaskShift = 0;
    if (offsets[offsets.size()-1] + 8 >= offset){
        lastMaskShift = offsets[offsets.size()-1] - (offset - 8);
        offsets[offsets.size()-1] = offset - 8;
    }

    std::string synthesizedHashFunc = "std::size_t synthesizedPextHash(const std::string& key) const {\n";
    
    // Calculate all concatenated masks based on ranges
    std::string mask = recursivelyCalculateMask(ranges, 0, 0);

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
			synthesizedHashFunc += "\tsize_t shift" + std::to_string(i) + " = " + "hashable" + std::to_string(i) + ";\n";
		} else {
			synthesizedHashFunc += "\tsize_t shift" + std::to_string(i) + " = " + "hashable" + std::to_string(i) + " << " + std::to_string(shifts[i]) + ";\n";
		}
	}

    // Create queue of "XORable" variables
	std::queue<std::string> queue;
	for (int i = 0; i < hashableID; ++i) {
		queue.push("shift" + std::to_string(i));
	}

    // Cascade XOR variables
    synthesizedHashFunc += cascadeXorVars(queue);

    synthesizedHashFunc += "\treturn " + queue.front() + "; \n";
    synthesizedHashFunc += "}\n";

    // load_u64_le function header
    synthesizedHashFunc = load_u64_le + "\n" + synthesizedHashFunc;

    return synthesizedHashFunc;
}

std::string synthethiseOffXorHashFunc(std::vector<Range>& ranges, size_t offset){

    // Calculate offsets
    std::vector<size_t> offsets = calculateOffsets(ranges);

    // Avoid out of bounds memory access on the last mask/offset
    if (offsets[offsets.size()-1] + 8 >= offset){
        offsets[offsets.size()-1] = offset - 8;
    }

    std::string synthesizedHashFunc = "std::size_t synthesizedOffXorHash(const std::string& key) const {\n";

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

    synthesizedHashFunc += "\treturn " + queue.front() + "; \n";
    synthesizedHashFunc += "}\n";

    return synthesizedHashFunc;
}

std::string synthethiseOffXorSimdFunc(std::vector<Range>& ranges, size_t offset) {
    // Calculate offsets
    std::vector<size_t> offsets = calculateOffsets(ranges, 16);

    // Avoid out of bounds memory access on the last mask/offset
    if (offsets[offsets.size()-1] + 16 >= offset){
        offsets[offsets.size()-1] = offset - 16;
    }

    std::string synthesizedHashFunc = "std::size_t synthesizeOffXorSimdHash(const std::string& key) const {\n";

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

    synthesizedHashFunc += "\treturn _mm_extract_epi64("+queue.front()+", 0) ^ _mm_extract_epi64("+queue.front()+" , 1); \n";
    synthesizedHashFunc += "}\n";
    synthesizedHashFunc =  "#include <immintrin.h>\n" + synthesizedHashFunc;

    return synthesizedHashFunc;
}

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "Incorrect arguments!\n"
                "Usage: %s <regex>\n", argv[0]);
        return 1;
    }

    std::string regexStr = std::string(argv[1]);

    // Create ranges
    size_t offset;
    std::vector<Range> ranges;
    std::pair<std::vector<Range>,size_t> res = calculateRanges(regexStr);
    ranges = res.first;
    offset = res.second;

    printf("%s\n", synthethisePextHashFunc(ranges,offset).c_str());
    if(regexStr.size() < 32){
        printf("%s", synthethiseOffXorHashFunc(ranges,offset).c_str());
    } else {
        printf("%s", synthethiseOffXorSimdFunc(ranges,offset).c_str());
    }

    return 0;

}
