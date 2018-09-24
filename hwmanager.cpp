#include <thread>
#include <chrono>

#include "hwmanager.h"
#include "hwfx3/FX3Dev.h"
#include "hwfx3/fx3devcyapi.h"
#include "hwfx3/FileSimDev.h"
#ifdef WIN32
#include <windows.h>
#endif

HWManager::HWManager(FX3Config* cfg, QObject *parent) :
    QObject(parent),
    dev( NULL ),
    router( NULL ),
    cfg( cfg )
{
    
}

HWManager::~HWManager()
{
    if ( dev ) {
        FX3DevIfce* safe_dev = dev;
        dev = NULL;
        delete safe_dev;
    }
}

void HWManager::SetRouter(StreamRouter* r) {
    router = r;
}

void HWManager::initHardware(DriverType_t drvType, const char* imageFileName , const char* additionalImageFileName,
                             const char* algoFileName, const char* dataFileName)
{
    fprintf( stderr, "HWManager::initHardware( %d, %s, %s )\n", (int32_t)drvType, imageFileName, additionalImageFileName );
    if ( dev ) {
        delete dev;
        dev = NULL;
    }

    switch ( drvType ) {
        case DrvTypeLibUsb:
            dev = new FX3Dev();
            break;
        case DrvTypeCypress:
            dev = new FX3DevCyAPI();
            break;
        case DrvTypeFileSim:
            dev = new FileSimDev( imageFileName, cfg->adc_sample_rate_hz, 4 * 1024 * 1024 );
            break;
        default:
            QString msg( "Device init error " );
            msg += fx3_get_error_string( FX3_ERR_DRV_NOT_IMPLEMENTED );
            emit informInitHWStatus( false, msg );;
            emit newDevicePointer( nullptr );
            return;
    }

    fx3_dev_err_t init_error = dev->init( imageFileName, /*additionalImageFileName*/ NULL);
    if( init_error != FX3_ERR_OK ){
        qDebug( "Device init error %d %s", init_error, fx3_get_error_string( init_error ) );
        dev = NULL;
        
        QString msg( "Device init error " );
        msg += fx3_get_error_string( init_error );
        emit informInitHWStatus( false, msg );
        emit newDevicePointer( nullptr );
        
        closeHardware();
    } else {
        // If nut2nt load Lattice firmware
        if(drvType == DrvTypeCypress || drvType == DrvTypeLibUsb)
        {
            fprintf( stderr, "initing fpga...\n" );
            fx3_dev_err_t fpga_error = dev->init_fpga(algoFileName, dataFileName);
            if(fpga_error != FX3_ERR_OK) {
                fprintf( stderr, "fpga error %d\n", fpga_error );
                dev = 0;
                QString msg( "Device init error " );
                msg += fx3_get_error_string( init_error );
                emit informInitHWStatus( false, msg );
                emit newDevicePointer( nullptr );

                closeHardware();
                return;
            }
            fprintf( stderr, "fpga inited\n" );

            fx3_dev_err_t eres = dev->load1065Ctrlfile(additionalImageFileName, 49);
            if ( eres != FX3_ERR_OK ) {
                fprintf( stderr, "FX3Dev::Init() __error__ loadAdditionalFirmware %d %s\n", eres, fx3_get_error_string( eres ) );
            } else {
                fprintf( stderr, "hex loaded\n" );
            }
        }

        emit informInitHWStatus( true, QString("Device was inited!") );
        emit newDevicePointer( dev );
    }
}

void HWManager::closeHardware() {
    stopStreams();
    delete dev;
    dev = NULL;
    emit informCloseHWStatus( true, "Device was closed" );
}

void HWManager::startStreams() {
    if ( dev ) {
        dev->startRead( this );
    }
    emit informStartHWStatus( true, "Data stream was started" );
}

void HWManager::stopStreams() {
    if ( dev ) {
        dev->stopRead();
    }
    emit informStopHWStatus( true, "Data stream was stopped" );
}

void HWManager::setAttReg(uint32_t reg_val) {
    if ( dev ) {
        dev->sendAttCommand5bits( reg_val );
    }
}

void HWManager::getDebugInfo() {
    if ( dev ) {
        fx3_dev_debug_info_t info = dev->getDebugInfoFromBoard(
                    cfg->adc_type == ADC_1ch_16bit ? false : true
                    );
        if ( info.status == FX3_ERR_OK ) {
            emit informDebugInfo( true, info );
        } else {
            emit informDebugInfo( false, info );
        }
    }
}

void HWManager::HandleDeviceData(void* data_pointer, size_t size_in_bytes) {
    if ( router ) {
        router->HandleADCStreamData(data_pointer, size_in_bytes);
    }
}

