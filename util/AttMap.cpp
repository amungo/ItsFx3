#include "AttMap.h"
#include <algorithm>

void AttMap::Clear() {
    attmap.clear();
    bits.resize(0);
}

void AttMap::AddBit(uint32_t bit_pos, uint32_t bit_att_force) {
    one_att_bit_t bit;
    bit.pos = bit_pos;
    bit.force = bit_att_force;
    bit.is_set = false;
    bits.push_back( bit );
}

void AttMap::Init() {
    recursiveCalc( 0 );
}

void AttMap::AddAttValue(uint32_t att_val, uint32_t reg_val) {
    attmap[ att_val ] = reg_val;
}

uint32_t AttMap::GetRegForAtt(uint32_t att_val) {
    
    return attmap[ att_val ];
}

std::vector<uint32_t> AttMap::GetAttVector() const {
    std::vector<uint32_t> atts;
    atts.resize( attmap.size() );
    uint32_t i = 0;
    std::map< uint32_t, uint32_t >::const_iterator cit = attmap.begin();
    while ( cit != attmap.end() ) {
        atts[i++] = cit->first;
        cit++;
    }
    std::sort( atts.begin(), atts.end() );
    return atts;
}

void AttMap::recursiveCalc(uint32_t bit_num) {
    if ( bit_num == bits.size() ) {
        uint32_t att = 0;
        uint32_t reg = 0;
        for ( uint32_t i = 0; i < bits.size(); i++ ) {
            if ( bits[i].is_set ) {
                att += bits[i].force;
                reg += ( 1 << bits[i].pos );
            }
        }
        AddAttValue(att, reg);
    } else {
        bits[bit_num].is_set = true;
        recursiveCalc( bit_num + 1 );
        bits[bit_num].is_set = false;
        recursiveCalc( bit_num + 1 );
    }
}
