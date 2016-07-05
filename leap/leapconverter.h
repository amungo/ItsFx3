#ifndef LEAPCONVERTER_H
#define LEAPCONVERTER_H

#if defined(__GNUC__) || defined(__MINGW32__)
#include "stdint.h"
#else
#include <cstdint>
#endif

#include <cstdio>

class LEAPConverter
{
public:
    LEAPConverter( int32_t SR, double gps_freq );
    
    void convertFile( const char* file_name,
                      uint32_t chunks_count,
                      uint32_t chunk_len_ms,
                      uint32_t chunk_gap_ms );
    
private:
    void writeHeader( FILE* f );
    void decode8points( int16_t* src, uint8_t* dest );
    void decodeNPoints( int16_t* src, uint8_t* dest, int N );
    
private:
    int32_t signature;
    int32_t version;
    int64_t timestamp_ticks;
    int32_t samples_per_ms;
    double  intermediate_freq;
    int32_t chk_count;
    int32_t chk_size_ms;
    int32_t chk_gap_ms;
    int32_t signal_data_format;
    
    static const int32_t SIGNATURE_DEFAULT = 0x5041454C;
    static const int32_t VERSION_DEFAULT   = 0x00010001;
    static const int64_t TICKS_DEFAULT     = 634688074793800000LL;
                                           
    
    
};

#endif // LEAPCONVERTER_H
