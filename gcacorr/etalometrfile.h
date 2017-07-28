#ifndef ETALOMETRFILE_H
#define ETALOMETRFILE_H

#include "util/etalonsfiles.h"

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
private:
    EtalonsFiles et_files;
};

#endif // ETALOMETRFILE_H
