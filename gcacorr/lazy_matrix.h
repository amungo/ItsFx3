#ifndef LAZY_MATRIX_H
#define LAZY_MATRIX_H


#include <vector>
#include "mathTypes.h"


typedef std::vector< std::vector< float_cpx_t > > matrix_t;

matrix_t create_matrix( int N );
matrix_t mul_matrix( const matrix_t& M, float_cpx_t x );
matrix_t transpose_matrix( const matrix_t& M );
float_cpx_t determinant( const matrix_t& M );
matrix_t cut_matrix( const matrix_t& M, int p, int q );
matrix_t inverse_matrix( const matrix_t& M );

void print_matrix( const matrix_t& M );

inline float msign( int k ) {
    if ( k % 2 == 0 ) return 1.0f;
    else return -1.0f;
}

inline float msign( int i, int k ) {
    return msign( i + k );
}

#endif // LAZY_MATRIX_H
