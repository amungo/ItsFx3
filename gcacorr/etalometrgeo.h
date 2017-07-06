#ifndef ETALOMETR_H
#define ETALOMETR_H

#include <vector>
#include "gcacorr/etalometrbase.h"

class EtalometrGeo : public EtalometrBase
{
public:

    EtalometrGeo();
    ~EtalometrGeo();

    // EtalometrBase interface
    void ResetCalibration();
    int MakeEtalons();


private:

    float_cpx_t GetEtalon( int ant, float thetta, float phi );

};

#endif // ETALOMETR_H
