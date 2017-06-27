#ifndef _math_types_h_
#define _math_types_h_

#define _USE_MATH_DEFINES
#include <math.h>

typedef struct float_cpx_t {
    float i;
    float q;
    
    inline float_cpx_t( float _i = 0.0f, float _q = 0.0f ) : i(_i), q(_q){}
    
    inline void add( const float_cpx_t& A ) {
        this->i += A.i;
        this->q += A.q;
    }
    
    inline float_cpx_t add_const( const float_cpx_t& A ) const {
        return float_cpx_t( i + A.i, q + A.q );
    }

    inline float_cpx_t& mul_real( const float x ) {
        this->i *= x;
        this->q *= x;
        return *this;
    }

    inline float_cpx_t operator*( const float x ) {
        this->i *= x;
        this->q *= x;
        return *this;
    }

    inline float_cpx_t operator+( const float_cpx_t& A ) {
        this->i += A.i;
        this->q += A.q;
        return *this;
    }

    inline float_cpx_t& mul_cpx( const float_cpx_t& A ) {
        float_cpx_t X = float_cpx_t( i*A.i - q*A.q, i*A.q + q*A.i );
        this->i = X.i;
        this->q = X.q;
        return *this;
    }


    inline float_cpx_t mul_real_const( const float x ) const {
        return float_cpx_t( this->i * x, this->q * x );
    }
    
    inline float_cpx_t mul_cpx_const( const float_cpx_t& A ) const {
        return float_cpx_t( i*A.i - q*A.q, i*A.q + q*A.i );
    }

    inline float_cpx_t mul_cpx_conj_const( const float_cpx_t& A ) const {
        return float_cpx_t( i*A.i - q*-A.q, i*-A.q + q*A.i );
    }

    inline float len_squared() const {
        return i*i + q*q;
    }
    
    inline float len() const {
        return sqrt( len_squared() );
    }

    inline double angle( const float_cpx_t& A ) const {
        double dot = i*A.i + q*A.q;     // dot product
        double det = i*A.q - q*A.i;     // determinant
        double angle = atan2(det, dot); // atan2(y, x) or atan2(sin, cos)
        return angle;
    }
    inline double angle_deg( const float_cpx_t& A ) const {
         return angle( A ) * 180.0 / M_PI;
    }

    inline float angle() const {
        return atan2(q,i);
    }

    inline float angle_deg() const {
        return angle() * 180.0f / (float)M_PI;;
    }

    inline float_cpx_t conj() const {
        return float_cpx_t( i, -q );
    }


} float_cpx_t;

static inline float_cpx_t cpx_mul( const float_cpx_t& A, const float_cpx_t& B ) {
    float_cpx_t S;
    S.i = A.i * B.i - A.q * B.q;
    S.q = A.i * B.q + A.q * B.i;
    return S;
}

static inline float_cpx_t operator* ( const float_cpx_t& A, const float_cpx_t& B ) {
    return cpx_mul( A, B );
}

static inline float_cpx_t operator- ( const float_cpx_t& A, const float_cpx_t& B ) {
    return A.add_const( B.mul_real_const( -1.0f ) );
}

static inline float_cpx_t operator+ ( const float_cpx_t& A, const float_cpx_t& B ) {
    return A.add_const( B );
}


#endif

