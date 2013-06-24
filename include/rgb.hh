#ifndef RGB_HH
#define RGB_HH

#include <cstdint>

typedef uint8_t rgb_t[3];
typedef int32_t rgb_sum_t[3];

struct rgb {
	constexpr static uint32_t tou32(const rgb_t v) {
		return ((uint32_t)v[0]<<16) | ((uint32_t)v[1]<<8) | (uint32_t)v[2];
	}
	constexpr static bool equals(const rgb_t c1, const rgb_t c2) {
		return c1[0] == c2[0] && c1[1] == c2[1] && c1[2] == c2[2];
	}
};

#endif
