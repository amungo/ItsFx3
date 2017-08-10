#include "gpsvis.h"
#include "cas_codes.h"

GPSVis::GPSVis(uint32_t prn, const double doppler_freq_border, const double doppler_step, const double sample_rate, const double gps_L1_freq_offset, bool is_glonass) :
    is_glonass( is_glonass ),
    PRN( prn ),
    SR( sample_rate ),
    GPS_FREQ( gps_L1_freq_offset ),
    NPNT( ( uint32_t ) ( sample_rate / 1000.0f ) ),
    DOPPLER_BORDER( doppler_freq_border ),
    DOPPLER_STEP( doppler_step ),
    DOPPLER_STEP_CNT( 1 + ( doppler_freq_border * 2 ) / doppler_step ),
    CPS( is_glonass ? 511000.0f : 1023000.0f ),
    fft( NPNT ),
    tmp_vec_cpx( NULL ),
    tmp_vec_flt( NULL ),
    etcode_fft_conj( NULL ),
    sigs( NULL )
{
    tmp_vec_cpx = new float_cpx_t[ NPNT ];
    tmp_vec_flt = new float[ NPNT ];
    etcode_fft_conj = new float_cpx_t[ NPNT ];
    GenerateEtalonCode();
}

GPSVis::~GPSVis() {
    if ( tmp_vec_cpx ) {
        delete [] tmp_vec_cpx;
    }
    if ( tmp_vec_flt ) {
        delete [] tmp_vec_flt;
    }
    if ( etcode_fft_conj ) {
        delete [] etcode_fft_conj;
    }
}

void GPSVis::SetSignal(std::vector<RawSignal *> *signals_ptr) {
    FlushCorr();
    sigs = signals_ptr;
}

void GPSVis::CalcCorrMatrix() {
    for ( int fi = 0; fi < DOPPLER_STEP_CNT; fi++ ) {
        double freq = fi * DOPPLER_STEP - DOPPLER_BORDER;
        CalcCorrVector( freq );
    }
}

void GPSVis::FlushCorr() {
    corr_matrix.cmap.clear();
    corr_matrix.all_stat.flush();
}

void GPSVis::GenerateEtalonCode() {
    const int* code = nullptr;
    if ( is_glonass ) {
        code = &( gln_ca[ 0 ] );
        //code = &( gln_ca_inv[ 0 ] );
    } else {
        code = &( ca_codes[ PRN - 1 ][ 0 ] );
    }
    float samples_per_char = SR / CPS;

    for ( int char_idx = 0; char_idx < NPNT; char_idx++ ) {
        int code_idx = ( int ) floor( char_idx / samples_per_char );
        tmp_vec_cpx[ char_idx ].i = 1.0f * ( float ) -code[ code_idx ];
        tmp_vec_cpx[ char_idx ].q = 0.0f;
    }
    fft.Transform( tmp_vec_cpx, etcode_fft_conj, false );
    conjugate( etcode_fft_conj, NPNT );
}

void GPSVis::CalcCorrVector(double doppler_freq) {
    if ( sigs == NULL ) {
        return;
    }

    bool newfreq = false;
    if ( corr_matrix.cmap.find( doppler_freq ) == corr_matrix.cmap.end() ) {
        newfreq = true;
    } else {
        return;
    }

    corr_vector_t& corrvec = corr_matrix.cmap[ doppler_freq ];
    if ( newfreq ) {
        corrvec.data.resize( NPNT, 0.0f );
        corrvec.stat.freq = doppler_freq;
    }

    for ( uint32_t i = 0; i < sigs->size(); i++ ) {
        const float_cpx_t* sig = sigs->at( i )->GetSignalShifted( doppler_freq + GPS_FREQ );

        mul_vectors( sig, etcode_fft_conj, tmp_vec_cpx, NPNT );
        fft.Transform( tmp_vec_cpx, tmp_vec_cpx, true );
        get_lengths( tmp_vec_cpx, tmp_vec_flt, NPNT, 1.0 / NPNT );
        add_vector( &( corrvec.data[ 0 ] ), tmp_vec_flt, NPNT );
    }
    corrvec.stat.process_vector( &( corrvec.data[ 0 ] ), NPNT );
}

