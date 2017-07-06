#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include "gcacorr/point3d_t.h"
#include "etalometrgeo.h"


EtalometrGeo::EtalometrGeo()
{
    ResetCalibration();
}

EtalometrGeo::~EtalometrGeo()
{

}

void EtalometrGeo::ResetCalibration()
{
    float_cpx_t defcalib[4] = {
        float_cpx_from_angle_deg(   0.0f),
        float_cpx_from_angle_deg( -55.0f),
        float_cpx_from_angle_deg(-145.0f),
        float_cpx_from_angle_deg(  90.0f)
    };
    SetNewCalibration( defcalib );
}

int EtalometrGeo::MakeEtalons()
{
    fprintf( stderr, "EtalometrGeo::MakeEtalons()\n" );

    float thetta_range_rad = deg2rad( max_thetta_angle_deg );
    float phi_range_rad    = deg2rad( max_phi_angle_deg );
    float step_rad = deg2rad(etalonStep);

    for ( int th_idx = 0; th_idx < thetta_cnt; th_idx++ ) {
        float thetta = -thetta_range_rad + th_idx * step_rad;

        for ( int ph_idx = 0; ph_idx < phi_cnt; ph_idx++ ) {

            float phi = -phi_range_rad + ph_idx * step_rad;
            PhasesDiff_t& et = etalons[th_idx][ph_idx];
            et.ant_pt[0] = GetEtalon(0, thetta, phi);
            et.ant_pt[1] = GetEtalon(1, thetta, phi);
            et.ant_pt[2] = GetEtalon(2, thetta, phi);
            et.ant_pt[3] = GetEtalon(3, thetta, phi);
        }
    }
    return 0;
}

float_cpx_t EtalometrGeo::GetEtalon(int ant, float thetta, float phi)
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

    return float_cpx_from_angle_rad(phase_diff);
}















