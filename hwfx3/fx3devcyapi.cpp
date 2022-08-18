#include "fx3commands.h"

#include <QDebug>
#include <QTextStream>
#include <QFile>

#include "fx3devcyapi.h"
#ifndef NO_CY_API

#include "HexParser.h"
#include <sstream>

FX3DevCyAPI::FX3DevCyAPI() :
    data_handler( NULL ),
    last_overflow_count( 0 ),
    size_tx_mb( 0.0 )
{

}

FX3DevCyAPI::~FX3DevCyAPI() {
    stopRead();

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    if ( resetECP5() == FX3_ERR_OK) {
        fprintf( stderr, "Going to switch_off lattice. Please wait\n" );
    }
    else {
        fprintf( stderr, "__error__ LATTICE CHIP SWITCH OFF failed\n" );
    }

    if ( resetFx3Chip() == FX3_ERR_OK ) {
        fprintf( stderr, "Fx3 is going to reset. Please wait\n" );
        for ( int i = 0; i < 20; i++ ) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            fprintf( stderr, "*" );
        }
        fprintf( stderr, " done\n" );
    } else {
        fprintf( stderr, "__error__ FX3 CHIP RESET failed\n" );
    }
}

fx3_dev_err_t FX3DevCyAPI::init(const char *firmwareFileName, const char *additionalFirmwareFileName) {
    StartParams.USBDevice = new CCyFX3Device();
    int boot = 0;
    int stream = 0;
    fx3_dev_err_t res = scan( boot, stream );
    if ( res != FX3_ERR_OK ) {
        return res;
    }

    bool need_fw_load = stream == 0 && boot > 0;

    if ( need_fw_load ) {
        fprintf( stderr, "FX3DevCyAPI::init() fw load needed\n" );

        FILE* f = fopen( firmwareFileName, "r" );
        if ( !f ) {
            return FX3_ERR_FIRMWARE_FILE_IO_ERROR;
        }

        if ( StartParams.USBDevice->IsBootLoaderRunning() ) {

            int retCode  = StartParams.USBDevice->DownloadFw((char*)firmwareFileName, FX3_FWDWNLOAD_MEDIA_TYPE::RAM);

            if ( retCode != FX3_FWDWNLOAD_ERROR_CODE::SUCCESS ) {
                switch( retCode ) {
                    case INVALID_FILE:
                    case CORRUPT_FIRMWARE_IMAGE_FILE:
                    case INCORRECT_IMAGE_LENGTH:
                    case INVALID_FWSIGNATURE:
                        return FX3_ERR_FIRMWARE_FILE_CORRUPTED;
                    default:
                        return FX3_ERR_CTRL_TX_FAIL;
                }
            } else {
                fprintf( stderr, "FX3DevCyAPI::init() boot ok!\n" );
            }
        } else {
            fprintf( stderr, "__error__ FX3DevCyAPI::init() StartParams.USBDevice->IsBootLoaderRunning() is FALSE\n" );
            return FX3_ERR_BAD_DEVICE;
        }
        last_overflow_count = 0;
    }

    if ( need_fw_load ) {
        int PAUSE_AFTER_FLASH_SECONDS = 2;
        fprintf( stderr, "FX3DevCyAPI::Init() flash completed!\nPlease wait for %d seconds\n", PAUSE_AFTER_FLASH_SECONDS );
        for ( int i = 0; i < PAUSE_AFTER_FLASH_SECONDS * 2; i++ ) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
            fprintf( stderr, "*" );
        }
        fprintf( stderr, "\n" );

        delete StartParams.USBDevice;
        StartParams.USBDevice = new CCyFX3Device();
        res = scan( boot, stream );
        if ( res != FX3_ERR_OK ) {
            return res;
        }
    }

    res = prepareEndPoints();
    if ( res != FX3_ERR_OK ) {
        return res;
    }

    if ( additionalFirmwareFileName != NULL ) {
        if ( additionalFirmwareFileName[ 0 ] != 0 ) {

            if ( std::string("manual") == std::string(additionalFirmwareFileName) ) {
                init_ntlab_default();
            } else {
                res = loadAdditionalFirmware( additionalFirmwareFileName, 48 );
                if ( res != FX3_ERR_OK ) {
                    fprintf( stderr, "FX3DevCyAPI::Init() __error__ loadAdditionalFirmware %d %s\n", res, fx3_get_error_string( res ) );
                    return res;
                }

            }
        }
    }

    bool In;
    int Attr, MaxPktSize, MaxBurst, Interface, Address;
    int EndPointsCount = EndPointsParams.size();
    for(int i = 0; i < EndPointsCount; i++){
        GetEndPointParamsByInd(i, &Attr, &In, &MaxPktSize, &MaxBurst, &Interface, &Address);
        fprintf( stderr, "FX3DevCyAPI::Init() EndPoint[%d], Attr=%d, In=%d, MaxPktSize=%d, MaxBurst=%d, Infce=%d, Addr=%d\n",
                 i, Attr, In, MaxPktSize, MaxBurst, Interface, Address);
    }

    return res;
}


