#ifndef FILESIMDEV_H
#define FILESIMDEV_H

#include "fx3devifce.h"
#include <thread>

class FileSimDev : public FX3DevIfce
{
public:
    FileSimDev( const char* sigfname, double real_sr, size_t block_pts );
    ~FileSimDev();

    fx3_dev_err_t init(const char *firmwareFileName, const char *additionalFirmwareFileName);
    void startRead(DeviceDataHandlerIfce *handler);
    void stopRead();
    void sendAttCommand5bits(uint32_t bits);
    fx3_dev_debug_info_t getDebugInfoFromBoard( bool ask_speed_only = false );

protected:
    virtual fx3_dev_err_t ctrlToDevice(   uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* data = nullptr, size_t data_len = 0 ) { return FX3_ERR_OK; }
    virtual fx3_dev_err_t ctrlFromDevice( uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* dest = nullptr, size_t data_len = 0 ) { return FX3_ERR_OK; }


private:
    DeviceDataHandlerIfce *cb_handle;
    FILE* file;

    double SR;
    int64_t block_pts;
    uint32_t sleep_ms;
    int64_t current_offset8;
    int64_t file_size8;
    int8_t*  buf_file;

    bool running;
    bool need_exit;
    std::thread thr;
    void run();

};

#endif // FILESIMDEV_H
