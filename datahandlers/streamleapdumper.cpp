#include "streamleapdumper.h"
#include <cstdio>

StreamLEAPDumper::StreamLEAPDumper() :
    state( DS_NoDumping ),
    file( NULL )
{
    
}

void StreamLEAPDumper::HandleADCStreamData(void* data, size_t size8) {
    mtx.lock();
    if ( state != DS_NoDumping ) {
        if ( pars.all_chans_raw_sig ) {
            processData( ( uint8_t* ) data, size8 );
        }
    }
    mtx.unlock();
}

void StreamLEAPDumper::HandleStreamDataOneChan(short* one_ch_data, size_t pts_cnt, int chan, int channel) {
    mtx.lock();
    if ( state != DS_NoDumping ) {
        if ( !pars.all_chans_raw_sig && pars.chan_num == channel ) {
            processData( ( uint8_t* ) one_ch_data, pts_cnt * sizeof( short ) );
        }
    }
    mtx.unlock();
}


int StreamLEAPDumper::DumpAsync(const char* file_name, ChunkDumpParams params, ChunkDumpCallbackIfce* callback) {
    file = fopen( file_name, "wb" );
    if ( !file ) {
        return -1;
    }

    mtx.lock();
    fname         = file_name;
    cback         = callback;
    pars          = params;
    have_count    = 0;
    cur_chunk_len = 0;
    cur_gap       = 0;
    state         = DS_Start;

    writeDateTime();

    mtx.unlock();
    return 0;
}

void StreamLEAPDumper::processData(uint8_t *p, int size) {

    while ( size > 0 && have_count < pars.count ) {
        //fprintf( stderr, "[%d] %d\n", state, size );
        if ( state == DS_ChunkProcess ) {
            // "FILLING CHUNK"
            int to_end = pars.len - cur_chunk_len;

            if ( size >= to_end ) {
                fwrite( p, 1, to_end, file );
                p    += to_end;
                size -= to_end;

                cur_chunk_len = 0;
                have_count++;

                state = DS_GapProcess;
                cur_gap = 0;
            } else {  // if ( size < pars.len - cur_chunk_len)
                fwrite( p, 1, size, file );
                p += size;

                cur_chunk_len += size;
                size = 0;
            }

        } else if ( state == DS_GapProcess ) {
            // "SKIPPING GAP"
            int to_end = pars.gap - cur_gap;

            if ( size >= to_end ) {
                p    += to_end;
                size -= to_end;
                cur_gap = 0;

                state = DS_ChunkProcess;
                cur_chunk_len = 0;
            } else {  // if ( size < pars.gap - cur_gap)
                p += size;
                cur_gap += size;
                size = 0;
            }

        } else {
            fprintf( stderr, "__error__ StreamLEAPDumper::processData() invalid state %d\n", (int)state );
            break;
        }
    }

    if ( have_count >= pars.count ) {
        fclose( file );
        state = DS_NoDumping;
        if ( cback ) {
            cback->onFileDumpComplete( fname, pars );
        }
    }

}

long long StreamLEAPDumper::GetUTCTicks() {
    time_t t = time(0);
    long long ticks = t;
    ticks += 62135596800LL;
    ticks *= 10000000LL;
    return ticks;
}

void StreamLEAPDumper::writeDateTime() {
    long long now_ticks = GetUTCTicks();
    fwrite( &now_ticks, sizeof( long long ), 1, file );

    state = DS_ChunkProcess;
}


















