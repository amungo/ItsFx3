#ifndef STREAMDATAHANDLER_H
#define STREAMDATAHANDLER_H

#include <cstddef>
#include <vector>

class StreamDataHandler {
public:
    virtual void HandleADCStreamData( void* data, size_t size8 ) {}
    virtual void HandleStreamDataOneChan( short* one_ch_data, size_t pts_cnt, int channel ) {}
    virtual void HandleAllChansData( std::vector<short*>& all_ch_data, size_t pts_cnt ) {}
};

#endif // STREAMDATAHANDLER_H