fx3_dev_err_t FX3DevCyAPI::init_fpga(const char* bitFileName)
{
    fx3_dev_err_t retCode = FX3_ERR_OK;

    QFile fwFile(bitFileName);
    if(!fwFile.open(QIODevice::ReadOnly))
        return FX3_ERR_CTRL_TX_FAIL;

    QByteArray buff = fwFile.readAll();
    qint64 sz = buff.size();
    fwFile.close();

    int written = 0;
    int rstFPGA = resetECP5();
    if(rstFPGA == FX3_ERR_OK) {
        retCode = set_spi_clock(25000);
        if(retCode != FX3_ERR_OK) {
            fprintf( stderr, "FX3Dev::Init() set_spi_clock(25000 Khz) Ret code %d\n", retCode);
            return retCode;
        }

        long len = 512;
        char* tbuff = buff.data();
        for (; written < sz; tbuff += len) {
            if((written + len) > sz)
                len = sz - written;

            if(sendECP5((quint8*)tbuff, len) != FX3_ERR_OK)
               qDebug() << "--- Error cfgFPGA!! - --- \n" ;
            written += len;
        }
    }

    retCode = checkECP5();
    if(retCode == FX3_ERR_OK)
    {
        // Set DAC
        retCode = setDAC(0x000AFFFF<<4);
        fprintf( stderr, "setDAC %d\n", retCode );

        retCode = device_stop();
        fprintf( stderr, "device_stop %d\n", retCode );

        retCode = reset_nt1065();
        fprintf( stderr, "reset_nt1065 %d\n", retCode );

        GetNt1065ChipID();
        readFwVersion();
        readNtReg(0x07);
        if(fwDescription.version >= 0x17072800 ) {
            pre_init_fx3();
        }
    }

    return retCode;
}


void FX3DevCyAPI::startRead(DeviceDataHandlerIfce *handler) {
    size_tx_mb = 0.0;
    startTransferData(0, 128, 4, 1500);
    data_handler = handler;
    xfer_thread = std::thread(&FX3DevCyAPI::xfer_loop, this);
    //@camry startGpif();
    //device_stop();
    //device_reset();

    device_start();
}

void FX3DevCyAPI::stopRead() {
    if( StartParams.bStreaming ) {
        StartParams.bStreaming = false;
        xfer_thread.join();
        data_handler = NULL;
    }
    fprintf( stderr, "FX3DevCyAPI::stopRead() all done!\n" );
}

void FX3DevCyAPI::sendAttCommand5bits(uint32_t bits) {
    UCHAR buf[16];
    buf[0]=(UCHAR)(bits);

    fprintf( stderr, "FX3DevCyAPI::sendAttCommand5bits() %0x02X\n", bits );
     
    long len=16;
    int success=StartParams.USBDevice->BulkOutEndPt->XferData(&buf[0],len);
    
    if ( !success ) {
        fprintf( stderr, "__error__ FX3DevCyAPI::sendAttCommand5bits() BulkOutEndPt->XferData return FALSE\n" );
    }
    
}

