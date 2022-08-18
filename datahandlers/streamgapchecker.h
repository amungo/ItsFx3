#ifndef STREAMGAPCHECKER_H
#define STREAMGAPCHECKER_H

#include "datastreams/streamdatahandler.h"


class GapCounterCallBackIfce {
public:
    virtual void GapCountChanged( int gap_count, double max_gap_value ) = 0;
};

class StreamGapChecker : public StreamDataHandler
{
public:
    StreamGapChecker();
    ~StreamGapChecker();

    // StreamDataHandler interface
public:
    void HandleADCStreamData(void *data, size_t size8);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int chip, int channel);

    void FlushStatistic();

    void SetCallBack( GapCounterCallBackIfce* cb_class );
    void SetGap( double gap_value );

    int     GetGapCount();
    double  GetMaxGapValue();

private:
    double gapval;
    double maxgap;
    int gapcnt;
    GapCounterCallBackIfce* cb;
    bool first_run;

    short last_val;
};

#endif // STREAMGAPCHECKER_H
