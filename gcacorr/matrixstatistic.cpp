#include "matrixstatistic.h"


VectorStat::VectorStat() : max( 0.0 ), idx( 0 ), mean( 0 ) {
}

void VectorStat::calc() {
    mean = 0.0f;
    max  = 0.0;
    for ( unsigned int i = 0; i < data.size(); i++ ) {
        mean += data[ i ];
        if ( max < data[ i ] ) {
            max = data[ i ];
            idx = i;
        }
    }
    mean /= data.size();
}

void VectorStat::print(int prn) {
    fprintf( stderr, "[%2d][ %6.0f ][ %5d ] max: %12.1f mean: %6.1f coef: %3.1f\n",
             prn, freq, idx, max, mean, max / mean );
}

MatrixStatistic::MatrixStatistic() {
    flush();
}

void MatrixStatistic::flush() {
    mtx.clear();
}

void MatrixStatistic::applyVector(double freq, float *vec, int N) {
    VectorStat& st = mtx[ freq ];
    if ( st.data.size() != ( unsigned int ) N ) {
        st.data.resize( N, 0.0 );
    }

    for ( int i = 0; i < N; i++ ) {
        st.data[ i ] += vec[ i ];
    }
    st.calc();
}

VectorStat *MatrixStatistic::GetMax() {
    std::map< double, VectorStat >::iterator it = mtx.begin();
    VectorStat* maxst = NULL;
    while ( it != mtx.end() ) {
        if ( maxst == NULL ) {
            maxst = &( it->second );
            maxst->freq = it->first;
        } else if ( maxst->max < it->second.max ) {
            maxst = &( it->second );
            maxst->freq = it->first;
        }
        ++it;
    }
    return maxst;
}

VectorStat *MatrixStatistic::GetForFreq(double freq) {
    if ( mtx.find( freq ) == mtx.end() ) {
        return NULL;
    } else {
        return &mtx[ freq ];
    }
}
