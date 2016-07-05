#include "streamgapchecker.h"
#include <stdlib.h>

StreamGapChecker::StreamGapChecker() :
    gapval( 100.0 ),
    maxgap( 0.0001 ),
    gapcnt( 0 ),
    cb( NULL ),
    first_run( true )
{

}

StreamGapChecker::~StreamGapChecker() {

}

void StreamGapChecker::HandleADCStreamData(void*, size_t) {
    // nop
}

void StreamGapChecker::HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int) {
    unsigned short gapval16 = ( unsigned short ) gapval;
    unsigned short maxgap16 = ( unsigned short ) maxgap;
    bool state_changed = false;

    int gc = 0;
    short prev = last_val;
    if ( first_run ) {
        prev = one_ch_data[ 0 ];
    }
    for ( size_t i = 0; i < pts_cnt; i++ ) {
        unsigned short diff = abs( one_ch_data[ i ] - prev );
        prev = one_ch_data[ i ];
        if ( diff > gapval16 ) {
            gc++;
            state_changed = true;
        }
        if ( diff > maxgap16 ) {
            maxgap16 = diff;
            state_changed = true;
        }
    }
    last_val = prev;

    if ( state_changed || first_run ) {
        maxgap = ( double ) maxgap16;
        gapcnt += gc;
        if ( cb ) {
            cb->GapCountChanged( gapcnt, maxgap );
        }
    }
    first_run = false;
}

void StreamGapChecker::FlushStatistic() {
    gapval = 0.0;
    maxgap = 0.0001;
    gapcnt = 0;
    first_run = true;
}

void StreamGapChecker::SetCallBack(GapCounterCallBackIfce *cb_class) {
    this->cb = cb_class;
}

void StreamGapChecker::SetGap(double gap_value) {
    gapval = gap_value;
}

int StreamGapChecker::GetGapCount() {
    return gapcnt;
}

double StreamGapChecker::GetMaxGapValue() {
    return maxgap;
}



