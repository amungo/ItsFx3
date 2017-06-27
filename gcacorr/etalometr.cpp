#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include "etalometr.h"

Etalometr::Etalometr( float base_len ) :
    base_len( base_len )
{

}

void Etalometr::SetFreq(double freq)
{
    this->freq = freq;
    this->wave_len = (float)( 3e8 / freq );
}

void Etalometr::SetCalib(float phases[])
{
    for ( int i = 0; i < 3; i++ ) {
        calib[ i ] = phases[ i ];
    }
}

void Etalometr::SetCalibDefault()
{
    float default_calib[3] = {
        (float)M_PI / 2.0f,
        (float)M_PI,
        3.0f * (float)M_PI / 2.0f
    };
    SetCalib( default_calib );
}

void Etalometr::CalcEtalons(double step_deg, double max_phi_angle_deg)
{
    int alpha_cnt = (int)(360.0 / step_deg);
    int phi_cnt   = (int)( max_phi_angle_deg / step_deg);
    etalons.resize( alpha_cnt );
    for ( int a = 0; a < alpha_cnt; a++ ) {
        etalons[a].resize( phi_cnt );
    }
    result.SetDimensions( alpha_cnt, phi_cnt );

    float step = (float)( M_PI * step_deg / 180.0 );

    for ( int a = 0; a < alpha_cnt; a++ ) {
        float alpha = (float)( /*-M_PI +*/ a * step );
        for ( int p = 0; p < phi_cnt; p++ ) {
            float phi = p * step;
            PhasesDiff_t& et = etalons[a][p];
            et.ant_pt[0] = GetEtalon(1, alpha, phi);
            et.ant_pt[1] = GetEtalon(2, alpha, phi);
            et.ant_pt[2] = GetEtalon(3, alpha, phi);
        }
    }
}

float_cpx_t Etalometr::GetEtalon(int ant, float alpha, float phi)
{
    float len = base_len;
    switch ( ant ) {
    case 0:
        return 0.0f;
    case 1:
        alpha += 0.0f;
        len = base_len;
        break;
    case 2:
        alpha += (float)( M_PI / 4.0f );
        len = base_len * sqrtf(2.0f);
        break;
    case 3:
        alpha += (float)( M_PI / 2.0f );
        len = base_len;
        break;
    }
    float delta_len = -len * cos( alpha ) * sin( phi );
    float phase_diff = 2.0f * (float)M_PI * (delta_len / wave_len);
    phase_diff += calib[ ant - 1 ];

    float_cpx_t unit_pt( cosf(phase_diff), sinf(phase_diff) );

    return unit_pt;
}

ConvResult* Etalometr::CalcConvolution(float_cpx_t iqs[] ) {
    result.Flush();

    for ( size_t a = 0; a < etalons.size(); a++ ) {
        std::vector<PhasesDiff_t>& etalons_raw = etalons[a];

        for ( size_t p = 0; p < etalons_raw.size(); p++ ) {

            PhasesDiff_t& et = etalons_raw[p];
            float_cpx_t conv( 0.0f, 0.0f );
            conv.add(  et.ant_pt[0].mul_cpx_conj_const( iqs[0] )  );
            conv.add(  et.ant_pt[1].mul_cpx_conj_const( iqs[1] )  );
            conv.add(  et.ant_pt[2].mul_cpx_conj_const( iqs[2] )  );
            conv.mul_real(1.0f/3.0f);
            result.data[a][p] = conv.len();

        }
    }

    result.ReCalc();

    return &result;
}

ConvResult* Etalometr::CalcConvolution(float phases[])
{
    float_cpx_t iqs[3];
    for ( int i = 0; i < 3; i++ ) {
        float_cpx_t X( cosf(phases[i]), sinf(phases[i]) );
        iqs[i] = X;
    }
    return CalcConvolution( iqs );
}

ConvResult *Etalometr::GetResult()
{
    return &result;
}

void Etalometr::debug()
{
    float alpha = (float)( M_PI *  0.0 / 180.0 );
    float phi   = (float)( M_PI * 45.0 / 180.0 );
    float_cpx_t iqs[3] = {
        GetEtalon(1, alpha, phi),
        GetEtalon(2, alpha, phi),
        GetEtalon(3, alpha, phi)
    };
    CalcConvolution( iqs );
    //result.print_dbg();
}














