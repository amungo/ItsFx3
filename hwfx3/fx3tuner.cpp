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

    fprintf( stderr, "Fx3Tuner::SetPLL( [%d], %s, %s )\n", pll_idx,
             band == BandL1 ? "L1" : "L2/L3/L5",
             is_enable ? "enable" : "DISABLE" );
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

double Fx3Tuner::SetFreq(int pll_idx, double freq) {
    double Ftcxo = 10.0e6;
    double f = freq / Ftcxo;
    int N0 = 0;
    int R0 = 0;
    double prec0 = 1e100;
    int N;
    double prec;

    for ( int R = 15; R > 0; R-- ) {
        N = round( f * R );
        if ( N >= 48 && N <= 511 ) {
            prec = abs(freq - Ftcxo*(double)N/(double)R);
            fprintf( stderr, "Fx3Tuner::SetFreq( %f MHz ) candidate N=%d, R=%d, prec=%f\n", freq/1.0e6, N, R, prec );
            if ( prec < prec0 ) {
                N0 = N;
                R0 = R;
                prec0 = prec;
            }
        }
    }

    if ( N0 != 0 ) {
        fprintf( stderr, "Fx3Tuner::SetFreq( %f MHz ) using N=%d, R=%d, prec=%f\n", freq/1.0e6, N0, R0, prec0 );
        SetFreqDivs( pll_idx, N0, R0 );
        return Ftcxo*(double)N0/(double)R0;
    } else {
        fprintf( stderr, "Fx3Tuner::SetFreq( %f MHz ) can not find N and R dividers\n", freq/1.0e6 );
    }
    return 0.0;
}

void Fx3Tuner::SetTCXO_LO(int tcxo_sel, int lo_src) {
    uint8_t addr = 3;
    uint8_t tcxo_bit = ( tcxo_sel == TCXO_sel_10000kHz ? 0 : 1 );
    uint8_t lo_bit   = ( lo_src == LO_src_A ? 0 : 1 );
    uint8_t reg = ( tcxo_bit << 1 ) | ( lo_bit << 0 );
    dev->send16bitSPI( reg, addr );
}

void Fx3Tuner::ConfigureClockLVDS(int clk_src, int clk_div, int ampl, int outDC) {
    if ( clk_div <  8 ) { clk_div =  8; }
    if ( clk_div > 31 ) { clk_div = 31; }
    uint8_t div_bit  = clk_div;
    uint8_t src_bit  = ( clk_src == CLK_src_PLLA ) ? 0 : 1;
    uint8_t type_bit = 1; // LVDS
    uint8_t cc_bit   = 0x2;
    switch ( ampl ) {
    case CLK_LVDS_ampl_230mV: cc_bit = 0x0; break;
    case CLK_LVDS_ampl_340mV: cc_bit = 0x1; break;
    case CLK_LVDS_ampl_460mV: cc_bit = 0x2; break;
    case CLK_LVDS_ampl_570mV: cc_bit = 0x3; break;
    }
    uint8_t ol_bit = 0x0;
    switch( outDC ) {
    case CLK_LVDS_outDC_1550mV: ol_bit = 0x0; break;
    case CLK_LVDS_outDC_2100mV: ol_bit = 0x1; break;
    case CLK_LVDS_outDC_2400mV: ol_bit = 0x2; break;
    case CLK_LVDS_outDC_2700mV: ol_bit = 0x3; break;
    }
    dev->send16bitSPI( ( div_bit << 0 ), 11 );
    dev->send16bitSPI( (src_bit<<5)|(type_bit<<4)|(cc_bit<<2)|(ol_bit<<0), 12 );
    pause();
}

void Fx3Tuner::ConfigureClockCMOS(int clk_src, int clk_div, int ampl) {
    if ( clk_div <  8 ) { clk_div =  8; }
    if ( clk_div > 31 ) { clk_div = 31; }
    uint8_t div_bit  = clk_div;
    uint8_t src_bit  = ( clk_src == CLK_src_PLLA ) ? 0 : 1;
    uint8_t type_bit = 0; // CMOS
    uint8_t cc_bit   = 0x2;
    uint8_t ol_bit   = 0x0;
    switch( ampl ) {
    case CLK_CMOS_ampl_1800mV: ol_bit = 0x0; break;
    case CLK_CMOS_ampl_2400mV: ol_bit = 0x1; break;
    case CLK_CMOS_ampl_2700mV: ol_bit = 0x2; break;
    case CLK_CMOS_ampl_VCC:    ol_bit = 0x3; break;
    }
    dev->send16bitSPI( ( div_bit << 0 ), 11 );
    dev->send16bitSPI( (src_bit<<5)|(type_bit<<4)|(cc_bit<<2)|(ol_bit<<0), 12 );
    pause();
}

void Fx3Tuner::pause(int ms)
{
    std::this_thread::sleep_for( std::chrono::milliseconds(ms) );
}
