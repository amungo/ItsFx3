#ifndef _att_map_h_
#define _att_map_h_

#include <map>
#include <vector>

#if defined(__GNUC__) || defined (__MINGW32__)
#include <stdint.h>
#else
#include <cstdint>
#endif

typedef struct one_att_bit_t {
    uint32_t pos;
    uint32_t force;
    bool is_set;
} one_att_bit_t;


class AttMap {
public:
    void Clear();
    void AddBit( uint32_t bit_pos, uint32_t bit_att_force );
    void Init();
    void AddAttValue( uint32_t att_val, uint32_t reg_val );
    uint32_t GetRegForAtt( uint32_t att_val );
    std::vector<uint32_t> GetAttVector() const;
private:
    void recursiveCalc( uint32_t bit_num );
    std::map< uint32_t, uint32_t > attmap;
    std::vector< one_att_bit_t > bits;
};


#endif
