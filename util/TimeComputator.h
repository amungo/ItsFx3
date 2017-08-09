#ifndef TIMECOMPUTATOR_H
#define TIMECOMPUTATOR_H

#include <string>
#include <cstdint>

class TimeComputator
{
public:
    TimeComputator( std::string name = "time" );
    void Start();
    void Finish();
    void Finish( int bytes );
    void SetPrintPeriod( int n );
private:
    int64_t time_last_mks;
    int print_period;
    int calls_count;
    double time_accum_mks;
    int64_t bytes_accum = 0;
    std::string name;

    int64_t getNowMks();
};

#endif // TIMECOMPUTATOR_H
