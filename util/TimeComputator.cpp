#include "TimeComputator.h"
#include <cstdio>
#include <chrono>

TimeComputator::TimeComputator( std::string name ) :
    print_period( 1 ),
    calls_count( 0 ),
    time_accum_mks( 0.0 ),
    name( name )
{

}

void TimeComputator::Start() {
    time_last_mks = getNowMks();
}

void TimeComputator::Finish() {
    int64_t time_now = getNowMks();
    time_accum_mks += time_now - time_last_mks;

    calls_count++;

    if ( calls_count >= print_period ) {
        double one_call_time_mks = ( time_accum_mks / ( double ) calls_count );
        fprintf( stderr, "[%s] %.0f mks (%d calls)\n", name.c_str(), one_call_time_mks, calls_count );
        calls_count = 0;
        time_accum_mks = 0.0;
    }
}

void TimeComputator::SetPrintPeriod(int n) {
    print_period = n;
}

int64_t TimeComputator::getNowMks() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


