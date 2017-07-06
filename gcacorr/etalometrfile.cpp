#include "etalometrfile.h"

EtalometrFile::EtalometrFile()
{
}

EtalometrFile::~EtalometrFile()
{

}

void EtalometrFile::ResetCalibration()
{
    float_cpx_t defcalib[4] = {
        float_cpx_from_angle_deg(0.0f),
        float_cpx_from_angle_deg(0.0f),
        float_cpx_from_angle_deg(0.0f),
        float_cpx_from_angle_deg(0.0f)
    };
    SetCalibration( defcalib );
}

void EtalometrFile::SetNewCalibration(float_cpx_t iqss[])
{
    int tcenter = etalons.size()/2;
    int pcenter = etalons[0].size()/2;
    PhasesDiff_t et = etalons[tcenter][pcenter];

    for ( int a = 0; a < 4; a++ ) {
        double angle = et.ant_pt[a].angle(iqss[a]);
        calib[ a ] = float_cpx_from_angle_rad(angle);
        fprintf( stderr, "angle[%d]: etalon: %4.0f, measured: %4.0f, for calib: %4.0f\n",
                 a, et.ant_pt[a].angle_deg(), iqss[a].angle_deg(), calib[a].angle_deg() );
    }
}


int EtalometrFile::MakeEtalons()
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

    int thetta_cnt = 2 * (int)(max_thetta_angle_deg / etalonStep);
    int phi_cnt    = 2 * (int)(max_phi_angle_deg    / etalonStep);

    etalons.resize( thetta_cnt );
    for ( int a = 0; a < thetta_cnt; a++ ) {
        etalons[a].resize( phi_cnt );
    }
    result.SetDimensions( thetta_cnt, phi_cnt );

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
    return 0;

}
