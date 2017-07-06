#include "etalometrbase.h"


EtalometrBase::EtalometrBase()
{
    etalons.resize(0);
    ResetCalibration();
}

EtalometrBase::~EtalometrBase()
{

}

void EtalometrBase::SetBaseParams(
        float base_len,
        double freq,
        double max_thetta_angle_deg,
        double max_phi_angle_deg,
        int etalon_precision_deg)
{
    this->base_len = base_len;
    this->freq = freq;
    this->wave_len = (float)( 3e8 / freq );
    this->max_thetta_angle_deg = max_thetta_angle_deg;
    this->max_phi_angle_deg = max_phi_angle_deg;
    this->startThettaIdx = (int)round(90.0 - max_thetta_angle_deg);
    this->startPhiIdx    = (int)round(90.0 - max_phi_angle_deg);
    this->etalonStep = etalon_precision_deg;

    this->thetta_cnt = 2 * (int)(max_thetta_angle_deg / etalonStep);
    this->phi_cnt    = 2 * (int)(max_phi_angle_deg    / etalonStep);

    etalons.resize( thetta_cnt );
    for ( int a = 0; a < thetta_cnt; a++ ) {
        etalons[a].resize( phi_cnt );
    }
    result.SetDimensions( thetta_cnt, phi_cnt );
    result.rangePhi = max_phi_angle_deg;
    result.rangeThetta = max_thetta_angle_deg;

}

void EtalometrBase::ResetCalibration()
{
    float_cpx_t defcalib[4] = {
        float_cpx_from_angle_deg(0.0f),
        float_cpx_from_angle_deg(0.0f),
        float_cpx_from_angle_deg(0.0f),
        float_cpx_from_angle_deg(0.0f)
    };
    SetNewCalibration( defcalib );
}

void EtalometrBase::SetNewCalibration(float_cpx_t iqss[])
{
    SetCalibration( iqss );
}

ConvResult *EtalometrBase::CalcConvolution(float_cpx_t iqs[])
{
    result.Flush();

    for ( size_t a = 0; a < etalons.size(); a++ ) {
        std::vector<PhasesDiff_t>& etalons_raw = etalons[a];

        for ( size_t p = 0; p < etalons_raw.size(); p++ ) {

            PhasesDiff_t& et = etalons_raw[p];
            float_cpx_t conv( 0.0f, 0.0f );
            conv.add(  et.ant_pt[0].mul_cpx_const(calib[0]).mul_cpx_conj_const( iqs[0] )  );
            conv.add(  et.ant_pt[1].mul_cpx_const(calib[1]).mul_cpx_conj_const( iqs[1] )  );
            conv.add(  et.ant_pt[2].mul_cpx_const(calib[2]).mul_cpx_conj_const( iqs[2] )  );
            conv.add(  et.ant_pt[3].mul_cpx_const(calib[3]).mul_cpx_conj_const( iqs[3] )  );
            conv.mul_real(1.0f/4.0f);
            result.data[a][p] = conv.len();

        }
    }

    result.ReCalc();

    return &result;
}

ConvResult *EtalometrBase::GetResult()
{
    return &result;
}

void EtalometrBase::SetCalibration(float_cpx_t iqss[])
{
    for ( int i = 0; i < 4; i++ ) {
        calib[i] = iqss[i];
    }
}
