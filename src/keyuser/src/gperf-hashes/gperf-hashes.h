#ifndef GPERF_HASHES_HPP
#define GPERF_HASHES_HPP

#include <stdlib.h>

unsigned int
GperfUrlComplexHash (const char *str, size_t len);

unsigned int
GperfUrlHash (const char *str, size_t len);

unsigned int
GperfUrlHash (const char *str, size_t len);

unsigned int
GperfMACHash (const char *str, size_t len);

unsigned int
GperfCPFHash (const char *str, size_t len);

unsigned int
GperfSSNHash (const char *str, size_t len);

unsigned int
GperfIPV4Hash (const char *str, size_t len);

unsigned int
GperfIPV6Hash (const char *str, size_t len);

unsigned int
GperfINTSHash (const char *str, size_t len);

#endif