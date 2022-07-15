#include "streamrouter.h"
#include "string.h"
#include "util/Chan2bitParser.h"

StreamRouter::StreamRouter( ADCType type ) :
    loop_running( true ),
    queue_size8( 0 ),
    adc_type( type ),
    hack_len( 0 ),
    tc("convert")
{
    data_handler_thread = std::thread(&StreamRouter::DataHandleLoop, this);
    tc.SetPrintPeriod(200);
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

void StreamRouter::HandleStreamDataOneChan(short*, size_t, int, int) {

}

void StreamRouter::SetHackedLen(int hacked_len) {
    this->hack_len = hacked_len;
}

static double ctr_fl = 0.0f;
void StreamRouter::RouteData(void* data, size_t size8) {
    if ( hack_len ) {
        size8 = hack_len;
    }

    std::vector< int16_t* > chans_data0;
    std::vector< int16_t* > chans_data1;
    std::vector< char> gyro_data;
    chans_data0.resize( 0 );
    chans_data1.resize( 0 );
    gyro_data.resize(NUT4_GYRO_HEADER_SIZE);

    size_t pkt_count = size8 / NUT4_GYRO_DATA_PACKET_SIZE;
    size_t pkt_data_size = NUT4_DATA_SIZE;
    size_t to_go = size8;

    bool need_delete = false;

    uint32_t pts_cnt;
    if ( adc_type == ADC_NT1065 || adc_type == ADC_SE4150 || adc_type == ADC_NT1065_File ) {
        tc.Start();
        pts_cnt = (size8 / sizeof( uint8_t )) - (NUT4_GYRO_HEADER_SIZE * pkt_count);

        chans_data0.resize( 4 );
        chans_data1.resize( 4 );

        need_delete = true;
        for ( uint32_t ch = 0; ch < chans_data0.size(); ch++ ) {
            chans_data0[ ch ] = new int16_t[ pts_cnt / 2];
            chans_data1[ ch ] = new int16_t[ pts_cnt / 2];
        }

        uint8_t* p8 = ( uint8_t* )data;
        p8 += NUT4_GYRO_HEADER_SIZE;
        size_t data_counter = 0;

        while(to_go > 0)
        {
            ::memcpy(gyro_data.data(), p8 - NUT4_GYRO_HEADER_SIZE, NUT4_GYRO_HEADER_SIZE);
            for ( uint32_t i = 0; i < pkt_data_size; i+=2)
            {
                chans_data0[ 0 ][data_counter] = decode_2bchar_to_short_ch0(p8[i]);
                chans_data0[ 1 ][data_counter] = decode_2bchar_to_short_ch1(p8[i]);
                chans_data0[ 2 ][data_counter] = decode_2bchar_to_short_ch2(p8[i]);
                chans_data0[ 3 ][data_counter] = decode_2bchar_to_short_ch3(p8[i]);

                chans_data1[ 0 ][data_counter] = decode_2bchar_to_short_ch0(p8[i+1]);
                chans_data1[ 1 ][data_counter] = decode_2bchar_to_short_ch1(p8[i+1]);
                chans_data1[ 2 ][data_counter] = decode_2bchar_to_short_ch2(p8[i+1]);
                chans_data1[ 3 ][data_counter] = decode_2bchar_to_short_ch3(p8[i+1]);

                data_counter ++;
            }

            to_go -= NUT4_GYRO_DATA_PACKET_SIZE;
            p8 += NUT4_GYRO_DATA_PACKET_SIZE;
        }

        tc.Finish(size8+pts_cnt*sizeof(short)*4);
    } else if ( adc_type == ADC_1ch_16bit ) {
        pts_cnt = size8 / sizeof( uint16_t );
        chans_data0.resize( 1 );
        need_delete = false;
        chans_data0[ 0 ] = ( int16_t* ) data;

    } else if ( adc_type == ADC_AD9361 ) {
        pts_cnt = size8 / sizeof( uint16_t );
        chans_data0.resize( 2 );
        need_delete = true;
        for ( uint32_t ch = 0; ch < chans_data0.size(); ch++ ) {
            chans_data0[ ch ] = new int16_t[ pts_cnt / 2 ];
        }

        int16_t* p16 = ( int16_t* ) data;
        for ( uint32_t i = 0; i < pts_cnt / 2; i += 4 ) {

            chans_data0[ 0 ][ i + 0 ] = *p16++; // I
            chans_data0[ 0 ][ i + 1 ] = *p16++; // Q

            chans_data0[ 1 ][ i + 0 ] = *p16++; // I
            chans_data0[ 1 ][ i + 1 ] = *p16++; // Q

            chans_data0[ 0 ][ i + 2 ] = *p16++; // I
            chans_data0[ 0 ][ i + 3 ] = *p16++; // Q

            chans_data0[ 1 ][ i + 2 ] = *p16++; // I
            chans_data0[ 1 ][ i + 3 ] = *p16++; // Q
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
        (*handler)->HandleAllChansData( chans_data0, pts_cnt );
        (*handler)->HandleGyroData(gyro_data.data(), NUT4_GYRO_HEADER_SIZE);
        for ( uint32_t ch = 0; ch < chans_data0.size(); ch++ ) {
            uint32_t p_cnt_fixed = pts_cnt / 2;
            if ( adc_type == ADC_AD9361 ) {
                p_cnt_fixed = pts_cnt / 2;
            }
            (*handler)->HandleStreamDataOneChan(chans_data0[ ch ], p_cnt_fixed, 0, ch);
            (*handler)->HandleStreamDataOneChan(chans_data1[ ch ], p_cnt_fixed, 1, ch);
        }
        handler++;
    }

    if ( need_delete ) {
        for ( uint32_t ch = 0; ch < chans_data0.size(); ch++ ) {
            delete [] chans_data0[ ch ];
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
