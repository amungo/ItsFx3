#include <cstdio>
#include "convresult.h"

ConvResult::ConvResult(int alpha_cnt, int phi_cnt)
{
    SetDimensions(alpha_cnt, phi_cnt);
}

void ConvResult::SetDimensions(int alpha_cnt, int phi_cnt)
{
    data.resize(alpha_cnt);
    for ( int a = 0; a < alpha_cnt; a++ ) {
        data[a].resize(phi_cnt);
    }
}

void ConvResult::Flush()
{
    max = 1.0f;
    min = 0.0f;
    maxAlpha = data.size() / 2;
    maxPhi = data[0].size() / 2;
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
                maxAlpha = a;
                maxPhi = p;
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


