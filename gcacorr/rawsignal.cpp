#include "rawsignal.h"



RawSignal::RawSignal(int pts_count, double sample_rate) :
    N( pts_count ),
    FILTER_WIDTH( 1000.0 ),
    SR( sample_rate ),
    fft( pts_count )
{
    signal_source = new float_cpx_t[ N ];
    signal_fft = new float_cpx_t[ N ];
}

RawSignal::~RawSignal() {
    ClearShiftedCache();
    delete [] signal_source;
    delete [] signal_fft;
}

void RawSignal::LoadDataFromFile(const char *fileName, DataType dtype, size_t offset_pts) {
    FILE* f = fopen( fileName, "rb" );
    if ( !f ) {
        fprintf( stderr, "RawSignal::LoadDataFromFile() file IO ERROR\n" );
    }


    if ( dtype == DT_INT8_REAL ) {
        uint32_t len = N + sizeof( int8_t ) * offset_pts;
        int8_t* p8 = new int8_t[ len ];
        fread( p8, sizeof( int8_t ), len, f );
        LoadData( p8, dtype, offset_pts );
        delete [] p8;
    } else if ( dtype == DT_INT16_REAL ) {
        uint32_t len = N + sizeof( int16_t ) * offset_pts;
        int16_t* p16 = new int16_t[ len ];
        fread( p16, sizeof( int16_t ), len, f );
        LoadData( p16, dtype, offset_pts );
        delete [] p16;
    } else {
        fprintf( stderr, "RawSignal::LoadDataFromFile() error data type unknown\n" );
    }
}

void RawSignal::LoadData(void *data, DataType dtype, uint32_t offset) {
    ClearShiftedCache();

    if ( dtype == DT_INT8_REAL ) {
        int8_t* p8 = ( int8_t* ) data;
        for ( int i = 0; i < N; i++ ) {
            signal_source[ i ].i = ( float ) p8[ i + offset ];
            signal_source[ i ].q = 0.0f;
        }
    } else if ( dtype == DT_INT16_REAL ) {
        int16_t* p16 = ( int16_t* ) data;
        for ( int i = 0; i < N; i++ ) {
            signal_source[ i ].i = ( float ) p16[ i + offset ];
            signal_source[ i ].q = 0.0f;
        }
    } else if ( dtype == DT_FLOAT_IQ ) {
        float_cpx_t* pIQF = ( float_cpx_t* ) data;
        for ( int i = 0; i < N; i++ ) {
            signal_source[ i ] = pIQF[ i + offset ];
        }
    } else {
        fprintf( stderr, "RawSignal::LoadData() error data type unknown\n" );
    }
    //file_dump( signal_source, N*8, "sig.flt" );
    MakeSignalFFT();
}


const float_cpx_t* RawSignal::GetSignalShifted(double freq) {
    //fprintf( stderr, "RawSignal::GetSignalShifted( %.0f )\n", freq );
    std::map< double, float_cpx_t* >::iterator it;
    it = signal_shifted.find( freq );
    if ( it == signal_shifted.end() ) {
        // Generate new

        int rotIdx = floor( freq / FILTER_WIDTH );
        float_cpx_t* sig;
        if ( abs( rotIdx * FILTER_WIDTH - freq ) < 0.5 ) {
            // can use fast alg - precision is 1 FILTER_WIDTH = 1 kHz
            sig = circle_shift( signal_fft, N, rotIdx );
        } else {
            // have to use slow alg - precision is 1 Hz (in theroy)
            sig = freq_shift( signal_source, N, SR, freq );
            fft.Transform( sig, sig, false );
        }
        signal_shifted[ freq ] = sig;
        return sig;
    } else {
        return it->second;
    }
}

void RawSignal::MakeSignalFFT() {
    fft.Transform( signal_source, signal_fft, false );
    //file_dump( signal_fft, N*8, "sig_fft.flt" );
    //float* spec = make_magnitude_spectrum( signal_fft, N );
    //file_dump( spec, N*4, "sig_spec.flt" );
}

void RawSignal::ClearShiftedCache() {
    std::map< double, float_cpx_t* >::iterator it = signal_shifted.begin();
    while ( it != signal_shifted.end() ) {
        delete it->second;
        ++it;
    }
}
