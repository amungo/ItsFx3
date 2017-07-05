#ifndef CONVRESULT_H
#define CONVRESULT_H

#include <vector>

class ConvResult
{
public:
    ConvResult( int thetta_cnt = 1, int phi_cnt = 1 );

    void SetDimensions( int thetta_cnt, int phi_cnt );
    void Flush();
    void ReCalc();

    void print_dbg();

    std::vector<std::vector<float>> data;
    float max;
    float min;
    int maxThettaIdx;
    int maxPhiIdx;
};

#endif // CONVRESULT_H
