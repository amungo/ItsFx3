#ifndef ETALOMETRFILE_H
#define ETALOMETRFILE_H

#include "gcacorr/etalometrbase.h"

class EtalometrFile : public EtalometrBase
{
public:
    EtalometrFile();
    ~EtalometrFile();

    // EtalometrBase interface
    void ResetCalibration();
    void SetNewCalibration(float_cpx_t iqss[]);
    int MakeEtalons();
};

#endif // ETALOMETRFILE_H
