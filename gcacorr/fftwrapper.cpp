#include "fftwrapper.h"

#include <string.h>


FFTWrapper::FFTWrapper(unsigned int N) :
    t( "fft" )
{
    t.SetPrintPeriod( 64 );
    this->N = N;
    in_float    = ( float* )         fftwf_malloc( N * sizeof( float ) );
    in_complex  = ( fftwf_complex* ) fftwf_malloc( N * sizeof( fftwf_complex ) );
    out_complex = ( fftwf_complex* ) fftwf_malloc( N * sizeof( fftwf_complex ) );

    plan_real         = fftwf_plan_dft_r2c_1d( N, in_float, out_complex, FFTW_ESTIMATE );
    plan_complex[ 0 ] = fftwf_plan_dft_1d( N, in_complex, out_complex, FFTW_FORWARD, FFTW_ESTIMATE );
    plan_complex[ 1 ] = fftwf_plan_dft_1d( N, in_complex, out_complex, FFTW_BACKWARD, FFTW_ESTIMATE );
}

FFTWrapper::~FFTWrapper() {
    fftwf_destroy_plan( plan_real );
    fftwf_destroy_plan( plan_complex[ 0 ] );
    fftwf_destroy_plan( plan_complex[ 1 ] );
    fftwf_free( in_float    );
    fftwf_free( in_complex  );
    fftwf_free( out_complex );
}

void FFTWrapper::Transform( const float *in, float_cpx_t *out ) {
    memcpy( in_float, in, N * sizeof( float ) );
    fftwf_execute( plan_real );
    convert( out, out_complex, N );
}

void FFTWrapper::TransformShort( const short *in, float_cpx_t *out ) {
    for ( int i = 0; i < N; i++ ) {
        in_float[ i ] = (float) in[ i ];
    }
    fftwf_execute( plan_real );
    convert_inv( out, out_complex, N );
}


void FFTWrapper::Transform( const float_cpx_t *in, float_cpx_t *out, bool is_inverse ) {
    t.Start();
    memcpy( in_complex, in, N * sizeof( float_cpx_t ) );
    fftwf_execute( plan_complex[ is_inverse ] );
    convert( out, out_complex, N );
    t.Finish();
}

void FFTWrapper::convert(float_cpx_t *dst, fftwf_complex *src, int len) {
    //memcpy( dst, src, len * sizeof( fftwf_complex ) );
    for ( int i = 0; i < len; i++ ) {
        dst[ i ].i = src[ i ][ 0 ];
        dst[ i ].q = src[ i ][ 1 ];
    }
}

void FFTWrapper::convert_inv(float_cpx_t *dst, fftwf_complex *src, int len) {
    for ( int i = 0; i < len/2; i++ ) {
        dst[ i ].i = src[ len/2 - i ][ 0 ];
        dst[ i ].q = src[ len/2 - i ][ 1 ];
    }
    for ( int i = len/2; i < len; i++ ) {
        dst[ i ].i = src[ len - i ][ 0 ];
        dst[ i ].q = src[ len - i ][ 1 ];
    }
}
