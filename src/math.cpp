
// simple slow Taylor series expansions of trig functions
// if we want speed, we should cache the result into a lookup table
// or use hardware trig functions or implement a CORDIC function

#include "math.h"

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
    
    // double fmod(double numer, double denom) {
    // 	double tquot = (double)(s64)(numer / denom);
    // 	return numer - tquot * denom;
    // }
    
    double fabs(double x) {
        if (x < 0) return -x;
        return x;
    }
    
    float fabsf(float x) {
        return (float) fabs((double) x);
    }
    
    double cos(double x) {
        u32 iterations = 32;
        
        double result = 1.0;
        
        double step = x*x;
        double numerator = 1.0;
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
    
    
    double sin(double x) {
        u32 iterations = 32;
        
        double result = x;
        
        double step = x*x;
        double numerator = x;
        double denom = 1.0;
        
        
        for (u32 i = 1; i < iterations; ++i) {
            numerator = -(numerator * step);
            
            denom = denom * ((double) (i*2) * (double) (i*2 + 1));
            
            result += numerator / denom;
        }
        
        return result;
    }
    
    float sinf(float x) {
        return (float) sin((double) x);
    }
    
    double atan(double x) {
        if (x > 1.0)  return (M_PI / 2.0) - atan(1.0 / x);
        if (x < -1.0) return (M_PI / 2.0) + atan(1.0 / x);
        
        u32 iterations = 32;
        
        double result = x;
        
        double step = x*x;
        double numerator = x;
        double denom = 1.0;
        
        
        for (u32 i = 1; i < iterations; ++i) {
            numerator = -(numerator * step);
            
            denom = (double) (i*2 + 1);
            
            result += numerator / denom;
        }
        
        return result;
    }
    
    float atanf(float x) {
        return (float) atan((double) x);
    }
    
    float roundf(float x) {
        return (float)roundl((long double)x);
    }
    
    double round(double x) {
        return (double)roundl((long double)x);
    }
    
    long double roundl(long double x) {
        long double integer = (long double)(s64)x;
        long double frac = x - integer;
        
        if (frac >= 0.5L) return integer + 1.0;
        return integer;
        
    }
    
#ifdef __cplusplus
}
#endif