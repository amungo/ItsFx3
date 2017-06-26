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
    float default_calib[3] = { (float)M_PI / 2.0f, (float)M_PI, 3.0f * (float)M_PI / 2.0f };
    SetCalib( default_calib );
}

void Etalometr::CalcEtalons(double step_deg, double max_phi_angle_deg)
{
    int alpha_cnt = (int)( (360.0 / step_deg) /*+ 1*/ );
    int phi_cnt   = (int)( (2.0 * max_phi_angle_deg / step_deg) /*+ 1*/ );
    etalons.resize( alpha_cnt );
    for ( int a = 0; a < alpha_cnt; a++ ) {
        etalons[a].resize( phi_cnt );
    }
    result.SetDimensions( alpha_cnt, phi_cnt );

    float step = (float)( M_PI * step_deg / 180.0 );
    float max_phi = (float)( M_PI * max_phi_angle_deg / 180.0 );


    for ( int a = 0; a < alpha_cnt; a++ ) {
        float alpha = (float)( -M_PI + a * step );
        for ( int p = 0; p < phi_cnt; p++ ) {
            float phi = -max_phi + p * step;
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
    }
    float delta_len = len * sinf( alpha ) * cos( phi );
    float phase_diff = 2.0f * (float)M_PI * (delta_len / wave_len);
    phase_diff += calib[ ant - 1 ];

    float_cpx_t unit_pt( cosf(phase_diff), sinf(phase_diff) );

    return unit_pt;
}


ConvResult *Etalometr::CalcConvolution(float phases[])
{
    result.Flush();

    float_cpx_t measures[3];
    for ( int i = 0; i < 3; i++ ) {
        measures[i] = float_cpx_t( cosf(phases[i]), sinf(phases[i]) );
    }

    for ( size_t a = 0; a < etalons.size(); a++ ) {
        std::vector<PhasesDiff_t>& etalons_raw = etalons[a];

        for ( size_t p = 0; p < etalons_raw.size(); p++ ) {

            PhasesDiff_t& et = etalons_raw[p];
            float_cpx_t conv( 0.0f, 0.0f );
            conv.add( cpx_mul( measures[0], et.ant_pt[0] ) );
            conv.add( cpx_mul( measures[1], et.ant_pt[1] ) );
            conv.add( cpx_mul( measures[2], et.ant_pt[2] ) );
            result.data[a][p] = conv.len();

        }
    }

    result.ReCalc();

    return &result;
}

ConvResult *Etalometr::GetResult()
{
    return &result;
}

void Etalometr::debug()
{
    float alpha = M_PI/4.0;
    float phi = M_PI/4.0;
    float measures[3] = {
        GetEtalon(1, alpha, phi).angle(),
        GetEtalon(2, alpha, phi).angle(),
        GetEtalon(3, alpha, phi).angle()
    };
    CalcConvolution( measures );
    result.print_dbg();
}














