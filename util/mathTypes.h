#ifndef _math_types_h_
#define _math_types_h_

#include <math.h>

typedef struct float_cpx_t {
    float i;
    float q;
    
    inline float_cpx_t( float _i = 0.0f, float _q = 0.0f ) : i(_i), q(_q){}
    
    inline void add( float_cpx_t& A ) {
        this->i += A.i;
        this->q += A.q;
    }
    
    inline float_cpx_t& mul( float x ) {
        this->i *= x;
        this->q *= x;
        return *this;
    }
    
    inline float_cpx_t mul_const( float x ) const {
        return float_cpx_t( this->i * x, this->q * x );
    }

    inline float len_squared() const {
        return i*i + q*q;
    }
    
    inline float len() const {
        return sqrt( len_squared() );
    }
    
} float_cpx_t;

static inline float_cpx_t cpx_mul( const float_cpx_t& A, const float_cpx_t& B ) {
    float_cpx_t S;
    S.i = A.i * B.i - A.q * B.q;
    S.q = A.i * B.q + A.q * B.i;
    return S;
}

#endif

