#ifndef FX3TUNER_H
#define FX3TUNER_H

#include "fx3devifce.h"

class Fx3Tuner
{
public:
    Fx3Tuner(FX3DevIfce* dev);

    static const int BandL2 = 0;
    static const int BandL1 = 1;
    void SetPLL( int pll_idx, int band, int is_enable );

    void SetFreqDivs( int pll_idx, uint32_t N, uint32_t R );
    double SetFreq( int pll_idx, double freq );

    static const int TCXO_sel_10000kHz = 0;
    static const int TCXO_sel_24840kHz = 1;
    static const int LO_src_A  = 0;
    static const int LO_src_AB = 1;
    void SetTCXO_LO( int tcxo_sel, int lo_src );


    static const int CLK_src_PLLA = 0;
    static const int CLK_src_PLLB = 1;
    static const int CLK_LVDS_ampl_230mV = 0;
    static const int CLK_LVDS_ampl_340mV = 1;
    static const int CLK_LVDS_ampl_460mV = 2;
    static const int CLK_LVDS_ampl_570mV = 3;
    static const int CLK_LVDS_outDC_1550mV = 0;
    static const int CLK_LVDS_outDC_2100mV = 1;
    static const int CLK_LVDS_outDC_2400mV = 2;
    static const int CLK_LVDS_outDC_2700mV = 3;
    void ConfigureClockLVDS( int clk_src, int clk_div, int ampl, int outDC );

    static const int CLK_CMOS_ampl_1800mV = 0;
    static const int CLK_CMOS_ampl_2400mV = 1;
    static const int CLK_CMOS_ampl_2700mV = 2;
    static const int CLK_CMOS_ampl_VCC    = 3;
    void ConfigureClockCMOS( int clk_src, int clk_div, int ampl );

    void Set_AGC();
    void Set_MGC(int rf_gain_idx , int ifa_coarse_gain_idx, int ifa_fine_gain_idx);
    static void ConvertRF( int idx, double& db );
    static void ConvertIFAFine( int idx, double& db );
    static void ConvertIFACoarse( int idx, double& db );


protected:
    FX3DevIfce* dev = nullptr;

private:
    void pause( int ms = 20 );
};

#endif // FX3TUNER_H
