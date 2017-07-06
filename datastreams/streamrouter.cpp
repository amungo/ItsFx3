#include "streamrouter.h"
#include "string.h"
#include "util/Chan2bitParser.h"


StreamRouter::StreamRouter( ADCType type ) :
    loop_running( true ),
    queue_size8( 0 ),
    adc_type( type )
{
    data_handler_thread = std::thread(&StreamRouter::DataHandleLoop, this);
}

StreamRouter::~StreamRouter() {
    loop_running = false;
    HandleADCStreamData( NULL, 0 );
    if ( data_handler_thread.joinable() ) {
        data_handler_thread.join();
    }
}

void StreamRouter::AddOutPoint(StreamDataHandler* handler) {
    mtx_hnd.lock();
    handlers.insert( handler );
    mtx_hnd.unlock();
}

void StreamRouter::DeleteOutPoint(StreamDataHandler* handler) {
    mtx_hnd.lock();
    int erased_count = handlers.erase( handler );
    if ( erased_count == 0 ) {
        fprintf( stderr, "__warning__ DeleteOutPoint(%p) not found\n", handler );
    }
    mtx_hnd.unlock();
}

void StreamRouter::HandleADCStreamData(void* data, size_t size8) {
    uint8_t* buf = NULL;
    StreamData elem;
    if ( data ) {
        buf = new uint8_t[ size8 ];
        memcpy( buf, data, size8 );
        elem.data = buf;
        elem.size8 = size8;
    }
    
    // there are some c++11 magic
    {
        std::lock_guard<std::mutex> lck( mtx_q );
        dqueue.push( elem );
        queue_size8 += size8;
    }
    cond_var.notify_one();
}

void StreamRouter::HandleStreamDataOneChan(short*, size_t, int) {

}

