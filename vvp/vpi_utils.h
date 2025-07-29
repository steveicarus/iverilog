#ifndef IVL_vpi_utils_H
#define IVL_vpi_utils_H

#include <cmath>
#include <cstdint>

static inline uint64_t vlg_round_to_u64(double rval)
{
      // Directly casting a negative double to an unsigned integer types is
      // undefined behavior and behaves differently on different architectures.
      // Cast to signed integer first to get the behavior we want.
      return static_cast<uint64_t>(static_cast<int64_t>(std::llround(rval)));
}

#endif
