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
    fprintf( stderr, "SetCalibIqs()\n" );
    if ( etalon_source == EtalonsFromFile ) {

        int tcenter = etalons.size()/2;
        int pcenter = etalons[0].size()/2;
        PhasesDiff_t et = etalons[tcenter][pcenter];

        for ( int a = 0; a < 4; a++ ) {
            double angle = et.ant_pt[a].angle(iqss[a]);
            calib[ a ] = float_cpx_t( cosf(angle), sinf(angle) );
            fprintf( stderr, "angle[%d]: etalon: %4.0f, measured: %4.0f, for calib: %4.0f\n",
                     a, et.ant_pt[a].angle_deg(), iqss[a].angle_deg(), calib[a].angle_deg() );
        }
    } else {
        for ( int a = 0; a < 4; a++ ) {
            calib[ a ] = iqss[ a ];
        }
    }
}

void Etalometr::SetCalibRadians(float phases[])
{
    float_cpx_t iqss[ 4 ] = {
        float_cpx_t( cosf(phases[0]), sinf(phases[0]) ),
        float_cpx_t( cosf(phases[1]), sinf(phases[1]) ),
        float_cpx_t( cosf(phases[2]), sinf(phases[2]) ),
        float_cpx_t( cosf(phases[3]), sinf(phases[3]) )
    };
    SetCalibIqs( iqss );
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

int Etalometr::LoadEtalonsFromFiles()
{
    std::vector<std::vector<PhasesDiff_t>> newet;
    newet.resize( 180 );
    for ( int a = 0; a < 180; a++ ) {
        newet[a].resize( 181 );
    }

    char fname[ 8192 ];
    for ( int a = 0; a < 4; a++ ) {
        sprintf( fname, "etalons_ant%d.csv", a+1 );
        fprintf( stderr, "LoadEtalonsFromFiles(): fname = '%s'\n", fname );
        FILE* f = fopen (fname, "r" );
        if ( !f ) {
            fprintf( stderr, "Error opening file '%s'\b", fname );
            return -1;
        }

        char c = 27;
        do { c = fgetc(f); } while ( c != 10 && c != 13 );

        for ( int th_idx = 0; th_idx < 180; th_idx++ ) {
            int num = -1;
            fscanf( f, "%d", &num );
            if ( num != th_idx ) {
                fprintf( stderr, "Error thetta index: exptected %d, but had %d\n", th_idx, num );
                return -2;
            }
            for ( int ph_idx = 0; ph_idx < 181; ph_idx++ ) {
                float re = -239.0f;
                float im = -239.0f;
                char im_sign = '.';
                int res = fscanf( f, ",%f %c %fi", &re, &im_sign, &im );
                if ( res != 3 ) {
                    fprintf( stderr, "File format error, can't read 2 numbers and sign. Thetta_idx = %d, phi_idx = %d\n", th_idx, ph_idx );
                }
                if ( im_sign == '-' ) {
                    im = -im;
                } else if ( im_sign == '+' ) {
                    // nop
                } else {
                    fprintf( stderr, "File format error, sign is invalid: '%c'\n", im_sign );
                }
                float_cpx_t e( re, im );
                newet[th_idx][ph_idx].ant_pt[a] = e;
            }
            c = 27;
            do { c = fgetc(f); } while ( c != 10 && c != 13 );

        }
        fclose(f);
    }


    fprintf( stderr, "thetta: %d ... %d  (by %d)\n", startThettaIdx, 180 - startThettaIdx, etalonStep );
    fprintf( stderr, "phi   : %d ... %d  (by %d)\n", startPhiIdx,    180 - startPhiIdx,    etalonStep );

    for ( int th_idx = startThettaIdx, tidx = 0; th_idx < 180 - startThettaIdx; th_idx += etalonStep, tidx++ ) {
        for ( int ph_idx = startPhiIdx, pidx = 0; ph_idx < 180 - startPhiIdx; ph_idx += etalonStep, pidx++ ) {
            PhasesDiff_t x;
            x.ant_pt[0] = newet[th_idx][ph_idx].ant_pt[3];
            x.ant_pt[1] = newet[th_idx][ph_idx].ant_pt[2];
            x.ant_pt[2] = newet[th_idx][ph_idx].ant_pt[1];
            x.ant_pt[3] = newet[th_idx][ph_idx].ant_pt[0];
            etalons[tidx][pidx] = x;
        }
    }

    float null_calib[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    SetCalibDegrees( null_calib );

    etalon_source = EtalonsFromFile;
    return 0;
}


void Etalometr::CalcEtalons(double step_deg, double max_thetta_angle_deg, double max_phi_angle_deg)
{
    fprintf( stderr, "CalcEtalons()\n" );

    this->max_thetta_angle_deg = max_thetta_angle_deg;
    this->max_phi_angle_deg = max_phi_angle_deg;
    startThettaIdx = (int)round(90.0 - max_thetta_angle_deg);
    startPhiIdx    = (int)round(90.0 - max_phi_angle_deg);
    etalonStep     = (int)round( step_deg );

    float thetta_range_rad = (float)( max_thetta_angle_deg * M_PI / 180.0 );
    float phi_range_rad    = (float)( max_phi_angle_deg    * M_PI / 180.0 );
    int thetta_cnt = 2 * (int)(max_thetta_angle_deg / step_deg);
    int phi_cnt    = 2 * (int)(max_phi_angle_deg    / step_deg);
    etalons.resize( thetta_cnt );
    for ( int a = 0; a < thetta_cnt; a++ ) {
        etalons[a].resize( phi_cnt );
    }
    result.SetDimensions( thetta_cnt, phi_cnt );

    float step = (float)( M_PI * step_deg / 180.0 );

    for ( int th_idx = 0; th_idx < thetta_cnt; th_idx++ ) {
        float thetta = -thetta_range_rad + th_idx * step;

        for ( int ph_idx = 0; ph_idx < phi_cnt; ph_idx++ ) {

            float phi = -phi_range_rad + ph_idx * step;
            PhasesDiff_t& et = etalons[th_idx][ph_idx];
            et.ant_pt[0] = GetEtalon(0, thetta, phi);
            et.ant_pt[1] = GetEtalon(1, thetta, phi);
            et.ant_pt[2] = GetEtalon(2, thetta, phi);
            et.ant_pt[3] = GetEtalon(3, thetta, phi);
        }
    }
    etalon_source = EtalonsCalculated;
}

void Etalometr::ReCalculateEtalons()
{
    if ( etalon_source == EtalonsCalculated ) {
        CalcEtalons( etalonStep, max_thetta_angle_deg, max_phi_angle_deg );
    } else {
        for ( size_t th_idx = 0; th_idx < etalons.size(); th_idx++ ) {
            std::vector<PhasesDiff_t>& raw = etalons[th_idx];
            for ( size_t ph_idx = 0; ph_idx < raw.size(); ph_idx++ ) {
                PhasesDiff_t& x = raw[ph_idx];
                x.ant_pt[0].mul_cpx(calib[0]);
                x.ant_pt[1].mul_cpx(calib[1]);
                x.ant_pt[2].mul_cpx(calib[2]);
                x.ant_pt[3].mul_cpx(calib[3]);
            }
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














