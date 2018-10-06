
#ifndef MATH_H
#define MATH_H

#ifdef __cplusplus
extern "C" {
#endif
    
    float cosf(float x);
    double cos(double x);
    
    float sinf(float x);
    double sin(double x);
    
    float tanf(float x);
    double tan(double x);
    
    float fabsf(float x);
    double fabs(double x);
    
    
    float roundf(float x);
    double round(double x);
    long double roundl(long double x);
    // @TODO there's many more rounding functions
    
    
#define M_PI 3.14159265358979323846264338327950288
    
#ifdef __cplusplus
}
#endif

#endif
