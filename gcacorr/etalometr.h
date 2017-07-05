#ifndef ETALOMETR_H
#define ETALOMETR_H

#include <vector>
#include "gcacorr/mathTypes.h"
#include "convresult.h"

struct PhasesDiff_t {
    float_cpx_t ant_pt[4];
};

class Etalometr
{
public:

    Etalometr(float base_len );
    void SetFreq( double freq );
    void SetCalibIqs(  float_cpx_t iqss[ 4 ] );
    void SetCalibRadians(  float phases[ 4 ] );
    void SetCalibDegrees(  float phases[ 4 ] );
    void SetCalibDefault();
    void CalcEtalons( double step_deg, double max_thetta_angle_deg, double max_phi_angle_deg );
    ConvResult* CalcConvolution( float_cpx_t iqs[ 4 ] );
    ConvResult* CalcConvolution( float phases[ 4 ] );
    ConvResult* GetResult();

    void debug();

private:

    float_cpx_t GetEtalon( int ant, float thetta, float phi );

    std::vector<std::vector<PhasesDiff_t>> etalons;
    float_cpx_t calib[4];
    float base_len;
    double freq;
    float wave_len;

    ConvResult result;
};

#endif // ETALOMETR_H
