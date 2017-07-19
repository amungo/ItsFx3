#ifndef FX3DEVIFCE
#define FX3DEVIFCE

#include "fx3deverr.h"
#include "fx3devdebuginfo.h"
#include <stddef.h>

// Interface for handling data from FX3 device
class DeviceDataHandlerIfce {
public:
    virtual void HandleDeviceData( void* data_pointer, size_t size_in_bytes ) = 0;
};


class FX3DevIfce {
public:
    virtual ~FX3DevIfce(){}

    // Opens device and flash it if neccessary (set firmwareFileName to NULL to disable flashing)
    virtual fx3_dev_err_t init( const char* firmwareFileName,
                                const char* additionalFirmwareFileName ) = 0;

    // Starts reading of signal from device and sends data to handler.
    // If handler is NULL, data will be read and skipped
    virtual void startRead( DeviceDataHandlerIfce* handler ) = 0;

    // Stop's reading from device.
    virtual void stopRead() = 0;

    virtual void sendAttCommand5bits( uint32_t bits ) = 0;

    virtual fx3_dev_debug_info_t getDebugInfoFromBoard( bool ask_speed_only = false ) = 0;

protected:
    virtual fx3_dev_err_t ctrlToDevice(   uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* data = nullptr, size_t data_len = 0 ) = 0;
    virtual fx3_dev_err_t ctrlFromDevice( uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* dest = nullptr, size_t data_len = 0 ) = 0;

    virtual fx3_dev_err_t send16bitSPI(uint8_t data, uint8_t addr);
    virtual fx3_dev_err_t read16bitSPI(uint8_t addr, uint8_t *data);

    virtual void writeGPIO( uint32_t gpio, uint32_t value );
    virtual void readGPIO( uint32_t gpio, uint32_t* value );

    virtual fx3_dev_err_t resetFx3Chip();
    virtual void pre_init_fx3();
    virtual void init_ntlab_default();


    virtual uint32_t GetNt1065ChipID();
    virtual void readNtReg(uint32_t reg);

    virtual void startGpif();
};

#endif // FX3DEVIFCE

