#ifndef STREAMLEAPDUMPER_H
#define STREAMLEAPDUMPER_H

#include <cstdint>
#include <datastreams/streamdatahandler.h>
#include <string>
#include <mutex>
#include "stdio.h"


// Example:
// XX.....XX.....XX.....XX
// count = 4 chunks
// len   = 2 bytes
// gap   = 5 bytes
struct ChunkDumpParams {
    int count;
    int len;
    int gap;
    bool all_chans_raw_sig;
    int chan_num;
};


enum DumpState_e {
    DS_NoDumping,
    DS_Start,
    DS_ChunkProcess,
    DS_GapProcess
};

class ChunkDumpCallbackIfce {
public:
    virtual void onFileDumpComplete( std::string fname, ChunkDumpParams params ) = 0;
};

class StreamLEAPDumper  : public StreamDataHandler
{
public:
    StreamLEAPDumper();
    int DumpAsync( const char* file_name, ChunkDumpParams params, ChunkDumpCallbackIfce* callback );
    
    // StreamDataHandler interface
    virtual void HandleADCStreamData(void* data, size_t size8);
    virtual void HandleStreamDataOneChan(short* one_ch_data, size_t pts_cnt, int channel);
    

private:    
    long long GetUTCTicks();
    void writeDateTime();
    void processData( uint8_t* p, int size );

private:
    std::string fname;
    FILE* file;
    ChunkDumpCallbackIfce* cback;
    std::mutex mtx;
    
    ChunkDumpParams pars;
    int have_count;
    int cur_chunk_len;
    int cur_gap;
    DumpState_e state;
};

#endif // STREAMLEAPDUMPER_H
