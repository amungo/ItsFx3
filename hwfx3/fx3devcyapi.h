#ifndef FX3DEVCYAPI_H
#define FX3DEVCYAPI_H

#include "fx3devifce.h"
#include "fx3deverr.h"
#include "fx3devdebuginfo.h"
#include <hwfx3/devioifce.h>

#ifndef NO_CY_API

#include <windows.h>
#include "cy_inc/CyAPI.h"

#include <memory>
#include <cstdint>
#include <vector>
#include <thread>


#define MAX_QUEUE_SZ  64
#define VENDOR_ID    ( 0x04B4 )
#define PRODUCT_STREAM  ( 0x00F1 )
#define PRODUCT_BOOT    ( 0x00F0 ) //( 0x00F3 ) //@camry -- Удалить !!!!! ( 0x00F0 ) пока ипользуем то что прошито.

typedef void (__stdcall * DataProcessorFunc)(char*, int);//data pointer, data size (bytes);

class SSPICore;

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


class FX3DevCyAPI : public FX3DevIfce, public DeviceControlIOIfce
{
public:
    FX3DevCyAPI();
    ~FX3DevCyAPI();

public:
    // FX3DevIfce interface
    virtual fx3_dev_err_t init(const char *firmwareFileName, const char *additionalFirmwareFileName);
    virtual fx3_dev_err_t init_fpga(const char* algoFileName, const char* dataFileName);

    void startRead(DeviceDataHandlerIfce *handler);
    void stopRead();
    void sendAttCommand5bits(uint32_t bits);
    fx3_dev_debug_info_t getDebugInfoFromBoard( bool ask_speed_only = false );
private:
    fx3_dev_err_t scan( int& loadable_count, int& streamable_count );
    fx3_dev_err_t prepareEndPoints();
    void startTransferData( unsigned int EndPointInd, int PPX, int QueueSize, int TimeOut );
    void AbortXferLoop(StartDataTransferParams* Params, int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED *inOvLap);
    void GetEndPointParamsByInd(unsigned int EndPointInd, int* Attr, bool* In, int* MaxPktSize, int* MaxBurst, int* Interface, int* Address);
    fx3_dev_err_t loadAdditionalFirmware( const char* fw_name, uint32_t stop_addr );

    fx3_dev_err_t send24bitSPI(unsigned char data, unsigned short addr);
    fx3_dev_err_t read24bitSPI(unsigned short addr, unsigned char* data);

    //------------------  Lattice (FX3DevIfce interFACE)  ------------------
    virtual fx3_dev_err_t send8bitSPI(uint8_t data, uint8_t addr);
    virtual fx3_dev_err_t read8bitSPI(uint8_t addr, uint8_t* data);
    virtual fx3_dev_err_t sendECP5(uint8_t* _data, long _data_len);
    virtual fx3_dev_err_t recvECP5(uint8_t* buf, long len);
    virtual fx3_dev_err_t resetECP5();
    virtual fx3_dev_err_t checkECP5();
    virtual fx3_dev_err_t csonECP5();
    virtual fx3_dev_err_t csoffECP5();
    virtual fx3_dev_err_t setDAC(unsigned int data);
    virtual fx3_dev_err_t device_start();
    virtual fx3_dev_err_t device_stop();
    virtual fx3_dev_err_t device_reset();
    virtual fx3_dev_err_t reset_nt1065();
    virtual fx3_dev_err_t load1065Ctrlfile(const char* fwFileName, int lastaddr);

protected:
    fx3_dev_err_t ctrlToDevice(   uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* data = nullptr, size_t data_len = 0 );
    fx3_dev_err_t ctrlFromDevice( uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* dest = nullptr, size_t data_len = 0 );

private:
    DeviceDataHandlerIfce* data_handler;
    std::thread xfer_thread;
    void xfer_loop(void);

    StartDataTransferParams StartParams;
    std::vector<EndPointParams> EndPointsParams;
    
    uint32_t last_overflow_count;
    double size_tx_mb;

    std::shared_ptr<SSPICore> m_SSPICore;
    

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
    virtual fx3_dev_err_t ctrlToDevice(   uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* data = nullptr, size_t data_len = 0 ) {}
    virtual fx3_dev_err_t ctrlFromDevice( uint8_t cmd, uint16_t value = 0, uint16_t index = 0, void* dest = nullptr, size_t data_len = 0 ) {}

};
#endif

#endif // FX3DEVCYAPI_H
