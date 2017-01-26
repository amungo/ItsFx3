#include <cstdio>

#include "lazy_matrix.h"

using namespace std;

matrix_t create_matrix(int N) {
    vector< vector< float_cpx_t > > ans;
    ans.resize( N );
    for( size_t i = 0; i < ans.size(); i++ ) {
        ans[ i ].resize( N );
    }

    for ( int i = 0; i < N; i++ ) {
        for ( int k = 0; k < N; k++ ) {
            ans[ i ][ k ] = float_cpx_t( 0.0f, 0.0f );
        }
    }
    return ans;
}

matrix_t mul_matrix(const matrix_t &M, float_cpx_t x) {
    int N = M.size();
    matrix_t ans = create_matrix( N );

    for ( int i = 0; i < N; i++ ) {
        for ( int k = 0; k < N; k++ ) {
            ans[i][k] = M[i][k] * x;
        }
    }
    return ans;
}

matrix_t transpose_matrix(const matrix_t &M) {
    int N = M.size();
    matrix_t ans = create_matrix( N );

    for ( int i = 0; i < N; i++ ) {
        for ( int k = 0; k < N; k++ ) {
            ans[i][k] = M[k][i];
        }
    }
    return ans;
}

float_cpx_t determinant(const matrix_t &M) {
    int N = M.size();
    if ( N == 2 ) {
        return M[0][0]*M[1][1] - M[0][1]*M[1][0];
    } else {
        float_cpx_t S( 0.0f, 0.0f );
        for ( int i = 0; i < N; i++ ) {
            S = S + msign(i) * M[0][i]*determinant( cut_matrix( M, 0, i) );
        }
        return S;
    }
}

matrix_t inverse_matrix(const matrix_t &M) {
    float_cpx_t det = determinant(M);
    if ( abs(det.q) > 0.0001f ) {
        fprintf( stderr, "Determinant isn't real\n" );
    }

    int N = M.size();
    matrix_t ans = create_matrix( N );

    for ( int i = 0; i < N; i++ ) {
        for ( int k = 0; k < N; k++ ) {
            ans[i][k] = msign(i,k) * determinant( cut_matrix( M, i, k ) );
        }
    }
    return mul_matrix( transpose_matrix( ans ), 1.0f/det.i );
    //return transpose_matrix( ans );
}


matrix_t cut_matrix(const matrix_t &M, int p, int q) {
    int oldN = M.size();
    int newN = oldN - 1;
    matrix_t ans = create_matrix( newN );
    int ii, kk;
    for ( int i = 0; i < newN; i++ ) {
        for ( int k = 0; k < newN; k++ ) {
            if ( i < p ) {
                ii = i;
            } else {
                ii = i + 1;
            }

            if ( k < q ) {
                kk = k;
            } else {
                kk = k + 1;
            }

            ans[i][k] = M[ii][kk];
        }
    }
    return ans;
}

void print_matrix(const matrix_t &M) {
    int N = M.size();

    for ( int i = 0; i < N; i++ ) {
        for ( int k = 0; k < N; k++ ) {
            printf( "%6.2f +%6.2fi ", M[i][k].i, M[i][k].q );
        }
        printf( "\n" );
    }
}

