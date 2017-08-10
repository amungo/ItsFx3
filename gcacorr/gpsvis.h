#ifndef GPSVIS_H
#define GPSVIS_H

#include "rawsignal.h"
#include "dsp_utils.h"
#include "stattype.h"
#include <vector>
#include <map>

struct corr_vector_t {
    std::vector< float > data;
    stat_type stat;
};
typedef std::map< double, corr_vector_t > corr_map_t;
typedef std::map< double, corr_vector_t >::iterator corr_map_iter_t;

struct correlation_matrix_t {
    corr_map_t cmap;
    stat_type all_stat;
};


class GPSVis
{
public:
    GPSVis( uint32_t prn,
            const double doppler_freq_border =     10000.0,
            const double doppler_step        =      1000.0,
            const double sample_rate         =  53000000.0,
            const double gps_L1_freq_offset  = -14580000.0,
            bool is_glonass = false
           );

    ~GPSVis();

    void SetSignal( std::vector< RawSignal* >* signals_ptr );
    void CalcCorrMatrix();
    bool FindMaxCorr(double& freq_out, int& time_shift_out, float &corr_val);
    void PreciseFreq(double& freq_out, int& time_shift_out, float &corr_val);
    void GetCorrMatrix( std::vector< std::vector< float > >& out, std::vector<double>& freqs);
    void SetEdgeKoef( double k );

private:
    void FlushCorr();
    void GenerateEtalonCode();
    void CalcCorrVector( double doppler_freq );
    bool FindMaxCorr(double& freq, int& time_shift_out, float &corr_val, double min_freq, double max_freq );

private:
    bool is_glonass = false;
    const int    PRN;
    const double SR;
    const double GPS_FREQ;
    const int    NPNT;
    const double DOPPLER_BORDER;
    const double DOPPLER_STEP;
    const int    DOPPLER_STEP_CNT;
    const double CPS;

private:
    FFTWrapper fft;
    float_cpx_t* tmp_vec_cpx;
    float* tmp_vec_flt;
    float_cpx_t* etcode_fft_conj;
    correlation_matrix_t corr_matrix;

    std::vector< RawSignal* >* sigs;

    double edgeKoef;

};

#endif // GPSVIS_H
