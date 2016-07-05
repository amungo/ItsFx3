#include "TimeComputator.h"
#include <cstdio>

#ifdef WIN32
static void gettimeofday( struct timeval* tv, void* ) {
    LARGE_INTEGER ticks;
    QueryPerformanceCounter( &ticks );

    LARGE_INTEGER ticks_per_second;
    QueryPerformanceFrequency(&ticks_per_second);

    long long mks = ( 1000000 * ticks.QuadPart ) / ticks_per_second.QuadPart;

    tv->tv_sec  = mks / 1000000;
    tv->tv_usec = mks % 1000000;
}

#endif

TimeComputator::TimeComputator( std::string name ) :
    print_period( 1 ),
    calls_count( 0 ),
    time_accum_mks( 0.0 ),
    name( name )
{

}

void TimeComputator::Start() {
    gettimeofday( &time_last, 0 );
}

void TimeComputator::Finish() {
    struct timeval time_now;
    gettimeofday( &time_now, 0 );
    time_accum_mks += time_now.tv_usec - time_last.tv_usec + 1000000.0 * ( time_now.tv_sec - time_last.tv_sec );

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


