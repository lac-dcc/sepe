
#define DECLARE_ONE_BENCH(name, hashname) (Benchmark*)new name<hashname>(#name,#hashname)

// Define Macro to set the benchmarks and HashInfo
#define REGISTER_BENCHMARKS(hashname)   benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedMapBench, hashname)); \
                                        benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedMultiMapBench, hashname)); \
                                        benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedSetBench, hashname)); \
                                        benchmarks.push_back(DECLARE_ONE_BENCH(UnorderedMultisetBench, hashname));

#define REGISTER_ALL_BENCHMARKS \
    REGISTER_BENCHMARKS(AbseilHash); \
    REGISTER_BENCHMARKS(STDHashBin); \
    REGISTER_BENCHMARKS(STDHashSrc); \
    REGISTER_BENCHMARKS(FNVHash); \
    REGISTER_BENCHMARKS(CityHash); \
    REGISTER_BENCHMARKS(SSNHashBitOps); \
    REGISTER_BENCHMARKS(CPFHashBitOps); \
    REGISTER_BENCHMARKS(CPFHashVectorizedMul); \
    REGISTER_BENCHMARKS(IPV4HashUnrolled); \
    REGISTER_BENCHMARKS(IPV4HashMove); \
    REGISTER_BENCHMARKS(IPV4HashBitOps); \
    REGISTER_BENCHMARKS(CarPlateHashBitOps); \
    REGISTER_BENCHMARKS(MacAddressHashBitOps); \
    REGISTER_BENCHMARKS(IntSimdHash); \
    REGISTER_BENCHMARKS(IntBitHash); \
    REGISTER_BENCHMARKS(UrlCompress); \
    REGISTER_BENCHMARKS(PextUrlComplex); \
    REGISTER_BENCHMARKS(PextUrl); \
    REGISTER_BENCHMARKS(PextMac); \
    REGISTER_BENCHMARKS(PextCPF); \
    REGISTER_BENCHMARKS(PextSSN); \
    REGISTER_BENCHMARKS(PextIPV4); \
    REGISTER_BENCHMARKS(PextIPV6); \
    REGISTER_BENCHMARKS(PextINTS); \
    REGISTER_BENCHMARKS(OffXorUrlComplex); \
    REGISTER_BENCHMARKS(OffXorUrl); \
    REGISTER_BENCHMARKS(OffXorMac); \
    REGISTER_BENCHMARKS(OffXorCPF); \
    REGISTER_BENCHMARKS(OffXorSSN); \
    REGISTER_BENCHMARKS(OffXorIPV4); \
    REGISTER_BENCHMARKS(OffXorIPV6); \
    REGISTER_BENCHMARKS(OffXorINTS); \
    REGISTER_BENCHMARKS(NaiveUrlComplex); \
    REGISTER_BENCHMARKS(NaiveUrl); \
    REGISTER_BENCHMARKS(NaiveMac); \
    REGISTER_BENCHMARKS(NaiveCPF); \
    REGISTER_BENCHMARKS(NaiveSSN); \
    REGISTER_BENCHMARKS(NaiveIPV4); \
    REGISTER_BENCHMARKS(NaiveIPV6); \
    REGISTER_BENCHMARKS(NaiveINTS); \
    REGISTER_BENCHMARKS(OffXorSimdUrlComplex); \
    REGISTER_BENCHMARKS(OffXorSimdUrl); \
    REGISTER_BENCHMARKS(OffXorSimdIPV6); \
    REGISTER_BENCHMARKS(OffXorSimdINTS); \
    REGISTER_BENCHMARKS(NaiveSimdUrlComplex); \
    REGISTER_BENCHMARKS(NaiveSimdUrl); \
    REGISTER_BENCHMARKS(NaiveSimdIPV6); \
    REGISTER_BENCHMARKS(NaiveSimdINTS); \
    REGISTER_BENCHMARKS(GptUrlComplex); \
    REGISTER_BENCHMARKS(GptUrl); \
    REGISTER_BENCHMARKS(GptMac); \
    REGISTER_BENCHMARKS(GptCPF); \
    REGISTER_BENCHMARKS(GptSSN); \
    REGISTER_BENCHMARKS(GptIPV4); \
    REGISTER_BENCHMARKS(GptIPV6); \
    REGISTER_BENCHMARKS(GptINTS); \
    REGISTER_BENCHMARKS(GperfUrlComplex); \
    REGISTER_BENCHMARKS(GperfUrl); \
    REGISTER_BENCHMARKS(GperfMac); \
    REGISTER_BENCHMARKS(GperfCPF); \
    REGISTER_BENCHMARKS(GperfSSN); \
    REGISTER_BENCHMARKS(GperfIPV4); \
    REGISTER_BENCHMARKS(GperfIPV6); \
    REGISTER_BENCHMARKS(GperfINTS);