fx3_dev_debug_info_t FX3DevCyAPI::getDebugInfoFromBoard(bool ask_speed_only) {
    if ( ask_speed_only ) {
        fx3_dev_debug_info_t info;
        info.status = FX3_ERR_OK;
        info.size_tx_mb_inc = size_tx_mb;
        size_tx_mb = 0.0;
        return info;
    } else {
        UCHAR buf[32];

        CCyControlEndPoint* CtrlEndPt;
        CtrlEndPt = StartParams.USBDevice->ControlEndPt;
        CtrlEndPt->Target    = TGT_DEVICE;
        CtrlEndPt->ReqType   = REQ_VENDOR;
        CtrlEndPt->Direction = DIR_FROM_DEVICE;
        CtrlEndPt->ReqCode = 0xB4;
        CtrlEndPt->Value = 0;
        CtrlEndPt->Index = 1;
        long len=32;
        int success = CtrlEndPt->XferData( &buf[0], len );
        unsigned int* uibuf = ( unsigned int* ) &buf[0];

        fx3_dev_debug_info_t info;
        if ( !success ) {
            fprintf( stderr, "FX3DevCyAPI::getDebugInfoFromBoard() __error__  CtrlEndPt->XferData is FALSE\n" );
            info.status = FX3_ERR_CTRL_TX_FAIL;
        } else {
            info.status = FX3_ERR_OK;
        }

        info.transfers   = uibuf[ 0 ];
        info.overflows   = uibuf[ 1 ];
        info.phy_err_inc = uibuf[ 2 ];
        info.lnk_err_inc = uibuf[ 3 ];
        info.err_reg_hex = uibuf[ 4 ];
        info.phy_errs    = uibuf[ 5 ];
        info.lnk_errs    = uibuf[ 6 ];
        info.fsm_state   = uibuf[ 7 ];

        info.size_tx_mb_inc = size_tx_mb;
        size_tx_mb = 0.0;

        info.overflows_inc = info.overflows - last_overflow_count;
        last_overflow_count = info.overflows;
        return info;
    }
}

fx3_dev_err_t FX3DevCyAPI::scan(int &loadable_count , int &streamable_count) {
    streamable_count = 0;
    loadable_count = 0;
    if (StartParams.USBDevice == NULL) {
        fprintf( stderr, "FX3DevCyAPI::scan() USBDevice == NULL" );
        return FX3_ERR_USB_INIT_FAIL;
    }
    unsigned short product = StartParams.USBDevice->ProductID;
    unsigned short vendor  = StartParams.USBDevice->VendorID;
    fprintf( stderr, "Device: 0x%04X 0x%04X ", vendor, product );
    if ( vendor == VENDOR_ID && product == PRODUCT_STREAM ) {
        fprintf( stderr, " STREAM\n" );
        streamable_count++;
    } else if ( vendor == VENDOR_ID && product == PRODUCT_BOOT ) {
        fprintf( stderr,  "BOOT\n" );
        loadable_count++;
    }
    fprintf( stderr, "\n" );

    if (loadable_count + streamable_count == 0) {
        return FX3_ERR_BAD_DEVICE;
    } else {
        return FX3_ERR_OK;
    }
}

fx3_dev_err_t FX3DevCyAPI::prepareEndPoints() {

    if ( ( StartParams.USBDevice->VendorID != VENDOR_ID) ||
         ( StartParams.USBDevice->ProductID != PRODUCT_STREAM ) )
    {
        return FX3_ERR_BAD_DEVICE;
    }

    int interfaces = StartParams.USBDevice->AltIntfcCount()+1;

    StartParams.bHighSpeedDevice = StartParams.USBDevice->bHighSpeed;
    StartParams.bSuperSpeedDevice = StartParams.USBDevice->bSuperSpeed;

    for (int i=0; i< interfaces; i++)
    {
        StartParams.USBDevice->SetAltIntfc(i);

        int eptCnt = StartParams.USBDevice->EndPointCount();

        // Fill the EndPointsBox
        for (int e=1; e<eptCnt; e++)
        {
            CCyUSBEndPoint *ept = StartParams.USBDevice->EndPoints[e];
            // INTR, BULK and ISO endpoints are supported.
            if ((ept->Attributes >= 1) && (ept->Attributes <= 3))
            {
                EndPointParams Params;
                Params.Attr = ept->Attributes;
                Params.In = ept->bIn;
                Params.MaxPktSize = ept->MaxPktSize;
                Params.MaxBurst = StartParams.USBDevice->BcdUSB == 0x0300 ? ept->ssmaxburst : 0;
                Params.Interface = i;
                Params.Address = ept->Address;

                EndPointsParams.push_back(Params);
            }
        }
    }

    return FX3_ERR_OK;
}

