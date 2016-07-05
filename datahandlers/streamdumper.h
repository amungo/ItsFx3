#ifndef STREAMERDUMPER_H
#define STREAMERDUMPER_H

#ifndef WIN32
#include <stdint.h>
#else
#include <cstdint>
#endif

#include <datastreams/streamdatahandler.h>
#include <datahandlers/filedumpcallbackifce.h>
#include <string>
#include <mutex>
#include "stdio.h"
#include <cstdint>

enum StreamDumpType_t {
    StreamDump_4ch_AllChan_Byte,
    StreamDump_AllData,
    StreamDump_4ch_0ch_Byte,
    StreamDump_4ch_1ch_Byte,
    StreamDump_4ch_2ch_Byte,
    StreamDump_4ch_3ch_Byte,
    StreamDump_TypeCount
};
static const char* StreamDumpTypeNames[ StreamDump_TypeCount ] = {
    "All 4ch data in one byte",
    "All ADC data (with dummy 0xFF)",
    "0-th chan as byte",
    "1-st chan as byte",
    "2-nd chan as byte",
    "3-rd chan as byte"
};


class StreamDumper : public StreamDataHandler
{
public:
    StreamDumper();
    void SetDumpFileName( const char* fname );
    void StartDump( StreamDumpType_t dump_type, bool save_time_stamp = false );
    void StartDump( StreamDumpType_t dump_type, int64_t samples_count, FileDumpCallbackIfce* callback );
    void StopDump();
    
    // StreamDataHandler interface
    virtual void HandleADCStreamData(void* data, size_t size8);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel);

private:
    void CheckStop( int64_t samples );

    std::string fname;
    FILE* dump_file;
    std::mutex fmtx;
    StreamDumpType_t type;
    FileDumpCallbackIfce* cback = nullptr;
    int64_t samples_togo;
    bool oneshot_dumping;
};

#endif // STREAMERDUMPER_H
