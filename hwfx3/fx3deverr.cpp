#include "fx3deverr.h"


const char* fx3_get_error_string(fx3_dev_err_t error) {
    switch ( error ) {
        case FX3_ERR_OK:                        return "FX3_ERR_OK";
        case FX3_ERR_DRV_NOT_IMPLEMENTED:       return "FX3_ERR_DRV_NOT_IMPLEMENTED";
        case FX3_ERR_USB_INIT_FAIL:             return "FX3_ERR_USB_INIT_FAIL";
        case FX3_ERR_NO_DEVICE_FOUND:           return "FX3_ERR_NO_DEVICE_FOUND";
        case FX3_ERR_BAD_DEVICE:                return "FX3_ERR_BAD_DEVICE";
        case FX3_ERR_FIRMWARE_FILE_IO_ERROR:    return "FX3_ERR_FIRMWARE_FILE_IO_ERROR";
        case FX3_ERR_FIRMWARE_FILE_CORRUPTED:   return "FX3_ERR_FIRMWARE_FILE_CORRUPTED";
        case FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR: return "FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR";
        case FX3_ERR_REG_WRITE_FAIL:            return "FX3_ERR_REG_WRITE_FAIL";
        case FX3_ERR_FW_TOO_MANY_ERRORS:        return "FX3_ERR_FW_TOO_MANY_ERRORS";
        case FX3_ERR_CTRL_TX_FAIL:              return "FX3_ERR_CTRL_TX_FAIL";
        default:                                return "FX3_ERR_UNKNOWN_ERROR";
    }

}

