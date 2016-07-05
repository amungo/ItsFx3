#ifndef STREAMDATAHANDLER_H
#define STREAMDATAHANDLER_H

#include <cstddef>

class StreamDataHandler {
public:
    virtual void HandleADCStreamData( void* data, size_t size8 ) = 0;
    virtual void HandleStreamDataOneChan( short* one_ch_data, size_t pts_cnt, int channel ) = 0;
};

#endif // STREAMDATAHANDLER_H
