#ifndef DSP_UTILS_H
#define DSP_UTILS_H

#include <cstdint>
#include "mathTypes.h"
#include <cmath>
#include <string.h>

void set_tmp_dir(const char* temp_dir_full_path);
void file_dump( void* data, uint32_t size8, const char* fname, const char* ext, int32_t idx , bool append = false );
void file_dump( void* data, uint32_t size8, const char* fname );


float* make_magnitude_spectrum( float_cpx_t* data, int samples );
void conjugate( float_cpx_t* data, int samples );
void mul_vectors( const float_cpx_t* A, const float_cpx_t* B, float_cpx_t* result, int len );
void add_vector(float *dst, const float *B, int len );
void add_vector(float_cpx_t *dst, const float_cpx_t *B, int len );
void get_lengths(const float_cpx_t* A, float* result, int len , double scale);
float get_mean( const float* A, int len );
float_cpx_t* freq_shift(const float_cpx_t *signal, uint32_t samples_count, double SR_hz, double freq_shift_hz);
float_cpx_t* make_fir(const float_cpx_t *S, const float *fir, int out_len, int fir_len );
float_cpx_t calc_correlation( float_cpx_t* A, float_cpx_t* B, int len );
void mul_vec( float_cpx_t* A, float_cpx_t k, int len );

template< typename data_t >
data_t* circle_shift( data_t* data, int samples, int rotate_idx ) {
    data_t* out = new data_t[ samples ];

    rotate_idx %= samples;

    // I think some implementation of C can make negative modules.
    if ( rotate_idx < 0 ) {
        rotate_idx += samples;
    }

    memcpy( out, data + rotate_idx, ( samples - rotate_idx ) * sizeof( data_t ) );
    memcpy( out + ( samples - rotate_idx ), data, ( rotate_idx ) * sizeof( data_t ) );

    return out;
}

#endif // DSP_UTILS_H
