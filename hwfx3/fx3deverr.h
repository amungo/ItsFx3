#ifndef FX3DEVERR
#define FX3DEVERR

enum fx3_dev_err_t {
    FX3_ERR_OK = 0,
    FX3_ERR_DRV_NOT_IMPLEMENTED       = -5,
    FX3_ERR_USB_INIT_FAIL             = -10,
    FX3_ERR_NO_DEVICE_FOUND           = -11,
    FX3_ERR_BAD_DEVICE                = -12,
    FX3_ERR_FIRMWARE_FILE_IO_ERROR    = -20,
    FX3_ERR_FIRMWARE_FILE_CORRUPTED   = -21,
    FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR = -25,
    FX3_ERR_REG_WRITE_FAIL            = -32,
    FX3_ERR_FW_TOO_MANY_ERRORS        = -33,
    FX3_ERR_CTRL_TX_FAIL              = -35
};
const char* fx3_get_error_string( fx3_dev_err_t error );

#endif // FX3DEVERR

