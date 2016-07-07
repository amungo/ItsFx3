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
    void SetPrintPeriod( int n );
private:
    int64_t time_last_mks;
    int print_period;
    int calls_count;
    double time_accum_mks;
    std::string name;

    int64_t getNowMks();
};

#endif // TIMECOMPUTATOR_H
