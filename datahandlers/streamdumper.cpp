#include "streamdumper.h"
#include <ctime>

long long GetUTCTicks() {
    time_t t = time(0);
    long long ticks = t;
    ticks += 62135596800LL;
    ticks *= 10000000LL;
    return ticks;
}



StreamDumper::StreamDumper() :
    dump_file( NULL )
{
    
}

void StreamDumper::SetDumpFileName(const char* fname) {
    fmtx.lock();
    this->fname = fname;
    fmtx.unlock();
}

void StreamDumper::StartDump(StreamDumpType_t dump_type, bool save_time_stamp) {
    fprintf( stderr, "Start dumping %s timestamp to file '%s'\n",
             save_time_stamp ? "with" : "without", fname.c_str() );

    fmtx.lock();
    type = dump_type;
    oneshot_dumping = false;
    dump_file = fopen( fname.c_str(), "wb" );
    if ( save_time_stamp ) {
        long long now_ticks = GetUTCTicks();
        fwrite( &now_ticks, sizeof( long long ), 1, dump_file );
    }
    fmtx.unlock();
}

void StreamDumper::StartDump(StreamDumpType_t dump_type, int64_t samples_count, FileDumpCallbackIfce *callback) {
    fmtx.lock();
    type = dump_type;
    oneshot_dumping = true;
    dump_file = fopen( fname.c_str(), "wb" );
    samples_togo = samples_count;
    cback = callback;
    fmtx.unlock();
}


void StreamDumper::StopDump() {
    fmtx.lock();
    if ( dump_file ) {
        fclose( dump_file );
        dump_file = NULL;
    }
    fmtx.unlock();

    if ( cback ) {
        cback->onFileDumpComplete();
        cback = nullptr;
    }
}


void StreamDumper::HandleADCStreamData(void* data, size_t size8) {
    int64_t samples = 0;
    fmtx.lock();
    if ( dump_file ) {
        if ( type == StreamDump_AllData ) {
            size_t wrote = fwrite( data, 1, size8, dump_file );
            if ( wrote != size8 ) {
                fprintf( stderr, "__error__ StreamDumper::HandleStreamData() wrote %u of %u!\n", wrote, size8 );
            }
            samples = size8 / sizeof(int8_t);

        }
    }
    fmtx.unlock();
    if ( samples ) {
        CheckStop( samples );
    }
}

void StreamDumper::HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int, int channel) {
    int samples = 0;
    fmtx.lock();
    if ( dump_file ) {
        if ( ( type == StreamDump_4ch_0ch_Byte && channel == 0 ) ||
             ( type == StreamDump_4ch_1ch_Byte && channel == 1 ) ||
             ( type == StreamDump_4ch_2ch_Byte && channel == 2 ) ||
             ( type == StreamDump_4ch_3ch_Byte && channel == 3 )
             )
        {
            int8_t* dst = new int8_t[ pts_cnt ];

            for ( uint32_t i = 0; i < pts_cnt; i++ ) {
                dst[ i ] = ( int8_t ) one_ch_data[ i ];
            }

            size_t wrote = fwrite( dst, sizeof( int8_t ), pts_cnt, dump_file );
            if ( wrote != pts_cnt ) {
                fprintf( stderr, "__error__ StreamDumper::HandleStreamData() wrote %u of %u!\n", wrote, pts_cnt );
            }

            delete [] dst;

            samples = pts_cnt;
        }
    }
    fmtx.unlock();
    if ( samples ) {
        CheckStop( samples );
    }
}

void StreamDumper::CheckStop( int64_t samples ) {
    if ( oneshot_dumping ) {
        samples_togo -= samples;
        if ( samples_togo <= 0 ) {
            StopDump();
        }
    }
}

