#include "leapconverter.h"

LEAPConverter::LEAPConverter(int32_t SR, double gps_freq) :
    signature( SIGNATURE_DEFAULT ),
    version( VERSION_DEFAULT ),
    timestamp_ticks( TICKS_DEFAULT ),
    samples_per_ms( SR / 1000 ),
    intermediate_freq( gps_freq ),
    chk_count( 1 ),
    chk_gap_ms( 50 ),
    signal_data_format( 1 )
{

}

void LEAPConverter::convertFile(const char *file_name,
                                uint32_t chunks_count,
                                uint32_t chunk_len_ms,
                                uint32_t chunk_gap_ms)
{
    fprintf( stderr, "convertFile( '%s' ) %dx%d ms via %d ms\n", file_name,
             chunks_count, chunk_len_ms, chunk_gap_ms );
    FILE* fin = fopen( file_name, "r" );
    if ( fin ) {
        fread( &timestamp_ticks, sizeof( long long ), 1, fin );
        chk_count   = chunks_count;
        chk_size_ms = chunk_len_ms;
        chk_gap_ms  = chunk_gap_ms;

        uint32_t src_size_pts   = samples_per_ms * chunk_len_ms * chunks_count;
        uint32_t dst_size_bytes = src_size_pts * sizeof( int16_t ) / 8;

        int16_t* src_buf = new int16_t[ src_size_pts ];
        fread( src_buf, sizeof( int16_t ), src_size_pts, fin );
        fclose( fin );

        FILE* fout = fopen( file_name, "wb" );

        writeHeader( fout );

        uint8_t* dest_buf = new uint8_t[ dst_size_bytes ];
        decodeNPoints( src_buf, dest_buf, src_size_pts );
        fwrite( dest_buf, sizeof( uint8_t ), dst_size_bytes, fout );

        delete [] dest_buf;
        delete [] src_buf;

        fclose( fout );
    }
}

void LEAPConverter::writeHeader(FILE *f) {
    if ( f ) {
        fwrite( &signature,          sizeof( int32_t ), 1, f );
        fwrite( &version,            sizeof( int32_t ), 1, f );
        fwrite( &timestamp_ticks,    sizeof( int64_t ), 1, f );
        fwrite( &samples_per_ms,     sizeof( int32_t ), 1, f );
        fwrite( &intermediate_freq,  sizeof(  double ), 1, f );
        fwrite( &chk_count,        sizeof( int32_t ), 1, f );
        fwrite( &chk_size_ms,      sizeof( int32_t ), 1, f );
        fwrite( &chk_gap_ms,       sizeof( int32_t ), 1, f );
        fwrite( &signal_data_format, sizeof( int32_t ), 1, f );
    }
}

void LEAPConverter::decode8points(int16_t* src, uint8_t* dest) {
    uint8_t* dst_sign  = &dest[ 0 ];
    uint8_t* dst_value = &dest[ 1 ];
    *dst_sign  = 0;
    *dst_value = 0;
    for ( int i = 0; i < 8; i++ ) {
        int16_t data = src[ i ];
        uint8_t sign  = ( data < 0 );
        uint8_t value = ( data == 3 || data == -3 );
        //fprintf( stderr, "%d -> %d %d\n", data, sign, value );
        
        *dst_sign  |= ( sign  << i );
        *dst_value |= ( value << i );
    }
    //fprintf( stderr, "%02X %02X\n", *dst_sign, *dst_value );
}

void LEAPConverter::decodeNPoints( int16_t* src, uint8_t* dest, int N ) {
    for ( int i = 0; i < N / 8; i++ ) {
        decode8points( src, dest );
        src  += 8;
        dest += 2;
    }
}

