#ifndef RAWSIGNAL_H
#define RAWSIGNAL_H


#include <cstdint>
#include <map>
#include "mathTypes.h"
#include "dsp_utils.h"
#include "fftwrapper.h"

enum DataType {
    DT_INT8_REAL  = 0,
    DT_INT16_REAL = 1,
    DT_FLOAT_IQ   = 2
};

class RawSignal {
public:
    RawSignal( int pts_count, double sample_rate );
    ~RawSignal();

public:
    void LoadDataFromFile(const char* fileName, DataType dtype, size_t offset_pts);
    void LoadData(void* data, DataType dtype , uint32_t offset);
    const float_cpx_t* GetSignalShifted( double freq );

private:
    void MakeSignalFFT();
    void ClearShiftedCache();
private:
    float_cpx_t* signal_source;
    float_cpx_t* signal_fft;
    int N;
    double FILTER_WIDTH;
    double SR;

    std::map< double, float_cpx_t* > signal_shifted;

    FFTWrapper fft;
};

#endif // RAWSIGNAL_H
