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


inline void decode4bytes_to_4shorts_ch0( unsigned int code4, short* dst ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    dst[ 0 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH0  <<  0 ) ) >> ( 0 + 0) ];
    dst[ 1 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH0  <<  8 ) ) >> ( 8 + 0) ];
    dst[ 2 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH0  << 16 ) ) >> (16 + 0) ];
    dst[ 3 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH0  << 24 ) ) >> (24 + 0) ];
}

inline void decode4bytes_to_4shorts_ch1( unsigned int code4, short* dst ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    dst[ 0 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH1  <<  0 ) ) >> ( 0 + 2) ];
    dst[ 1 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH1  <<  8 ) ) >> ( 8 + 2) ];
    dst[ 2 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH1  << 16 ) ) >> (16 + 2) ];
    dst[ 3 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH1  << 24 ) ) >> (24 + 2) ];
}

inline void decode4bytes_to_4shorts_ch2( unsigned int code4, short* dst ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    dst[ 0 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH2  <<  0 ) ) >> ( 0 + 4) ];
    dst[ 1 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH2  <<  8 ) ) >> ( 8 + 4) ];
    dst[ 2 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH2  << 16 ) ) >> (16 + 4) ];
    dst[ 3 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH2  << 24 ) ) >> (24 + 4) ];
}

inline void decode4bytes_to_4shorts_ch3( unsigned int code4, short* dst ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    dst[ 0 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH3  <<  0 ) ) >> ( 0 + 6) ];
    dst[ 1 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH3  <<  8 ) ) >> ( 8 + 6) ];
    dst[ 2 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH3  << 16 ) ) >> (16 + 6) ];
    dst[ 3 ] = decode_samples_i16[ ( code4 & ( CODE_MASK_CH3  << 24 ) ) >> (24 + 6) ];
}


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


inline short decode_2bchar_to_short_ch0( unsigned char code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH0 ) ];
}

inline short decode_2bchar_to_short_ch1( unsigned char code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH1 ) >> CODE_SHIFT_CH1 ];
}

inline short decode_2bchar_to_short_ch2( unsigned char code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH2 ) >> CODE_SHIFT_CH2 ];
}

inline short decode_2bchar_to_short_ch3( unsigned char code ) {
    static short decode_samples_i16[4] = {1, 3, -1, -3};
    return decode_samples_i16[ ( code & CODE_MASK_CH3 ) >> CODE_SHIFT_CH3 ];
}



inline float decode_2bchar_to_float_ch0( unsigned char code ) {
    static float decode_samples_f32[4] = {1.0f, 3.0f, -1.0f, -3.0f};
    return decode_samples_f32[ ( code & CODE_MASK_CH0 ) ];
}

inline float decode_2bchar_to_float_ch1( unsigned char code ) {
    static float decode_samples_f32[4] = {1.0f, 3.0f, -1.0f, -3.0f};
    return decode_samples_f32[ ( code & CODE_MASK_CH1 ) >> CODE_SHIFT_CH1 ];
}

inline float decode_2bchar_to_float_ch2( unsigned char code ) {
    static float decode_samples_f32[4] = {1.0f, 3.0f, -1.0f, -3.0f};
    return decode_samples_f32[ ( code & CODE_MASK_CH2 ) >> CODE_SHIFT_CH2 ];
}

inline float decode_2bchar_to_float_ch3( unsigned char code ) {
    static float decode_samples_f32[4] = {1.0f, 3.0f, -1.0f, -3.0f};
    return decode_samples_f32[ ( code & CODE_MASK_CH3 ) >> CODE_SHIFT_CH3 ];
}

#endif
