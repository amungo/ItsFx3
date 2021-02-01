#ifndef HWMANAGER_H
#define HWMANAGER_H

#include <QThread>
#include "hwfx3/fx3devdrvtype.h"
#include "hwfx3/fx3devifce.h"
#include "hwfx3/FX3Dev.h"
#include "datastreams/streamrouter.h"

class HWManager :
        public QObject,
        public DeviceDataHandlerIfce
{
    Q_OBJECT
public:
    explicit HWManager(FX3Config *cfg, QObject *parent = 0);
    ~HWManager();
    void SetRouter( StreamRouter* r );
    // DeviceDataHandlerIfce interface
    virtual void HandleDeviceData(void* data_pointer, size_t size_in_bytes);
private:
    FX3DevIfce* dev;
    StreamRouter* router;
    FX3Config* cfg;
signals:
    
public slots:
    void initHardware( DriverType_t drvType, const char* imageFileName, const char* additionalImageFileName,
                       const char* bitFileName);
    void closeHardware();
    void startStreams();
    void stopStreams();
    
    void setAttReg( uint32_t reg_val );
    void getDebugInfo();

signals:
    void newDevicePointer( FX3DevIfce* );
    void informInitHWStatus( bool, QString );
    void informCloseHWStatus( bool, QString );
    void informStartHWStatus( bool, QString );
    void informStopHWStatus( bool, QString );
    void informDebugInfo( bool, fx3_dev_debug_info_t );
    
};

#endif // HWMANAGER_H
