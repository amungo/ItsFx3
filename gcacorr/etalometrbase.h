#ifndef ETALOMETRBASE_H
#define ETALOMETRBASE_H

#include "gcacorr/mathTypes.h"
#include "convresult.h"

struct PhasesDiff_t {
    float_cpx_t ant_pt[4];
};

class EtalometrBase {
public:
    EtalometrBase();
    virtual ~EtalometrBase();
    virtual void SetBaseParams(
            float base_len,
            double freq,
            double max_thetta_angle_deg,
            double max_phi_angle_deg,
            int etalon_precision_deg );

    virtual void ResetCalibration();
    virtual void SetNewCalibration( float_cpx_t iqss[4] );
    virtual int MakeEtalons() = 0;
    virtual ConvResult* CalcConvolution( float_cpx_t iqs[ 4 ] );
    ConvResult* GetResult();

protected:

    void SetCalibration( float_cpx_t iqss[4] );

    ConvResult result;
    std::vector<std::vector<PhasesDiff_t>> etalons;
    float_cpx_t calib[4];
    float base_len;
    double freq;
    float wave_len;
    int startThettaIdx = 0;
    int startPhiIdx    = 0;
    int etalonStep     = 1;
    double max_thetta_angle_deg = 60.0;
    double max_phi_angle_deg    = 60.0;

    int thetta_cnt = 0;
    int phi_cnt = 0;
};

#endif // ETALOMETRBASE_H
