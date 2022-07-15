#ifndef STREAMROUTER_H
#define STREAMROUTER_H

#include <queue>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "streamdatahandler.h"
#include "hwfx3/fx3config.h"
#include "util/TimeComputator.h"


#define NUT4_GYRO_DATA_PACKET_SIZE  16384
#define NUT4_GYRO_HEADER_SIZE       18 //20      // (6 bytes 3xADXRS450 + 12 bytes ADXL357 + 2 bytes padding)
#define NUT4_DATA_SIZE              16366 // 16364        //16366 @camry

struct StreamData {
    StreamData();
    void* data;
    size_t size8;
};

class StreamRouter : public StreamDataHandler
{
public:
    StreamRouter( ADCType type );
    virtual ~StreamRouter();
    void AddOutPoint( StreamDataHandler* handler );
    void DeleteOutPoint( StreamDataHandler* handler );
    
    // StreamDataHandler interface
    virtual void HandleADCStreamData(void* data, size_t size8);
    virtual void HandleStreamDataOneChan( short* one_ch_data, size_t pts_cnt, int chip, int channel );

    void SetHackedLen( int hacked_len );
protected:
    virtual void RouteData( void* data, size_t size8 );
    virtual void onOverrun( uint64_t over_size8, uint32_t over_queue_count );
    
private:
    std::set< StreamDataHandler* > handlers;
    std::queue< StreamData > dqueue;
    
    std::mutex mtx_q, mtx_hnd;
    std::thread data_handler_thread;
    void DataHandleLoop(void);
    bool loop_running;
    std::condition_variable cond_var;
    
    uint64_t queue_size8;
    ADCType adc_type;

    int hack_len;
    TimeComputator tc;

};

#endif
