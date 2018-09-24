
#define cos me_cos
#define cosf me_cosf
#define sin me_sin
#define sinf me_sinf
#define atan me_atan
#define atanf me_atanf
#define fabs me_fabs
#define fabsf me_fabsf
#include "../src/math.cpp"
#undef cos
#undef cosf
#undef sin
#undef sinf
#undef atan
#undef atanf
#undef fabs
#undef fabsf

#include <stdio.h>

#define _USE_MATH_DEFINES 
#include <math.h>

int main(int argc, char **argv) {
	s32 cuts = 32;
	float step = (M_PI * 2.0f) / (float) cuts;
	for (s32 i = 0; i < cuts; ++i) {
		float result_libm  = cosf(step * i);
		float result_me = me_cosf(step * i);
		printf("CUT: %f, libm (%f), me (%f)\n", step * i, result_libm, result_me);
	}

	printf("\n\n");

	for (s32 i = 0; i < cuts; ++i) {
		float result_libm  = sinf(step * i);
		float result_me = me_sinf(step * i);
		printf("CUT: %f, libm (%f), me (%f)\n", step * i, result_libm, result_me);
	}

	printf("\n\n");

	for (s32 i = -cuts; i < cuts; ++i) {
		float result_libm  = atanf(step * i);
		float result_me = me_atanf(step * i);
		printf("CUT: %f, libm (%f), me (%f)\n", step * i, result_libm, result_me);
	}
}
