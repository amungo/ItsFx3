#ifndef _chan2bit_parser_h_
#define _chan2bit_parser_h_

#define CODE_MASK_CH0 ( 0x03 )
#define CODE_MASK_CH1 ( 0x0C )
#define CODE_MASK_CH2 ( 0x30 )
#define CODE_MASK_CH3 ( 0xC0 )

#define CODE_SHIFT_CH0 ( 0 )
#define CODE_SHIFT_CH1 ( 2 )
#define CODE_SHIFT_CH2 ( 4 )
#define CODE_SHIFT_CH3 ( 6 )

inline float decode2bit_to_float_ch0( unsigned short code ) {
    static float decode_samples_f32[4] = {1.0f, 3.0f, -1.0f, -3.0f};
    return decode_samples_f32[ ( code & CODE_MASK_CH0 ) ];
}

inline short decode2bit_to_short_ch0( unsigned short code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH0 ) ];
}

inline short decode2bit_to_short_ch1( unsigned short code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH1 ) >> CODE_SHIFT_CH1 ];
}

inline short decode2bit_to_short_ch2( unsigned short code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH2 ) >> CODE_SHIFT_CH2 ];
}

inline short decode2bit_to_short_ch3( unsigned short code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH3 ) >> CODE_SHIFT_CH3 ];
}


#endif
