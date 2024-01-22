#include <iostream>
#include <vector>
#include <queue>

static const std::string load_u64_le = R"(inline static uint64_t load_u64_le(const char* b) {
    uint64_t Ret;
    // This is a way for the compiler to optimize this func to a single movq instruction
    memcpy(&Ret, b, sizeof(uint64_t)); 
    return Ret;
})";

static inline std::string intToHex(size_t num){
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

int countZeros(const size_t x) {
	int zeros = 0;
	for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
		if ((x & (1L << i)) == 0) {
			++zeros;
		}
	}
	return zeros;
}

inline bool isAlphanumerical(char c){
    return (c >= '0' && c <= '9') || 
           (c >= 'a' && c <= 'z') || 
           (c >= 'A' && c <= 'Z');
}

inline bool nextIsContinuous(const std::vector<Range>& ranges, size_t rangeIdx){
    return ((size_t)ranges[rangeIdx+1].offset == ranges[rangeIdx].offset + ranges[rangeIdx].repetition);
}

std::string recursivelyCalculateMask(const std::vector<Range>& ranges, size_t rangeIdx, size_t numRepetitions){

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

static inline std::string hashable(int hashableID, size_t offset){
    return "\tconst std::size_t hashable" + 
                std::to_string(hashableID) + 
                " = _pext_u64(load_u64_le(key.c_str()+" + 
                std::to_string(offset) + 
                "), mask" + 
                std::to_string(hashableID) + 
                ");\n";
}

std::string synthethiseHashFunc(std::string& regex){

    // Create ranges
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
		return "\
			std::size_t synthethisedHashFunc::operator()(const std::string& key) const {\n\
    			\treturn std::hash<std::string>{}(key);\n\
			}";
    }

    // Calculate offsets
    std::vector<size_t> offsets;
    size_t rangesID = 0;
    int currOffset = ranges[rangesID].offset;
    while( rangesID < ranges.size() ){
        // synthethisedHashFunc += hashable(hashableID++, currOffset);
        offsets.push_back(currOffset);
        currOffset += 8;
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

    // Avoid out of bounds memory access on the last mask/offset
    size_t lastMaskShift = 0;
    if (offsets[offsets.size()-1] + 8 >= offset){
        lastMaskShift = offsets[offsets.size()-1] - (offset - 8);
        offsets[offsets.size()-1] = offset - 8;
    }

    std::string synthethisedHashFunc = "std::size_t synthethisedHashFunc(const std::string& key) const {\n";
    
    std::string mask = recursivelyCalculateMask(ranges, 0, 0);
    // printf("Mask: %s\n", mask.c_str());
    int numZeroes = mask.size() % 16;
    if(numZeroes > 0){
        for(int i = 0; i < (16-numZeroes); i++){
            mask += "0";
        }
    }

    int maskID = 0;
	std::vector<int> shifts;
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

        synthethisedHashFunc += "\tconstexpr std::size_t mask" + 
                                std::to_string(maskID) + 
                                " = 0x" + 
                                currMask + 
                                ";\n";
        maskID++;
    }

    int hashableID = 0;
    for(const auto& off : offsets){
        synthethisedHashFunc += hashable(hashableID++, off);
    }

	for (int i = 0; i < hashableID; ++i) {
		if (i % 2 == 0) {
			synthethisedHashFunc += "\tsize_t shift" + std::to_string(i) + " = " + "hashable" + std::to_string(i) + ";\n";
		} else {
			synthethisedHashFunc += "\tsize_t shift" + std::to_string(i) + " = " + "hashable" + std::to_string(i) + " << " + std::to_string(shifts[i]) + ";\n";
		}
	}

	std::queue<std::string> queue;
	for (int i = 0; i < hashableID; ++i) {
		queue.push("shift" + std::to_string(i));
	}


	int tmpID = 0;
	while (queue.size() > 1) {
		std::string id1 = queue.front();
		queue.pop();
		std::string id2 = queue.front();
		queue.pop();
		std::string tmpVar = "tmp" + std::to_string(tmpID++);
		synthethisedHashFunc += "\tsize_t " + tmpVar + " = " + id1 + " ^ " +id2 + ";\n";
		queue.push(tmpVar);
	}

    synthethisedHashFunc += "\treturn " + queue.front() + "; \n";
    synthethisedHashFunc += "}\n";
    return synthethisedHashFunc;
}

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "Incorrect arguments!\n"
                "Usage: %s <regex>\n", argv[0]);
        return 1;
    }

    std::string regexStr = std::string(argv[1]);

    printf("%s", synthethiseHashFunc(regexStr).c_str());

    return 0;

}
