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

private:
    DeviceDataHandlerIfce *cb_handle;
    FILE* file;

    double SR;
    size_t block_pts;
    uint32_t sleep_ms;
    size_t current_offset8;
    size_t file_size8;
    int8_t*  buf_file;

    bool running;
    bool need_exit;
    std::thread thr;
    void run();

};

#endif // FILESIMDEV_H
