
#define cos me_cos
#define cosf me_cosf
#include "../src/math.cpp"
#undef cos
#undef cosf
#include <stdio.h>

#define _USE_MATH_DEFINES 
#include <math.h>

int main(int argc, char **argv) {
	u32 cuts = 32;
	float step = (M_PI * 2.0f) / (float) cuts;
	for (u32 i = 0; i < cuts; ++i) {
		float result_libm  = cosf(step * i);
		float result_me = me_cosf(step * i);
		printf("CUT: %f, libm (%f), me (%f)\n", step * i, result_libm, result_me);
	}
}
