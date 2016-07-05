#ifndef MATRIXSTATISTIC_H
#define MATRIXSTATISTIC_H

#include "mathTypes.h"
#include <map>
#include <vector>
#include <cstdio>


struct VectorStat {
    float max;
    int   idx;
    float mean;
    double freq;
    VectorStat();
    void calc();
    void print( int prn);

    std::vector< float > data;
};


class MatrixStatistic
{
public:
    MatrixStatistic();

    void flush();
    void applyVector( double freq, float* vec, int N );
    VectorStat* GetMax();
    VectorStat* GetForFreq( double freq );

private:
    std::map< double, VectorStat > mtx;
};

#endif // MATRIXSTATISTIC_H
