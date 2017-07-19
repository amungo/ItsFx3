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

    uint8_t regHi = ( N >> 1 ) & 0xFF;
    uint8_t regLo = ( ( N & 0x01 ) << 7 ) | ( R << 3 ) | ( 1 << 0 );
    pause();
    dev->send16bitSPI( regHi, addrHi );
    dev->send16bitSPI( regLo, addrLo );
    pause();
}

void Fx3Tuner::pause()
{
    std::this_thread::sleep_for( std::chrono::milliseconds(20) );
}
