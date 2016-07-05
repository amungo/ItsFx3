#ifndef TIMECOMPUTATOR_H
#define TIMECOMPUTATOR_H

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <string>

class TimeComputator
{
public:
    TimeComputator( std::string name = "time" );
    void Start();
    void Finish();
    void SetPrintPeriod( int n );
private:
    struct timeval time_last;
    int print_period;
    int calls_count;
    double time_accum_mks;
    std::string name;
};

#endif // TIMECOMPUTATOR_H
