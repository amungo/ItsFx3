#include "dsp_utils.h"
#include "stdio.h"
#include <cstdint>
#include "string.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static const char* DIR_TMP = "c:\\tmp";


void set_tmp_dir(const char * temp_dir_full_path ) {
    DIR_TMP = temp_dir_full_path;
}


void file_dump( void* data, uint32_t size8, const char* fname, const char* ext, int32_t idx, bool append ) {

    char fn[ 1024 ];
    sprintf( fn, "%s\\%s_%d.%s", DIR_TMP, fname, idx, ext );

    if ( size8 == 0 ) {
        FILE* f = fopen( fn, "w" );
        if ( f ) {
            fclose( f );
        }
        return;
    }

    const char* mode = "wb";
    if ( append ) {
        mode = "ab";
    }
    FILE* f = fopen( fn, mode );
    if ( f ) {
        fwrite( data, 1, size8, f );
        fclose( f );
    }
}

void file_dump(void *data, uint32_t size8, const char *fname) {
    char fn[ 1024 ];
    sprintf( fn, "%s\\%s", DIR_TMP, fname );
    FILE* f = fopen( fn, "wb" );
    if ( f ) {
        fwrite( data, 1, size8, f );
        fclose( f );
    }
}

float *make_magnitude_spectrum(float_cpx_t *data, int samples) {
    float* mag = new float[ samples ];
    for ( int i = 0; i < samples; i++ ) {
        float x = data[ ( samples / 2 + i  ) % samples ].len_squared();
        if ( x == 0.0f ) {
            mag[ i ] = 0.0f;
        } else {
            mag[ i ] = 10.0f * log10( x );
        }
    }
    return mag;
}


void conjugate(float_cpx_t *data, int samples) {
    for ( int i = 0; i < samples; i++ ) {
        data[ i ].q = -data[ i ].q;
    }
}

void mul_vectors(const float_cpx_t *A, const float_cpx_t *B, float_cpx_t *result, int len) {
    for ( int i = 0; i < len; i++ ) {
        result[ i ] = A[ i ].mul_cpx_const( B[ i ] );
    }
}

void add_vector( float* dst, const float* B, int len ) {
    for ( int i = 0; i < len; i++ ) {
        dst[ i ] += B[ i ];
    }
}

void get_lengths(const float_cpx_t *A, float *result, int len, double scale ) {
    for ( int i = 0; i < len; i++ ) {
        result[ i ] = A[ i ].len() * scale;
    }
}



float get_mean(const float *A, int len) {
    float mean = 0.0f;
    for ( int i = 0; i < len; i++ ) {
        mean += A[ i ];
    }
    mean /= len;
    return mean;
}

float_cpx_t *freq_shift(const float_cpx_t *signal, uint32_t samples_count, double SR_hz, double freq_shift_hz) {
    float_cpx_t* out = new float_cpx_t[ samples_count ];
    float t = freq_shift_hz * 2.0f * M_PI / SR_hz;
    for ( uint32_t i = 0; i < samples_count; i++ ) {
        float phi = t * i;
        float_cpx_t css( cos( phi ), sin( phi ) );
        out[ i ] = signal[ i ].mul_cpx_const( css );
    }
    return out;
}
