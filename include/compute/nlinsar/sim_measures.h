// M_PI_F is define by opencl, if we do no
#ifndef M_PI_F
#define _USE_MATH_DEFINES
#include <math.h>
#define M_PI_F M_PI
#endif

float pixel_similarity(float am1, float as1, float dp1,
                       float am2, float as2, float dp2)
{
    const float A = pow(am1*am1 + as1*as1 + am2*am2 + as2*as2, 2.0f);
    const float B = 4.0f * (am1*am1*as1*as1 + am2*am2*as2*as2 + 2.0f*am1*as1*am2*as2*cos(dp1-dp2));
    const float C = am1*as1*am2*as2;
    const float f_el = pow(C/B, 1.5f) * ( (A+B)/A * sqrt(B/(A-B)) - asin(sqrt(B/A)));

    return log(f_el);
}

float pixel_kullback_leibler(float ref1, float dp1, float coh1,
                             float ref2, float dp2, float coh2) 
{
    // test if ref1 or ref 2 are zero
    const float a1 = ref2 ? ref1/ref2 * (1.0f-coh1*coh2*cos(dp1-dp2))/(1.0f-coh2*coh2) : 1.0f;
    const float a2 = ref1 ? ref2/ref1 * (1.0f-coh1*coh2*cos(dp1-dp2))/(1.0f-coh1*coh1) : 1.0f;
    return -4.0f/M_PI_F*(a1+a2-2.0f);
}