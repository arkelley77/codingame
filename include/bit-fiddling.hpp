#include <typedefs.hpp>

// bit-fiddling.hpp
namespace kel {
#define pow2(power) (1ull << (power))
#define ones(num) (pow2(num) - 1ull)
#define bitRange(nbits, start, stop) (ones(nbits, stop) & (~ones(nbits, start)))

#define getLS1B(mask) ((mask) & -(mask))
#define clearLS1B(mask) (mask &= (mask - static_cast<decltype(mask)>(1)))
}
