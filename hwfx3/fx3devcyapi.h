#ifndef FX3DEVCYAPI_H
#define FX3DEVCYAPI_H

#include "fx3devifce.h"
#include "fx3deverr.h"
#include "fx3devdebuginfo.h"
#include <hwfx3/devioifce.h>

#ifndef NO_CY_API

#include <windows.h>
#include "cy_inc/CyAPI.h"

#include <cstdint>
#include <vector>
#include <thread>


#define MAX_QUEUE_SZ  64
#define VENDOR_ID    ( 0x04B4 )
#define PRODUCT_STREAM  ( 0x00F1 )
#define PRODUCT_BOOT    ( 0x00F3 )

typedef void (__stdcall * DataProcessorFunc)(char*, int);//data pointer, data size (bytes);

struct EndPointParams{
    int Attr;
    bool In;
    int MaxPktSize;
    int MaxBurst;
    int Interface;
    int Address;
};

struct StartDataTransferParams{
    CCyFX3Device	*USBDevice;
    CCyUSBEndPoint  *EndPt;
    int				PPX;
    int				QueueSize;
    int				TimeOut;
    bool			bHighSpeedDevice;
    bool			bSuperSpeedDevice;
    bool			bStreaming;
    bool			ThreadAlreadyStopped;
    DataProcessorFunc DataProc;
    EndPointParams  CurEndPoint;
};

#define NT1065EN		(17)		/* GPIO17, CTL[0] */
#define NT1065AOK		(52)		/* GPIO52 */
#define VCTCXOEN		(19)		/* GPIO19, CTL[2] */
#define ANTLNAEN		(50)		/* GPIO50 */
#define ANTFEEDEN		(18)		/* GPIO18, CTL[1] */

class FX3DevCyAPI : public FX3DevIfce, public DeviceControlIOIfce
{
public:
    FX3DevCyAPI();
    ~FX3DevCyAPI();

public:
    // FX3DevIfce interface
    fx3_dev_err_t init(const char *firmwareFileName, const char *additionalFirmwareFileName);
    void startRead(DeviceDataHandlerIfce *handler);
    void stopRead();
    void sendAttCommand5bits(uint32_t bits);
    fx3_dev_err_t resetFx3Chip();

    fx3_dev_debug_info_t getDebugInfoFromBoard( bool ask_speed_only = false );
    void fx3_development_call();
    uint32_t GetNt1065ChipID();
    void readNtReg(uint32_t reg);

    void pre_init();

    void writeGPIO( uint32_t gpio, uint32_t value );
    void readGPIO( uint32_t gpio, uint32_t* value );
    void startGpif();

private:


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

    fx3_dev_err_t scan( int& loadable_count, int& streamable_count );
    fx3_dev_err_t prepareEndPoints();
    void startTransferData( unsigned int EndPointInd, int PPX, int QueueSize, int TimeOut );
    void AbortXferLoop(StartDataTransferParams* Params, int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED *inOvLap);
    void GetEndPointParamsByInd(unsigned int EndPointInd, int* Attr, bool* In, int* MaxPktSize, int* MaxBurst, int* Interface, int* Address);
    fx3_dev_err_t loadAdditionalFirmware( const char* fw_name, uint32_t stop_addr );

    fx3_dev_err_t send16bitSPI(unsigned char data, unsigned char addr);
    fx3_dev_err_t read16bitSPI(unsigned char addr, unsigned char* data);

    fx3_dev_err_t send24bitSPI(unsigned char data, unsigned short addr);
    fx3_dev_err_t read24bitSPI(unsigned short addr, unsigned char* data);

private:
    DeviceDataHandlerIfce* data_handler;
    std::thread xfer_thread;
    void xfer_loop(void);

    StartDataTransferParams StartParams;
    std::vector<EndPointParams> EndPointsParams;
    
    uint32_t last_overflow_count;
    double size_tx_mb;
    

    // DeviceControlIOIfce interface
public:
    uint8_t peek8(uint32_t register_address24);
    void poke8(uint32_t register_address24, uint8_t value);

};


#else
class FX3DevCyAPI : public FX3DevIfce
{
public:
    FX3DevCyAPI(){}
    ~FX3DevCyAPI(){}

    // FX3DevIfce interface
    fx3_dev_err_t init(const char *firmwareFileName, const char *additionalFirmwareFileName){ return FX3_ERR_DRV_NOT_IMPLEMENTED; }
    void startRead(DeviceDataHandlerIfce *handler){}
    void stopRead(){}
    void sendAttCommand5bits(uint32_t bits){}
    fx3_dev_debug_info_t getDebugInfoFromBoard(bool ask_speed_only){ return fx3_dev_debug_info_t(); }
    void fx3_development_call(){}
};
#endif

#endif // FX3DEVCYAPI_H
