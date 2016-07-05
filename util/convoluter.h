#ifndef _convoluter_h_
#define _convoluter_h_

#include <cstdint>
#include "mathTypes.h"

void test_conv();

class Convoluter {
public:
    static float conv_real_cpx( float* real_in, float_cpx_t* cpx_in, uint32_t pts_cnt );
    static float conv_real_cpx2( float* real_in, float_cpx_t* cpx_in, uint32_t pts_cnt );
    static float conv_multiple_real_cpx(float* base_single_in, float_cpx_t* multiple_linear_in, float* conv_out,
                                         uint32_t one_conv_len, uint32_t conv_cnt, uint32_t step_base , uint32_t linear_in_distance = 0);
};


float_cpx_t* gen_carrier( float SR_hz, float sub_freq, uint32_t samples_count );
float_cpx_t* modulate_bpsk(float SR_hz, float speed_cps, uint32_t pts_cnt, const float_cpx_t* carrier, const int32_t* code, int idx = 0 );


#endif
