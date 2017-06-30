#ifndef SDR_FX3DEV_H
#define SDR_FX3DEV_H

extern "C" {
#include "libusb.h"
}

#include "fx3devifce.h"
#include "fx3deverr.h"
#include "fx3devdebuginfo.h"
#include "fx3fwparser.h"

#ifndef WIN32
#include <stdint.h>
#include <unistd.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <thread>
#include <mutex>
#include <list>
#include <vector>



/*
 * Find, opens and flash FX3 device.
 * 1. Create with new FX3Dev( size ) and set block size.
 * 2. Start device reading and set handler class.
 *    You will receive data continuously block by block with size you set in constructor,
 *    The handler's function HandleDeviceData will be called - you must copy data and return from function as fast as possible.
 *    If you have errors of skipping data you should do one or more of the following:
 *      a) increase block size (which is set in constructor)
 *      b) increase buffers_count (which is set in constructor)
 *      c) Just copy data and return in HandleDeviceData function! Do not do any signal processing there!
 * 
 * 3. Stop reading. (not tested)
 * 4. Destroy FX3Dev 
 */
class FX3Dev : public FX3DevIfce {
public:

    FX3Dev( size_t one_block_size8 = (3072 * 1250), uint32_t dev_buffers_count = 2 );
    virtual ~FX3Dev();

    // **** overrides FX3DevIfce
    fx3_dev_err_t init( const char* firmwareFileName = NULL, const char* additionalFirmwareFileName = NULL );
    void startRead( DeviceDataHandlerIfce* handler );
    void stopRead();
    void sendAttCommand5bits( uint32_t bits );
    fx3_dev_debug_info_t getDebugInfoFromBoard( bool ask_speed_only = false );
    // ****
    
private:    
    static const uint32_t VENDOR_ID = 0x04b4;
    static const uint32_t DEV_PID_NO_FW_NEEDED = 0x00f1; // PID of device with already flashed firmware
    static const uint32_t DEV_PID_FOR_FW_LOAD  = 0x00f3; // PID of device without flashed firmware. This device must be flashed before use.
    
    static const uint32_t MAX_UPLOAD_BLOCK_SIZE8 = 2048; // For firmware flashing.
    static const uint8_t  CMD_FW_LOAD   = 0xA0;          // Vendor command for firmware flashing.
    
    static const uint8_t  CMD_GET_VERSION     = ( 0xB0 );
    static const uint8_t  CMD_INIT_PROJECT    = ( 0xB1 );
    static const uint8_t  CMD_REG_WRITE       = ( 0xB3 );
    static const uint8_t  CMD_READ_DEBUG_INFO = ( 0xB4 );
    static const uint8_t  CMD_REG_READ        = ( 0xB5 );
    static const uint8_t  CMD_CYPRESS_RESET   = ( 0xBF );

    static const uint8_t  CMD_START       = ( 0xC0 );
    static const uint8_t  CMD_WRITE_GPIO  = ( 0xC1 );
    static const uint8_t  CMD_READ_GPIO   = ( 0xC2 );

    //static const int32_t  FX3_DEBUG_LEVEL_DEFAULT   = LIBUSB_LOG_LEVEL_NONE;
    //static const int32_t  FX3_DEBUG_LEVEL_DEFAULT   = LIBUSB_LOG_LEVEL_ERROR;
    static const int32_t  FX3_DEBUG_LEVEL_DEFAULT   = LIBUSB_LOG_LEVEL_WARNING;
    //static const int32_t  FX3_DEBUG_LEVEL_DEFAULT   = LIBUSB_LOG_LEVEL_INFO;
    //static const int32_t  FX3_DEBUG_LEVEL_DEFAULT   = LIBUSB_LOG_LEVEL_DEBUG;
    
    static const int32_t  PAUSE_AFTER_FLASH_SECONDS = 2;    // Seconds to wait after firmware is loaded.
    static const uint32_t DEV_UPLOAD_TIMEOUT_MS     = 1000; // Timeout for uploading one block of firmware. Block size is less or equal to MAX_UPLOAD_BLOCK_SIZE8 bytes.
    static const uint32_t DEV_DOWNLOAD_TIMEOUT_MS   =  500; // Timeout for downloading one block of data from device. One block size is half_full_size8 (set in constructor).
    
    static const uint32_t ADD_FW_LOAD_PAUSE_MS      = 20;   // Pause used for loading additional fw (in ms)
    static const uint32_t FW_LOAD_TRY_COUNT         = 10;   // Try count for loading firmware

    
private:
    fx3_dev_err_t scan();
    
    static void LIBUSB_CALL onDataReady(libusb_transfer* xfr);
    static void LIBUSB_CALL onWriteReady(libusb_transfer* xfr);
    fx3_dev_err_t firmwareFlashFromFile( const char* fw_fname );
    fx3_dev_err_t firmwareUploadToDevice( SectionList_t& sections );
    fx3_dev_err_t txFwToRAM( uint8_t* src, uint32_t dev_ram_addr, uint32_t size8 );
    fx3_dev_err_t txCheckFwFromRAM( uint8_t* dest, uint32_t dev_ram_addr, uint32_t size8 );
    fx3_dev_err_t txControlToDevice(     uint8_t* src, uint32_t size8, uint8_t cmd, uint16_t wValue, uint16_t wIndex );
    fx3_dev_err_t txControlFromDevice(  uint8_t* dest, uint32_t size8, uint8_t cmd, uint16_t wValue, uint16_t wIndex );
    
    fx3_dev_err_t loadAdditionalFirmware( const char* fw_name, uint32_t stop_addr );
    
    fx3_dev_err_t send16bitToDeviceSynch( uint8_t byte0 , uint8_t byte1 );
    fx3_dev_err_t sendDataToDeviceASynch(uint8_t* data, uint32_t size8 );
    
private:
    libusb_context *ctx = NULL;

    libusb_device_handle *device_handle;
    
    uint8_t endpoint_from_hst_num;
    uint8_t endpoint_from_dev_num;
    static const uint8_t endpoint_invalid = 255;

    std::thread event_thread;
    bool event_loop_running;
    void event_loop(void);
    
    uint32_t buffers_count; // Number of buffers which is used for continuous data transfer from device
    size_t half_full_size8;     // Size of one transfer block (data from device to host)
    std::vector< struct libusb_transfer* > transfers;
    std::vector< uint8_t* > half_full_buffers;
    
    struct libusb_transfer* write_transfer;
    uint8_t* write_buffer;

    DeviceDataHandlerIfce* data_handler;
    
    uint32_t last_overflow_count;
    double size_tx_mb;

    FX3DevFwParser fw_parser;
};


#endif //SDR_FX3DEV_H
