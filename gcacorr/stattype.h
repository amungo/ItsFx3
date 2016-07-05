#ifndef STATTYPE
#define STATTYPE

#include <cstdio>

struct stat_type {
    float  max_correlation;
    int    time_shift;
    double freq;
    double mean;
    inline float corr_relative_coef() { return (float)( max_correlation / mean ); }
    inline bool check( float corr, int tshift ) {
        if ( max_correlation < corr ) {
            max_correlation = corr;
            time_shift = tshift;
            return true;
        }
        return false;
    }
    inline bool check( const stat_type& another ) {
        if ( check( another.max_correlation, another.time_shift ) ) {
            freq = another.freq;
            mean = another.mean;
            return true;
        }
        return false;
    }
    void print( int prn ) {
        fprintf( stderr,
                 "[%2d][ %6.0f ][ %5d ] corr = %5.0f  k = %4.1f  (mean %4.0f)\n",
                 prn, freq, time_shift, max_correlation, corr_relative_coef(), mean );
    }
    float process_vector( const float* vec, int len ) {
        for ( int i = 0; i < len; i++ ) {
            mean += vec[ i ];
            check( vec[ i ], i );
        }
        mean /= len;
        return max_correlation;
    }

    void flush() {
        max_correlation = 0.0f;
        time_shift = 0;
        freq = 0.0;
        mean = 0.0;
    }

    stat_type( double freq = 0.0 ) : max_correlation( 0.0f ), time_shift( 0 ), freq( freq ), mean( 0.0 ){}
};

#endif // STATTYPE