bool GPSVis::FindMaxCorr(double &freq_out, int &time_shift_out, float& corr_val ) {
    return FindMaxCorr( freq_out, time_shift_out, corr_val, -DOPPLER_BORDER, +DOPPLER_BORDER );
}

bool GPSVis::FindMaxCorr(double &freq_out, int &time_shift_out, float& corr_val, double min_freq, double max_freq ) {
    corr_map_iter_t it = corr_matrix.cmap.begin();
    while ( it != corr_matrix.cmap.end() ) {
        if ( min_freq <= it->first && it->first <= max_freq ) {
            stat_type& stat_one = it->second.stat;
            //stat_one.print( PRN );
            corr_matrix.all_stat.check( stat_one );
        }
        ++it;
    }
    bool found = corr_matrix.all_stat.corr_relative_coef() > this->edgeKoef;
    if ( found ) {
        fprintf( stderr, "**" );
    } else {
        fprintf( stderr, " ." );
    }
    corr_matrix.all_stat.print( PRN );

    freq_out       = corr_matrix.all_stat.freq;
    time_shift_out = corr_matrix.all_stat.time_shift;
    //corr_val       = corr_matrix.all_stat.max_correlation;
    corr_val = corr_matrix.all_stat.corr_relative_coef();
    if ( found ) {
        return true;
    } else {
        return false;
    }
}

void GPSVis::PreciseFreq(double &freq_out, int &time_shift_out, float &corr_val) {
    double cur_freq = corr_matrix.all_stat.freq;

    for ( int cur_step = 256; cur_step >= 1; cur_step /= 4 ) {

        for ( int fi = -2; fi < 3; fi++ ) {
            double freq = fi * cur_step + cur_freq;
            CalcCorrVector( freq );
            stat_type& st = corr_matrix.cmap[ freq ].stat;
            corr_matrix.all_stat.check( st );
            //fprintf( stderr, "%f ... %f\n", freq, st.max_correlation );
        }
        cur_freq = corr_matrix.all_stat.freq;
        //fprintf( stderr, "\n --> %.0f (corr %.4f)\n", cur_freq, corr_matrix.all_stat.max_correlation );
    }
    fprintf( stderr, "**" );
    corr_matrix.all_stat.print( PRN );

    freq_out       = corr_matrix.all_stat.freq;
    time_shift_out = corr_matrix.all_stat.time_shift;
    //corr_val       = corr_matrix.all_stat.max_correlation;
    corr_val = corr_matrix.all_stat.corr_relative_coef();

    //file_dump( &corr_matrix.cmap[ freq_out ].data[ 0 ], corr_matrix.cmap[ freq_out ].data.size() * 4, "dump.flt" );

}

void GPSVis::GetCorrMatrix( std::vector< std::vector< float > >& ans, std::vector<double>& freqs) {
    const int K = 300;
    ans.resize( corr_matrix.cmap.size() );
    freqs.resize( corr_matrix.cmap.size() );

    int x = 0;
    int center = corr_matrix.all_stat.time_shift;
    corr_map_iter_t it = corr_matrix.cmap.begin();
    while ( it != corr_matrix.cmap.end() ) {
        std::vector< float >& vec = it->second.data;

        ans[ x ].resize( K );
        freqs[ x ] = it->first;
        for ( unsigned int i = 0; i < K; i++ ) {
            int idx = center - K / 2 + i;
            idx = idx % vec.size();
            ans[ x ][ i ] = vec[ idx ];
        }
        ++it;
        x++;
    }
}

void GPSVis::SetEdgeKoef(double k) {
    this->edgeKoef = k;
}







