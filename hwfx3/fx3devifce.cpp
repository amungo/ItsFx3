#include <cstdio>
#include <thread>
#include <chrono>
#include "fx3commands.h"
#include "fx3devifce.h"


fx3_dev_err_t FX3DevIfce::resetFx3Chip() {
    return ctrlToDevice( fx3cmd::CYPRESS_RESET );
}

void FX3DevIfce::pre_init_fx3() {
    writeGPIO(NT1065EN,  0);
    writeGPIO(NT1065AOK, 0);
    writeGPIO(VCTCXOEN,  0);
    writeGPIO(ANTLNAEN,  0);
    writeGPIO(ANTFEEDEN, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writeGPIO(VCTCXOEN,  1);
    writeGPIO(NT1065EN,  1);

    writeGPIO(ANTLNAEN,  1);
    writeGPIO(ANTFEEDEN, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void FX3DevIfce::init_ntlab_default() {
    unsigned char reg = 0;

    //  SetSingleLO
    send16bitSPI(0x00, 3);
    send16bitSPI(0x00, 45);

    // SetPLLTune
    int npll = 0;
    read16bitSPI(43+npll*4, &reg);
    send16bitSPI(reg|1, 43+npll*4);


    //PLLstat = NT1065_GetPLLStat(0);

    //AOK
    readNtReg(0x07);

    // NT1065_SetSideBand( chan, side )
    int chan = 1;
    int side = 1;
    read16bitSPI(13+chan*7, &reg);
    send16bitSPI((reg&0xFD)|((side&1)<<1), 13+chan*7);

    chan = 2;
    side = 1;
    read16bitSPI(13+chan*7, &reg);
    send16bitSPI((reg&0xFD)|((side&1)<<1), 13+chan*7);

    chan = 3;
    side = 1;
    read16bitSPI(13+chan*7, &reg);
    send16bitSPI((reg&0xFD)|((side&1)<<1), 13+chan*7);

    //AOK
    readNtReg(0x07);

    // SetOutAnalog_IFMGC
    send16bitSPI(0x22, 15);
    send16bitSPI(0x22, 22);
    send16bitSPI(0x22, 29);
    send16bitSPI(0x22, 36);

//    // setRFGain
//    chan = 0;
//    int gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    chan = 1;
//    gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    chan = 2;
//    gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    chan = 3;
//    gain = 11;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0x3)|(((gain-11)&0x0F)<<4), 17+chan*7);

//    // setIfGain
//    chan = 0;
//    int gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);

//    chan = 1;
//    gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);

//    chan = 2;
//    gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);

//    chan = 3;
//    gain_code = 10;
//    read16bitSPI(17+chan*7, &reg);
//    send16bitSPI((reg&0xFC)|(((gain_code>>3)&0x03)), 17+chan*7);
//    send16bitSPI( (gain_code&0x07)<<5, 18+chan*7);


    // SetLPFCalStart
    send16bitSPI(1, 4);

    // SetOutADC_IFMGC
    send16bitSPI(0x23, 15);
    send16bitSPI(0x0B, 19);
    send16bitSPI(0x23, 22);
    send16bitSPI(0x0B, 26);
    send16bitSPI(0x23, 29);
    send16bitSPI(0x0B, 33);
    send16bitSPI(0x23, 36);
    send16bitSPI(0x0B, 40);
}

uint32_t FX3DevIfce::GetNt1065ChipID() {
    unsigned char reg0 = 0;
    read16bitSPI(0x00, &reg0);

    unsigned char reg1 = 0;
    read16bitSPI(0x01, &reg1);

    uint32_t id = (unsigned int)reg0<<21 | ((unsigned int)reg1&0xF8)<<13 | reg1&0x07;

    fprintf( stderr, "ChipID: %08X\n", id );
    return id;
}

void print_bits( uint32_t val, int bits_count = 8 ) {
    for ( int i = bits_count - 1; i >=0; i-- ) {
        if ( val & ( 1 << i ) ) {
            fprintf( stderr, "  1" );
        } else {
            fprintf( stderr, "  0" );
        }
    }
    fprintf( stderr, "\n" );

    for ( int i = bits_count - 1; i >=0; i-- ) {
        fprintf( stderr, "%3d", i );
    }
    fprintf( stderr, "\n" );
}

void FX3DevIfce::readNtReg(uint32_t reg) {
    fx3_dev_err_t res = FX3_ERR_OK;
    unsigned char val = 0x00;
    res = read16bitSPI(reg, &val);
    fprintf( stderr, "Reg%d (0x%02X), val = 0x%08X\n", reg, reg, val );
    print_bits(val);
}

fx3_dev_err_t FX3DevIfce::send16bitSPI(uint8_t data, uint8_t addr)
{
    uint8_t buf[16];
    buf[0] = data;
    buf[1] = addr;

    //fprintf( stderr, "send16bitToDevice( 0x%02X, 0x%02X )\n", data, addr );
    fx3_dev_err_t res = ctrlToDevice( fx3cmd::REG_WRITE, 0, 1, buf, 16 );

    if ( res == FX3_ERR_OK ) {
        return FX3_ERR_OK;
    } else {
        return FX3_ERR_CTRL_TX_FAIL;
    }
}

fx3_dev_err_t FX3DevIfce::read16bitSPI(uint8_t addr, uint8_t *data)
{
    uint8_t addr_fix = (addr|0x80);
    uint8_t buf[16];
    buf[0] = *data;
    buf[1] = addr_fix;
    fx3_dev_err_t res = ctrlFromDevice( fx3cmd::REG_READ, *data, addr_fix, buf, 16 );

    *data = buf[0];
    if ( res == FX3_ERR_OK ) {
        fprintf( stderr, "[0x%02X] is 0x%02X\n", addr, *data );
        return FX3_ERR_OK;
    } else {
        fprintf( stderr, "__error__ FX3Dev::read16bitSPI() FAILED\n" );
        return FX3_ERR_CTRL_TX_FAIL;
    }
}

void FX3DevIfce::writeGPIO(uint32_t gpio, uint32_t value) {
    uint32_t ans[4];
    fx3_dev_err_t res = ctrlFromDevice( fx3cmd::WRITE_GPIO, value, gpio, ans, 16 );

    if ( res == FX3_ERR_OK ) {
        if ( ans[0] != 0 ) {
            fprintf(stderr, "writeGPIO( %d, %d ) ERROR CyU3PGpioSetValue returned 0x%02X\n", gpio, value, ans[0] );
        } else {
            fprintf( stderr, "writeGPIO( %d, %d ) - ok\n", gpio, value );
        }
    } else {
        fprintf(stderr, "writeGPIO( %d, %d ) FAILED\n", gpio, value );
    }
}

void FX3DevIfce::readGPIO(uint32_t gpio, uint32_t *value) {
    uint32_t ans[4];
    fx3_dev_err_t res = ctrlFromDevice( fx3cmd::READ_GPIO, 0, gpio, ans, 16 );

    if ( res == FX3_ERR_OK ) {
        if ( ans[0] != 0 ) {
            fprintf(stderr, "readGPIO( %d ) ERROR CyU3PGpioGetValue returned 0x%02X\n", gpio, ans[0] );
        } else {
            fprintf(stderr, "readGPIO( %d ) have %d value \n", gpio, ans[1] );
            *value = ans[1];
        }
    } else {
        fprintf(stderr, "readGPIO( %d ) FAILED\n", gpio );
    }
}

void FX3DevIfce::startGpif() {
    fprintf( stderr, "startGpif()\n" );
    ctrlFromDevice( fx3cmd::START );
}
