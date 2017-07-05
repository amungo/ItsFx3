#include <cstdio>
#include "convresult.h"

ConvResult::ConvResult(int thetta_cnt, int phi_cnt)
{
    SetDimensions(thetta_cnt, phi_cnt);
}

void ConvResult::SetDimensions(int thetta_cnt, int phi_cnt)
{
    data.resize(thetta_cnt);
    for ( int a = 0; a < thetta_cnt; a++ ) {
        data[a].resize(phi_cnt);
    }
}

void ConvResult::Flush()
{
    max = 1.0f;
    min = 0.0f;
    maxThettaIdx = data.size() / 2;
    maxPhiIdx = data[0].size() / 2;
}

void ConvResult::ReCalc()
{
    max = data[0][0];
    min = data[0][0];
    for ( size_t a = 0; a < data.size(); a++ ) {
        std::vector<float>& raw = data[a];
        for ( size_t p = 0; p < raw.size(); p++ ) {
            float& x = raw[ p ];
            if ( x > max ) {
                max = x;
                maxThettaIdx = a;
                maxPhiIdx = p;
            }
            if ( x < min ) {
                min = x;
            }
        }
    }
}

void ConvResult::print_dbg() {
    fprintf( stderr, "\n******************\n" );

    for ( size_t a = 0; a < data.size(); a++ ) {
        std::vector<float>& raw = data[a];
        for ( size_t p = 0; p < raw.size(); p++ ) {
            float& x = raw[ p ];
            fprintf( stderr, "%4.1f ", x );
        }
        fprintf( stderr, "\n" );
    }

    fprintf( stderr, "******************\n" );
}