static double ctr_fl = 0.0f;
void StreamRouter::RouteData(void* data, size_t size8) {

    std::vector< int16_t* > chans_data;
    chans_data.resize( 0 );
    bool need_delete = false;

    uint32_t pts_cnt;
    if ( adc_type == ADC_NT1065 || adc_type == ADC_SE4150 || adc_type == ADC_NT1065_File ) {
        pts_cnt = size8 / sizeof( uint8_t );
        chans_data.resize( 4 );

        need_delete = true;
        for ( uint32_t ch = 0; ch < chans_data.size(); ch++ ) {
            chans_data[ ch ] = new int16_t[ pts_cnt ];
        }
        uint8_t* p8 = ( uint8_t* ) data;
        for ( uint32_t i = 0; i < pts_cnt; i+=4 ) {
            chans_data[ 0 ][ i+0 ] = decode_2bchar_to_short_ch0(p8[i+0]);
            chans_data[ 0 ][ i+1 ] = decode_2bchar_to_short_ch0(p8[i+1]);
            chans_data[ 0 ][ i+2 ] = decode_2bchar_to_short_ch0(p8[i+2]);
            chans_data[ 0 ][ i+3 ] = decode_2bchar_to_short_ch0(p8[i+3]);

            chans_data[ 1 ][ i+0 ] = decode_2bchar_to_short_ch1(p8[i+0]);
            chans_data[ 1 ][ i+1 ] = decode_2bchar_to_short_ch1(p8[i+1]);
            chans_data[ 1 ][ i+2 ] = decode_2bchar_to_short_ch1(p8[i+2]);
            chans_data[ 1 ][ i+3 ] = decode_2bchar_to_short_ch1(p8[i+3]);

            chans_data[ 2 ][ i+0 ] = decode_2bchar_to_short_ch2(p8[i+0]);
            chans_data[ 2 ][ i+1 ] = decode_2bchar_to_short_ch2(p8[i+1]);
            chans_data[ 2 ][ i+2 ] = decode_2bchar_to_short_ch2(p8[i+2]);
            chans_data[ 2 ][ i+3 ] = decode_2bchar_to_short_ch2(p8[i+3]);

            chans_data[ 3 ][ i+0 ] = decode_2bchar_to_short_ch3(p8[i+0]);
            chans_data[ 3 ][ i+1 ] = decode_2bchar_to_short_ch3(p8[i+1]);
            chans_data[ 3 ][ i+2 ] = decode_2bchar_to_short_ch3(p8[i+2]);
            chans_data[ 3 ][ i+3 ] = decode_2bchar_to_short_ch3(p8[i+3]);
        }
    } else if ( adc_type == ADC_1ch_16bit ) {
        pts_cnt = size8 / sizeof( uint16_t );
        chans_data.resize( 1 );
        need_delete = false;
        chans_data[ 0 ] = ( int16_t* ) data;

    } else if ( adc_type == ADC_AD9361 ) {
        pts_cnt = size8 / sizeof( uint16_t );
        chans_data.resize( 2 );
        need_delete = true;
        for ( uint32_t ch = 0; ch < chans_data.size(); ch++ ) {
            chans_data[ ch ] = new int16_t[ pts_cnt / 2 ];
        }

        int16_t* p16 = ( int16_t* ) data;
        for ( uint32_t i = 0; i < pts_cnt / 2; i += 4 ) {

            chans_data[ 0 ][ i + 0 ] = *p16++; // I
            chans_data[ 0 ][ i + 1 ] = *p16++; // Q

            chans_data[ 1 ][ i + 0 ] = *p16++; // I
            chans_data[ 1 ][ i + 1 ] = *p16++; // Q

            chans_data[ 0 ][ i + 2 ] = *p16++; // I
            chans_data[ 0 ][ i + 3 ] = *p16++; // Q

            chans_data[ 1 ][ i + 2 ] = *p16++; // I
            chans_data[ 1 ][ i + 3 ] = *p16++; // Q

        }

    } else {
        static int error_count = 0;
        if ( error_count++ % 10 == 0 ) {
            fprintf( stderr, "__error__ StreamRouter::RouteData UNKNOWN adc type %d\n", ( int ) adc_type );
        }
    }
    mtx_hnd.lock();
    std::set< StreamDataHandler* > handlers_copy( handlers );
    mtx_hnd.unlock();
    std::set< StreamDataHandler* >::iterator handler = handlers_copy.begin();
    while ( handler != handlers_copy.end() ) {
        (*handler)->HandleADCStreamData(data, size8);
        (*handler)->HandleAllChansData( chans_data, pts_cnt );
        for ( uint32_t ch = 0; ch < chans_data.size(); ch++ ) {
            uint32_t p_cnt_fixed = pts_cnt;
            if ( adc_type == ADC_AD9361 ) {
                p_cnt_fixed = pts_cnt / 2;
            }
            (*handler)->HandleStreamDataOneChan(chans_data[ ch ], p_cnt_fixed, ch);
        }
        handler++;
    }

    if ( need_delete ) {
        for ( uint32_t ch = 0; ch < chans_data.size(); ch++ ) {
            delete [] chans_data[ ch ];
        }
    }
}

void StreamRouter::onOverrun(uint64_t over_size8, uint32_t over_queue_count) {
    fprintf( stderr, "__warning__ queue overrun: %d %f MBytes\n", over_queue_count, over_size8 / ( 1024.0 * 1024.0 ) );
}

void StreamRouter::DataHandleLoop() {
    uint32_t queue_size = 0;
    
    while ( loop_running ) {
        StreamData elem;

        {
            std::unique_lock<std::mutex> lck( mtx_q );
            queue_size = dqueue.size();
            if ( queue_size ) {
                elem = dqueue.front();
                dqueue.pop();
                lck.unlock();
            } else {
                cond_var.wait( lck );
                queue_size = dqueue.size();
                if ( queue_size ) {
                    elem = dqueue.front();
                    dqueue.pop();
                }
            }
        }

        queue_size8 -= elem.size8;
        
        if ( queue_size > 1 ) {
            onOverrun( queue_size8, queue_size );
        }
        
        if ( loop_running && queue_size && elem.data && elem.size8 ) {
            RouteData( elem.data, elem.size8 );
            delete [] (uint8_t*) elem.data;
        }
    }
}

StreamData::StreamData() :
    data( NULL ),
    size8( 0 )
{
    
}
