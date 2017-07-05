#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include "gcacorr/point3d_t.h"
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

void Etalometr::SetCalibIqs(float_cpx_t iqss[])
{
    for ( int i = 0; i < 4; i++ ) {
        calib[ i ] = iqss[ i ];
    }
}

void Etalometr::SetCalibRadians(float phases[])
{
    for ( int i = 0; i < 4; i++ ) {
        float_cpx_t pt( cosf(phases[ i ]), sinf(phases[ i ]) );
        calib[ i ] = pt;
    }
}

void Etalometr::SetCalibDegrees(float phases[])
{
    float calib[4] = {
        (float)( M_PI * phases[0] / 180.0 ),
        (float)( M_PI * phases[1] / 180.0 ),
        (float)( M_PI * phases[2] / 180.0 ),
        (float)( M_PI * phases[3] / 180.0 )
    };
    SetCalibRadians( calib );

}

void Etalometr::SetCalibDefault()
{
    float default_calib[4] = {
           0.0f,
         -55.0f,
        -145.0f,
          90.0f
    };
    SetCalibDegrees( default_calib );
}

void Etalometr::CalcEtalons(double step_deg, double max_thetta_angle_deg, double max_phi_angle_deg)
{
    int thetta_cnt = 2 * (int)(max_thetta_angle_deg / step_deg);
    int phi_cnt    = 2 * (int)(max_phi_angle_deg    / step_deg);
    etalons.resize( thetta_cnt );
    for ( int a = 0; a < thetta_cnt; a++ ) {
        etalons[a].resize( phi_cnt );
    }
    result.SetDimensions( thetta_cnt, phi_cnt );

    float step = (float)( M_PI * step_deg / 180.0 );

    for ( int th_idx = 0; th_idx < thetta_cnt; th_idx++ ) {
        float thetta = (float)( (float)-M_PI_2 + th_idx * step );

        for ( int ph_idx = 0; ph_idx < phi_cnt; ph_idx++ ) {

            float phi = (float)( (float)-M_PI_2 + ph_idx * step );
            PhasesDiff_t& et = etalons[th_idx][ph_idx];
            et.ant_pt[0] = GetEtalon(0, thetta, phi);
            et.ant_pt[1] = GetEtalon(1, thetta, phi);
            et.ant_pt[2] = GetEtalon(2, thetta, phi);
            et.ant_pt[3] = GetEtalon(3, thetta, phi);
        }
    }
}

float_cpx_t Etalometr::GetEtalon(int ant, float thetta, float phi)
{
    point3d_t E( 0.0f, 0.0f, 0.0f );

    switch ( ant ) {
    case 0:
        E.x = -base_len / 2.0f;
        E.y = -base_len / 2.0f;
        break;
    case 1:
        E.x = -base_len / 2.0f;
        E.y =  base_len / 2.0f;
        break;
    case 2:
        E.x =  base_len / 2.0f;
        E.y =  base_len / 2.0f;
        break;
    case 3:
        E.x =  base_len / 2.0f;
        E.y = -base_len / 2.0f;
        break;
    }

    E.rotateOX( -phi );
    E.rotateOY( -thetta );

    float delta_len = E.z;
    float phase_diff = 2.0f * (float)M_PI * (delta_len / wave_len);

    float_cpx_t pt( cosf(phase_diff), sinf(phase_diff) );
    pt.mul_cpx( calib[ ant ] );

    return pt;
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
            conv.add(  et.ant_pt[3].mul_cpx_conj_const( iqs[3] )  );
            conv.mul_real(1.0f/4.0f);
            result.data[a][p] = conv.len();

        }
    }

    result.ReCalc();

    return &result;
}

ConvResult* Etalometr::CalcConvolution(float phases[])
{
    float_cpx_t iqs[4];
    for ( int i = 0; i < 4; i++ ) {
        float phase_rad = (float)( M_PI * phases[i] / 180.0 );
        float_cpx_t X( cosf(phase_rad), sinf(phase_rad) );
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
    float thetta = (float)( M_PI *   30.0 / 180.0 );
    float phi    = (float)( M_PI *    5.0 / 180.0 );
    float_cpx_t iqs[4] = {
        GetEtalon(0, thetta, phi),
        GetEtalon(1, thetta, phi),
        GetEtalon(2, thetta, phi),
        GetEtalon(3, thetta, phi)
    };
    CalcConvolution( iqs );

//    float phases[3] = {
//           0.0f,
//          90.0f,
//        -145.0f,
//         -55.0f
//    };
//    CalcConvolution( phases );
    //result.print_dbg();
}














