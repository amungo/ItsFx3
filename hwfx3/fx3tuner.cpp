#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>

#include "fx3tuner.h"

Fx3Tuner::Fx3Tuner(FX3DevIfce* dev) : dev( dev )
{

}

void Fx3Tuner::SetPLL(int pll_idx, int band, int is_enable)
{
    uint8_t bit_enable = is_enable ? 1 : 0;
    uint8_t bit_band   = ( band == BandL1 ) ? 1 : 0;
    uint8_t bit_full   = ( bit_band << 1 ) | ( bit_enable << 0 );

    uint8_t addr = ( pll_idx == 0 ) ? 41 : 45;

    pause();
    dev->send16bitSPI( bit_full, addr );
    pause();
}

void Fx3Tuner::SetFreqDivs(int pll_idx, uint32_t N, uint32_t R)
{
    int addrHi, addrLo;
    if ( pll_idx == 0 ) {
        addrHi = 42;
        addrLo = 43;
    } else {
        addrHi = 46;
        addrLo = 47;
    }

    if ( N <  48 ) { N = 48;  }
    if ( N > 511 ) { N = 511; }
    if ( R <  1 ) { R = 1;  }
    if ( R > 15 ) { R = 15; }
    fprintf( stderr, "Fx3Tuner::SetFreqDivs( [%d], N=%u, R=%u )\n", pll_idx, N, R );

    uint8_t regHi = ( N >> 1 ) & 0xFF;
    uint8_t regLo = ( ( N & 0x01 ) << 7 ) | ( R << 3 ) | ( 1 << 0 );
    pause();
    dev->send16bitSPI( regHi, addrHi );
    pause(5);
    dev->send16bitSPI( regLo, addrLo );
    pause();
}

void Fx3Tuner::SetFreq(int pll_idx, double freq) {
    freq = freq / 10000000.0;
    int N;
    for ( int R = 1; R < 16; R++ ) {
        N = freq / R;
        if ( N >= 48 && N <= 511 ) {
            fprintf( stderr, "Fx3Tuner::SetFreq( %f MHz ) found N=%d, R=%d\n", N, R );
            SetFreqDivs( pll_idx, N, R );
            return;
        }
    }
    fprintf( stderr, "Fx3Tuner::SetFreq( %f MHz ) can not find N and R dividers\n" );
}

void Fx3Tuner::pause(int ms)
{
    std::this_thread::sleep_for( std::chrono::milliseconds(ms) );
}
