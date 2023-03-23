#include <typedefs.hpp>

// bit-fiddling.hpp
namespace kel {
# define pow2(nbits, power) (static_cast<u##nbits##>(1) << (power))
# define ones(nbits, num) (pow2(nbits, num) - static_cast<u##nbits##>(1))
# define bitRange(nbits, start, stop) (ones(nbits, stop) & (~ones(nbits, start)))

# define getLS1B(mask) ((mask) & -(mask))
# define clearLS1B(mask) (mask &= (mask - static_cast<decltype(mask)>(1)))
}
