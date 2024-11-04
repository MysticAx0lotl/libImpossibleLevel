#ifdef _MSC_VERS
# include <stdlib.h>
# define BSWAP16(val) _byteswap_ushort(val)
# define BSWAP32(val) _byteswap_ulong(val)
# define BSWAP64(val) _byteswap_uint64(val)
#else
#include <stdint.h>
#include <bit>
# define BSWAP16(val) ((val >> 8) | (val << 8))
# define BSWAP32(val) __builtin_bswap32(val)
# define BSWAP64(val) __builtin_bswap64(val)
#endif

namespace util::data {
#ifndef __cpp_lib_byteswap
    uint16_t byteswapU16(uint16_t val) {
        return BSWAP16(val);
    }

    uint32_t byteswapU32(uint32_t val) {
        return BSWAP32(val);
    }

    uint64_t byteswapU64(uint64_t val) {
        return BSWAP64(val);
    }
#endif // __cpp_lib_byteswap
};

#undef BSWAP16
#undef BSWAP32
#undef BSWAP64