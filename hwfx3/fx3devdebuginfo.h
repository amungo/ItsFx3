#ifndef FX3DEVDEBUGINFO
#define FX3DEVDEBUGINFO

#include <cstdint>

#include "fx3deverr.h"

struct fx3_dev_debug_info_t {
    fx3_dev_err_t status;
    uint32_t transfers;
    uint32_t overflows;
    uint32_t overflows_inc;
    uint32_t phy_err_inc;
    uint32_t lnk_err_inc;
    uint32_t phy_errs;
    uint32_t lnk_errs;
    uint32_t err_reg_hex;
    double size_tx_mb_inc;
    bool speed_only;

    fx3_dev_debug_info_t() :
        status( FX3_ERR_OK ), transfers( 0 ), overflows( 0 ), overflows_inc( 0 ),
        phy_err_inc( 0 ), lnk_err_inc( 0 ), phy_errs( 0 ), lnk_errs( 0 ),
        err_reg_hex( 0 ), size_tx_mb_inc( 0.0 ), speed_only( true )
    {}
};


#endif // FX3DEVDEBUGINFO

