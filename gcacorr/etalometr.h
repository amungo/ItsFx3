#ifndef ETALOMETR_H
#define ETALOMETR_H

#include <vector>
#include "gcacorr/mathTypes.h"
#include "convresult.h"

struct PhasesDiff_t {
    float_cpx_t ant_pt[3];
};

class Etalometr
{
public:

    Etalometr(float base_len );
    void SetFreq( double freq );
    void SetCalib(  float phases[ 3 ] );
    void SetCalibDefault();
    void CalcEtalons( double step_deg, double max_phi_angle_deg );
    ConvResult* CalcConvolution( float phases[ 3 ] );
    ConvResult* GetResult();

    void debug();

private:

    float_cpx_t GetEtalon( int ant, float alpha, float phi );

    std::vector<std::vector<PhasesDiff_t>> etalons;
    float calib[3];
    float base_len;
    double freq;
    float wave_len;

    ConvResult result;
};

#endif // ETALOMETR_H
