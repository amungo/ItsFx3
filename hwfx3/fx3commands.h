#ifndef FX3COMMANDS_H
#define FX3COMMANDS_H

enum fx3cmd {
    FW_LOAD         = 0xA0,
    GET_VERSION     = 0xB0,
    INIT_PROJECT    = 0xB1,
    REG_WRITE       = 0xB3,
    READ_DEBUG_INFO = 0xB4,
    REG_READ        = 0xB5,
    CYPRESS_RESET   = 0xBF,
    START           = 0xC0,
    WRITE_GPIO      = 0xC1,
    READ_GPIO       = 0xC2
};

enum fx3gpio {
    NT1065EN  = 17, /* CTL[0] */
    NT1065AOK = 52, /*        */
    VCTCXOEN  = 19, /* CTL[2] */
    ANTLNAEN  = 50, /*        */
    ANTFEEDEN = 18  /* CTL[1] */
};


#endif // FX3COMMANDS_H
