
// simple slow Taylor series expansions of trig functions
// if we want speed, we should cache the result into a lookup table
// or use hardware trig functions or implement a CORDIC function

#include <stdint.h>
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#ifdef __cplusplus
extern "C" {
#endif

double cos(double x) {
	u32 iterations = 32;

	double result = 1.0f;

	double step = x*x;
	double numerator = 1.0f;
	double denom = 1.0;


	for (u32 i = 1; i < iterations; ++i) {
		numerator = -(numerator * step);

		denom = denom * ((double) (i*2 - 1) * (double) (i*2));

		result += numerator / denom;
	}

	return result;
}

float cosf(float x) {
	return (float) cos((double) x);
}


#ifdef __cplusplus
}
#endif