void FX3DevCyAPI::startTransferData(unsigned int EndPointInd, int PPX, int QueueSize, int TimeOut) {
    if(EndPointInd >= EndPointsParams.size())
        return;
    StartParams.CurEndPoint = EndPointsParams[EndPointInd];
    StartParams.PPX = PPX;
    StartParams.QueueSize = QueueSize;
    StartParams.TimeOut = TimeOut;
    StartParams.bStreaming = true;
    StartParams.ThreadAlreadyStopped = false;
    //StartParams.DataProc = DataProc;

    int alt = StartParams.CurEndPoint.Interface;
    int eptAddr = StartParams.CurEndPoint.Address;
    int clrAlt = (StartParams.USBDevice->AltIntfc() == 0) ? 1 : 0;
    if (! StartParams.USBDevice->SetAltIntfc(alt))
    {
        StartParams.USBDevice->SetAltIntfc(clrAlt); // Cleans-up
        return;
    }

    StartParams.EndPt = StartParams.USBDevice->EndPointOf((UCHAR)eptAddr);
}

void FX3DevCyAPI::AbortXferLoop(StartDataTransferParams *Params, int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED *inOvLap)
{
    //EndPt->Abort(); - This is disabled to make sure that while application is doing IO and user unplug the device, this function hang the app.
    long len = Params->EndPt->MaxPktSize * Params->PPX;

    for (int j=0; j< Params->QueueSize; j++)
    {
        if (j<pending)
        {
            if (!Params->EndPt->WaitForXfer(&inOvLap[j], Params->TimeOut))
            {
                Params->EndPt->Abort();
                if (Params->EndPt->LastError == ERROR_IO_PENDING)
                    WaitForSingleObject(inOvLap[j].hEvent,2000);
            }

            Params->EndPt->FinishDataXfer(buffers[j], len, &inOvLap[j], contexts[j]);
        }

        CloseHandle(inOvLap[j].hEvent);

        delete [] buffers[j];
        delete [] isoPktInfos[j];
    }

    delete [] buffers;
    delete [] isoPktInfos;
    delete [] contexts;

    Params->bStreaming = false;
    Params->ThreadAlreadyStopped = true;
}

void FX3DevCyAPI::GetEndPointParamsByInd(unsigned int EndPointInd, int *Attr, bool *In, int *MaxPktSize, int *MaxBurst, int *Interface, int *Address) {
    if(EndPointInd >= EndPointsParams.size())
        return;
    *Attr = EndPointsParams[EndPointInd].Attr;
    *In = EndPointsParams[EndPointInd].In;
    *MaxPktSize = EndPointsParams[EndPointInd].MaxPktSize;
    *MaxBurst = EndPointsParams[EndPointInd].MaxBurst;
    *Interface = EndPointsParams[EndPointInd].Interface;
    *Address = EndPointsParams[EndPointInd].Address;
}

fx3_dev_err_t FX3DevCyAPI::loadAdditionalFirmware(const char* fw_name, uint32_t stop_addr) {
    if ( !fw_name ) {
        return FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR;
    }
    
    FILE* f = fopen( fw_name, "r" );
    if ( !f ) {
        fprintf( stderr, "FX3Dev::loadAdditionalFirmware __error__ can't open '%s'\n", fw_name );
        return FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR;
    } else {
        fclose( f );
    }
    
    std::vector<uint32_t> addr;
    std::vector<uint32_t> data;
    
    parse_hex_file( fw_name, addr, data );
    
    for ( uint32_t i = 0; i < addr.size(); i++ ) {
        fx3_dev_err_t eres = send16bitSPI(data[i], addr[i]);
        if ( eres != FX3_ERR_OK ) {
            return eres;
        }
        
        std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );

        if ( addr[i] == stop_addr ) {
            break;
        }
    }
    return FX3_ERR_OK;
}

