#ifndef FX3CONFIG
#define FX3CONFIG

#include "fx3devdrvtype.h"
#include <string>

// ITs FX3
// GPS L1   = 1575.42 MHz
// RCV FREQ = 1590.00 MHz

#define GPS_L1_FREQ ( 1575.42e6 )

//#define RCV_SR          (  53.000e6 )
//#define GPS_FREQ_IN_RCV  ( -14.58e6 )
//#define ADC_DATA_TYPE   ( DT_INT16_REAL )
//#define DATA_SIZE       (    53000 )

// ITs FX3 GPS special w injector
#define RCV_SR          ( 16.368e6 )
#define GPS_FREQ_IN_RCV  ( 4.092e6 )
//#define ADC_DATA_TYPE   ( DT_INT16_REAL )
//#define DATA_SIZE       ( 16368 )


//#define DEF_FW_NAME "fx3_newsub_injector.img"
//#define DEF_FW_NAME "fx3_att_dbg.img"
#define DEF_FW_NAME "AmungoItsFx3Firmware-cs2.img"

enum ADCType {
    ADC_NT1065,
    ADC_NT1065_File,
    ADC_SE4150,
    ADC_AD9361,
    ADC_1ch_16bit,
    ADC_Types_Count
};

enum SignalTypeIQR {
    SigTypeRealPartOnly,
    SigTypeIQParts
};


struct FX3Config {
    ADCType adc_type;
    DriverType_t drv_type;
    int chan_count;
    SignalTypeIQR signal_type;
    bool have_submodules;
    bool have_atts;
    bool have_dbg;
    std::string fn_hex;
    std::string fn_img;
    std::string fn_bit;
    double adc_sample_rate_hz;
    double inter_freq_hz;
    bool auto_start_streams;
    int hacked_len = 0;

    FX3Config() :
        adc_type( ADC_1ch_16bit ),
        drv_type( DrvTypeLibUsb ),
        chan_count( 1 ),
        signal_type( SigTypeRealPartOnly ),
        have_submodules( false ),
        have_atts( false ),
        have_dbg( false ),
        fn_hex( "default_fix_ADC_OUTCLK_bak_full.hex" ),
        fn_img( DEF_FW_NAME ),
        adc_sample_rate_hz( RCV_SR ),
        inter_freq_hz( GPS_FREQ_IN_RCV ),
        auto_start_streams( true )
    {}

    static const char* GetAdcString( ADCType t ) {
        switch( t ) {
        case ADC_1ch_16bit: return "ADC_1ch_16bit";
        case ADC_SE4150:    return "SE4150";
        case ADC_NT1065:    return "NT1065";
        case ADC_NT1065_File:    return "NT1065_File";
        case ADC_AD9361:    return "AD9361";
        default: return "Unknown ADC type";
        }
    }

    void FinishConfigure() {
        switch ( adc_type ) {
        case ADC_1ch_16bit:
            chan_count = 1;
            break;
        case ADC_SE4150:
            chan_count = 4;
            break;
        case ADC_NT1065:
            chan_count = 4;
            break;
        case ADC_AD9361:
            chan_count = 2;
            break;
        }
    }

    void ConfigDefault( ADCType newtype ) {
        switch ( newtype ) {
        case ADC_1ch_16bit:
            adc_type        = ADC_1ch_16bit;
            drv_type        = DrvTypeCypress;
            chan_count      = 1;
            signal_type     = SigTypeRealPartOnly;
            have_submodules = false;
            have_atts       = true;
            have_dbg        = true;
            fn_hex          = "";
            fn_img          = "fx3_att_dbg.img";
            adc_sample_rate_hz = 120.000e6;
            inter_freq_hz  = 0.0;
            break;
        case ADC_SE4150:
            adc_type        = ADC_SE4150;
            drv_type        = DrvTypeCypress;
            chan_count      = 4;
            signal_type     = SigTypeRealPartOnly;
            have_submodules = false;
            have_atts       = false;
            have_dbg        = true;
            fn_hex          = "";
            fn_img          = "fx3_newsub_injector.img";
            adc_sample_rate_hz = 16.368e6;
            inter_freq_hz      = GPS_L1_FREQ - 4.092e6;
            break;
        case ADC_NT1065:
            adc_type        = ADC_NT1065;
            drv_type        = DrvTypeCypress;
            chan_count      = 4;
            signal_type     = SigTypeRealPartOnly;
            have_submodules = true;
            have_atts       = false;
            have_dbg        = true;
            fn_hex          = "ConfigSet_all_GPS_L1_patched_ldvs.hex"; //"manual";
            //fn_img          = "D://work//raw_signal.img";
            fn_img          = DEF_FW_NAME;
            fn_bit          = "top.bit";
            adc_sample_rate_hz = 53.0e6;
            inter_freq_hz      = 1590.0e6;
            break;
        case ADC_NT1065_File:
            adc_type        = ADC_NT1065_File;
            drv_type        = DrvTypeFileSim;
            chan_count      = 4;
            signal_type     = SigTypeRealPartOnly;
            have_submodules = false;
            have_atts       = false;
            have_dbg        = false;
            fn_hex          = "nofile.hex";
            fn_img          = "D://work//raw_signal.img";
            adc_sample_rate_hz = 53.0e6;
            inter_freq_hz      = 1590.0e6;
            break;
        case ADC_AD9361:
            adc_type        = ADC_AD9361;
            drv_type        = DrvTypeCypress;
            chan_count      = 2;
            signal_type     = SigTypeIQParts;
            have_submodules = false;
            have_atts       = false;
            have_dbg        = true;
            fn_hex          = "";
            fn_img          = "AD9361BBCZ.img";
            adc_sample_rate_hz = 70.000e6;
            inter_freq_hz  = 0.0;
            break;
        }
    }
};

#endif // FX3CONFIG