fx3_dev_err_t FX3DevCyAPI::send24bitSPI(unsigned char data, unsigned short addr)
{
    fprintf( stderr, "[0x%03X] <= 0x%02X\n", addr, data );

    UCHAR buf[16];
    addr |= 0x8000;
    buf[0] = (UCHAR)(data);
    buf[1] = (UCHAR)(addr&0x0FF);
    buf[2] = (UCHAR)(addr>>8);

    CCyControlEndPoint* CtrlEndPt;
    CtrlEndPt = StartParams.USBDevice->ControlEndPt;
    CtrlEndPt->Target = TGT_DEVICE;
    CtrlEndPt->ReqType = REQ_VENDOR;
    CtrlEndPt->Direction = DIR_TO_DEVICE;
    CtrlEndPt->ReqCode = 0xB6;
    CtrlEndPt->Value = 0;
    CtrlEndPt->Index = 1;
    long len = 16;
    int success = CtrlEndPt->XferData(&buf[0], len);

    if ( !success ) {
        fprintf( stderr, "__error__ FX3Dev::setDAC() FAILED (len = %d)\n", len );
    }

    return success ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;;
}

fx3_dev_err_t FX3DevCyAPI::read24bitSPI(unsigned short addr, unsigned char* data)
{
    UCHAR buf[16];
    //addr |= 0x8000;
    buf[0] = (UCHAR)(*data);
    buf[1] = (UCHAR)(addr&0x0FF);
    buf[2] = (UCHAR)(addr>>8);

    fprintf( stderr, "FX3Dev::read24bitSPI() from  0x%03X\n", addr );

    CCyControlEndPoint* CtrlEndPt;
    CtrlEndPt = StartParams.USBDevice->ControlEndPt;
    CtrlEndPt->Target = TGT_DEVICE;
    CtrlEndPt->ReqType = REQ_VENDOR;
    CtrlEndPt->Direction = DIR_FROM_DEVICE;
    CtrlEndPt->ReqCode = 0xB5;
    CtrlEndPt->Value = *data;
    CtrlEndPt->Index = addr;
    long len = 16;
    int success = CtrlEndPt->XferData(&buf[0], len);

    *data = buf[0];
    if ( success ) {
        fprintf( stderr, "[0x%03X] => 0x%02X\n", addr, *data);
    } else {
        fprintf( stderr, "__error__ FX3Dev::read24bitSPI() FAILED\n" );
    }
    return success ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

// ---------------------- Lattice control -------------------------
fx3_dev_err_t FX3DevCyAPI::send8bitSPI(uint8_t _addr, uint8_t _data)
{
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;
    buf[0] = (UCHAR)(_addr);
    buf[1] = (UCHAR)(_data);

    //qDebug() << "Reg" << _addr << " " << hex << _data;
    //fprintf( stderr, "Reg:%u 0x%04x \n", _addr, _data);

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(REG_WRITE8, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::read8bitSPI(uint8_t addr, uint8_t* data, uint8_t chip)
{
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;
    uint8_t addr_fix = (addr|0x80);

    //fprintf( stderr, "FX3Dev::read16bitSPI_ECP5() from  0x%03X\n", addr );

    WORD value = addr_fix;
    WORD index = chip;
    fx3_dev_err_t success = ctrlFromDevice(REG_READ8, value, index, buf, len);
    *data = buf[0];

    return success;

}

fx3_dev_err_t FX3DevCyAPI::writeADXL(uint8_t _addr, uint8_t _data)
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;

    buf[0] = (UCHAR)(_addr << 1);
    buf[1] = (UCHAR)_data;

    WORD value = 0;
    WORD index = 1;
    //fprintf( stderr, "Reg:%u 0x%04x \n", _addr, _data);

   return ctrlToDevice(ADXL_WRITE, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::readADXL(uint8_t _addr, uint8_t* _data)
{
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 1;
    uint8_t addr_fix = ((_addr << 1) | 0x01);

    // fprintf( stderr, "FX3DevCyAPI::read16bitSPI_ECP5() from  0x%03X\n", addr );

    uint16_t value = addr_fix;
    uint16_t index = *_data;
    fx3_dev_err_t success = ctrlFromDevice(ADXL_READ, value, index, buf, len);
    *_data = buf[0];

    return success;
}


fx3_dev_err_t FX3DevCyAPI::sendECP5(uint8_t* _data, long _data_len)
{
    UCHAR dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG  len = 16;
    UCHAR* buf = dummybuf;
    if(_data && _data_len != 0) {
        buf = (UCHAR*)_data;
        len = _data_len;
    }

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(ECP5_WRITE, value, index, buf, len);

}

fx3_dev_err_t FX3DevCyAPI::recvECP5(uint8_t* _data, long _data_len)
{
    UCHAR  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG   len = 16;
    UCHAR* buf = dummybuf;
    if(_data && _data_len != 0) {
        buf = (UCHAR*)_data;
        len = _data_len;
    }

    WORD value = 0;
    WORD index = 1;
    return ctrlFromDevice(ECP5_READ, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::resetECP5()
{
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    WORD value = 0;
    WORD index = 1;
    LONG len = 16;
    fx3_dev_err_t result = ctrlFromDevice(ECP5_RESET, value, index, buf, len);

    return (result == FX3_ERR_OK && buf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevCyAPI::switchoffECP5()
{
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    WORD value = 0;
    WORD index = 1;
    LONG len = 16;
    fx3_dev_err_t result = ctrlFromDevice(ECP5_OFF, value, index, buf, len);

    return (result == FX3_ERR_OK && buf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevCyAPI::checkECP5()
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;
    memset(&buf[0], 0xff, len);

    WORD value = 0;
    WORD index = 1;
    fx3_dev_err_t result = ctrlFromDevice(ECP5_CHECK, value, index, buf, len);

    return (result == FX3_ERR_OK && buf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevCyAPI::csonECP5()
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;
    memset(&buf[0], 0xff, len);

    WORD value = 0;
    WORD index = 1;
    fx3_dev_err_t result = ctrlToDevice(ECP5_CSON, value, index, buf, len);

    return (result == FX3_ERR_OK && buf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevCyAPI::csoffECP5()
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;
    memset(&buf[0], 0xff, len);

    WORD value = 0;
    WORD index = 1;
    fx3_dev_err_t result = ctrlToDevice(ECP5_CSOFF, value, index, buf, len);

    return (result == FX3_ERR_OK && buf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevCyAPI::setDAC(unsigned int data)
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 16;
    buf[0] = (UCHAR)(data>>16);
    buf[1] = (UCHAR)(data>>8);
    buf[2] = (UCHAR)(data);

    //qDebug() << "Reg" << hex << data;

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(ECP5_SET_DAC, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::device_start()
{
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    buf[2] = (UCHAR)(0xFF);
    LONG len = 16;

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(DEVICE_START, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::device_stop()
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    buf[2] = (UCHAR)(0xFF);
    LONG len = 16;

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(DEVICE_STOP, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::device_reset()
{   
    UCHAR buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    buf[2] = (UCHAR)(0xFF);
    LONG len = 16;

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(CYPRESS_RESET, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::reset_nt1065()
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    buf[2] = (UCHAR)(0xFF);
    LONG len = 16;

    WORD value = 0;
    WORD index = 1;
    return ctrlToDevice(NT1065_RESET, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::set_spi_clock(uint16_t _clock)
{
    UCHAR  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG len = 1;

    WORD value = _clock;
    WORD index = 1;

    return ctrlToDevice(SET_SPI_CLOCK, value, index, buf, len);
}

fx3_dev_err_t FX3DevCyAPI::load1065Ctrlfile(const char* fwFileName, int lastaddr)
{
    fx3_dev_err_t retCode = FX3_ERR_OK;
    CHAR line[128];

    FILE* pFile = fopen( fwFileName, "r" );
    if(!pFile) {
        return FX3_ERR_FIRMWARE_FILE_IO_ERROR;
    }

    while(fgets(line, 128, pFile) != NULL)
    {
        std::string sline(line);
        if(sline[0] != ';')
        {
            int regpos = sline.find("Reg");
            if(regpos != std::string::npos)
            {
                std::string buf;
                std::stringstream ss(sline); // Insert the string into a stream
                std::vector<std::string> tokens;
                while(ss >> buf)
                    tokens.push_back(buf);
                if(tokens.size() == 2) // addr & value
                {
                    int addr = std::stoi(tokens[0].erase(0,3), nullptr, 10);
                    int value = std::stoi(tokens.at(1), nullptr, 16);
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    retCode = send8bitSPI(addr, value);
                    if(retCode != FX3_ERR_OK || addr == lastaddr)
                        break;
                }
            }
        }
    }

    fclose(pFile);

    return retCode;
}

//-------------------------------------------------------------------------------------------------

fx3_dev_err_t FX3DevCyAPI::ctrlToDevice(uint8_t cmd, uint16_t value, uint16_t index, void *data, size_t data_len)
{
    UCHAR dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG  len = 16;
    UCHAR* buf = dummybuf;
    if ( data && data_len != 0 ) {
        buf = (UCHAR*)data;
        len = data_len;
    }
    CCyControlEndPoint* CtrlEndPt = StartParams.USBDevice->ControlEndPt;
    CtrlEndPt->Target    = TGT_DEVICE;
    CtrlEndPt->ReqType   = REQ_VENDOR;
    CtrlEndPt->Direction = DIR_TO_DEVICE;
    CtrlEndPt->ReqCode   = cmd;
    CtrlEndPt->Value     = value;
    CtrlEndPt->Index     = index;
    int success = CtrlEndPt->XferData(buf, len);

    return success ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3DevCyAPI::ctrlFromDevice(uint8_t cmd, uint16_t value, uint16_t index, void *dest, size_t data_len)
{
    UCHAR  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    LONG   len = 16;
    UCHAR* buf = dummybuf;
    if ( dest && data_len != 0 ) {
        buf = (UCHAR*)dest;
        len = data_len;
    }

    CCyControlEndPoint* CtrlEndPt = StartParams.USBDevice->ControlEndPt;
    CtrlEndPt->Target    = TGT_DEVICE;
    CtrlEndPt->ReqType   = REQ_VENDOR;
    CtrlEndPt->Direction = DIR_FROM_DEVICE;
    CtrlEndPt->ReqCode   = cmd;
    CtrlEndPt->Value     = value;
    CtrlEndPt->Index     = index;

    int success = CtrlEndPt->XferData(buf, len);

    return success ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

void FX3DevCyAPI::xfer_loop() {
    fprintf( stderr, "FX3DevCyAPI::xfer_loop() started\n" );
    StartDataTransferParams* Params = &StartParams;
    
    
    if(Params->EndPt->MaxPktSize==0)
        return;
    
    // Limit total transfer length to 4MByte
    long len = ((Params->EndPt->MaxPktSize) * Params->PPX);
    
    int maxLen = 0x400000;  //4MByte
    if (len > maxLen){
        Params->PPX = maxLen / (Params->EndPt->MaxPktSize);
        if((Params->PPX%8)!=0)
            Params->PPX -= (Params->PPX%8);
    }
    
    if ((Params->bSuperSpeedDevice || Params->bHighSpeedDevice) && (Params->EndPt->Attributes == 1)){  // HS/SS ISOC Xfers must use PPX >= 8
        Params->PPX = max(Params->PPX, 8);
        Params->PPX = (Params->PPX / 8) * 8;
        if(Params->bHighSpeedDevice)
            Params->PPX = max(Params->PPX, 128);
    }

    long BytesXferred = 0;
    unsigned long Successes = 0;
    unsigned long Failures = 0;
    int i = 0;

    // Allocate the arrays needed for queueing
    PUCHAR			*buffers		= new PUCHAR[Params->QueueSize];
    CCyIsoPktInfo	**isoPktInfos	= new CCyIsoPktInfo*[Params->QueueSize];
    PUCHAR			*contexts		= new PUCHAR[Params->QueueSize];
    OVERLAPPED		inOvLap[MAX_QUEUE_SZ];

    Params->EndPt->SetXferSize(len);

    // Allocate all the buffers for the queues
    for (i=0; i< Params->QueueSize; i++)
    {
        buffers[i]        = new UCHAR[len];
        isoPktInfos[i]    = new CCyIsoPktInfo[Params->PPX];
        inOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);

        memset(buffers[i],0xEF,len);
    }

    // Queue-up the first batch of transfer requests
    for (i=0; i< Params->QueueSize; i++)
    {
        contexts[i] = Params->EndPt->BeginDataXfer(buffers[i], len, &inOvLap[i]);
        if (Params->EndPt->NtStatus || Params->EndPt->UsbdStatus) // BeginDataXfer failed
        {
            AbortXferLoop(Params, i+1, buffers,isoPktInfos,contexts,inOvLap);
            return;
        }
    }

    i=0;



    // The infinite xfer loop.
    for (;Params->bStreaming;)
    {
        long rLen = len;	// Reset this each time through because
        // FinishDataXfer may modify it

        if (!Params->EndPt->WaitForXfer(&inOvLap[i], Params->TimeOut))
        {
            Params->EndPt->Abort();
            if (Params->EndPt->LastError == ERROR_IO_PENDING)
                WaitForSingleObject(inOvLap[i].hEvent,2000);
        }

        if (Params->EndPt->Attributes == 1) // ISOC Endpoint
        {
            if (Params->EndPt->FinishDataXfer(buffers[i], rLen, &inOvLap[i], contexts[i], isoPktInfos[i]))
            {
                CCyIsoPktInfo *pkts = isoPktInfos[i];
                for (int j=0; j< Params->PPX; j++)
                {
                    if ((pkts[j].Status == 0) && (pkts[j].Length<=Params->EndPt->MaxPktSize))
                    {
                        BytesXferred += pkts[j].Length;
                        Successes++;
                    }
                    else
                        Failures++;
                    pkts[j].Length = 0;	// Reset to zero for re-use.
                    pkts[j].Status = 0;
                }
            }
            else
                Failures++;
        }
        else // BULK Endpoint
        {
            if (Params->EndPt->FinishDataXfer(buffers[i], rLen, &inOvLap[i], contexts[i]))
            {
                Successes++;
                BytesXferred += len;
            }
            else
                Failures++;
        }

        BytesXferred = max(BytesXferred, 0);

        size_tx_mb += ( ( double ) len ) / ( 1024.0 * 1024.0 );
        if ( data_handler ) {
            data_handler->HandleDeviceData(buffers[i], len);
        }

        // Re-submit this queue element to keep the queue full
        contexts[i] = Params->EndPt->BeginDataXfer(buffers[i], len, &inOvLap[i]);
        if (Params->EndPt->NtStatus || Params->EndPt->UsbdStatus) // BeginDataXfer failed
        {
            AbortXferLoop(Params, Params->QueueSize,buffers,isoPktInfos,contexts,inOvLap);
            return;
        }

        i = (i + 1) % Params->QueueSize;
    }  // End of the infinite loop

    AbortXferLoop( Params, Params->QueueSize, buffers, isoPktInfos, contexts, inOvLap );

    fprintf( stderr, "FX3DevCyAPI::xfer_loop() finished\n" );
}

uint8_t FX3DevCyAPI::peek8(uint32_t register_address24) {
    uint8_t val = 0;
    if ( read24bitSPI( register_address24, &val ) == FX3_ERR_OK ) {
        return val;
    } else {
        return 0xFF;
    }
}

void FX3DevCyAPI::poke8(uint32_t register_address24, uint8_t value) {
    send24bitSPI( value, register_address24 );
}

#endif

