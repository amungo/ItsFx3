#include "itsmain.h"
#include "hwmanager.h"
#include "rawsignalform.h"
#include "dialogconfig.h"
#include "hwfx3/fx3config.h"
#include "datastreams/streamrouter.h"
#include "gpscorrform.h"
#include "SpectrumForm.h"
#include "phaseform.h"
#include "tuneform.h"

#include "stdio.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    QApplication a(argc, argv);

    FX3Config cfg;
    DialogConfig dcfg( &cfg );
    dcfg.setModal(true);
    int cfg_result = dcfg.exec();
    if ( !cfg_result ) {
        return -1;
    }



    StreamRouter router( cfg.adc_type );
    router.SetHackedLen( cfg.hacked_len );
        
    RawSignalForm rawForm( &cfg );
    rawForm.router = &router;

    GPSCorrForm gpsForm( &cfg );
    gpsForm.router = &router;

    SpectrumForm specForm( &cfg );
    specForm.router = &router;

    PhaseForm phaseForm;
    phaseForm.router = &router;

    TuneForm tuneForm;

    ItsMain mainWindow( &cfg );
    mainWindow.rawForm = &rawForm;
    mainWindow.gpsForm = &gpsForm;
    mainWindow.specForm = &specForm;
    mainWindow.phaseForm = &phaseForm;
    mainWindow.tuneForm  = &tuneForm;

    HWManager hwm(&cfg);
    hwm.SetRouter(&router);


    router.AddOutPoint( &gpsForm );

    QObject::connect(&mainWindow, SIGNAL(signalInitHw(DriverType_t, const char*, const char*, const char* )), &hwm, SLOT(initHardware(DriverType_t, const char*, const char*, const char*)));
    QObject::connect(&mainWindow, SIGNAL(signalCloseHw()),           &hwm, SLOT(closeHardware()) );
    QObject::connect(&mainWindow, SIGNAL(signalStartHwStreams()),    &hwm, SLOT(startStreams()) );
    QObject::connect(&mainWindow, SIGNAL(signalStopHwStreams()),     &hwm, SLOT(stopStreams()) );
    QObject::connect(&mainWindow, SIGNAL(signalSetAttReg(uint32_t)), &hwm, SLOT(setAttReg(uint32_t)) );
    QObject::connect(&mainWindow, SIGNAL(signalGetDebugInfo()),      &hwm, SLOT(getDebugInfo()) );

    QObject::connect(&hwm, SIGNAL(informInitHWStatus(bool, QString)), &mainWindow, SLOT(hardwareInitStatus(bool, QString)) );
    QObject::connect(&hwm, SIGNAL(informCloseHWStatus(bool,QString)), &mainWindow, SLOT(hardwareCloseStatus(bool,QString)) );
    QObject::connect(&hwm, SIGNAL(informStartHWStatus(bool,QString)), &mainWindow, SLOT(hardwareStartStatus(bool,QString)) );
    QObject::connect(&hwm, SIGNAL(informStopHWStatus(bool,QString)),  &mainWindow, SLOT(hardwareStopStatus(bool,QString)) );
    QObject::connect(&hwm, SIGNAL(informDebugInfo(bool,fx3_dev_debug_info_t)),  &mainWindow, SLOT(hardwareDebugInfo(bool,fx3_dev_debug_info_t)) );    
    QObject::connect(&hwm, SIGNAL(newDevicePointer(FX3DevIfce*)), &tuneForm, SLOT(setDevicePointer(FX3DevIfce*)) );

    QObject::connect(&tuneForm, SIGNAL(newFreq(double)), &phaseForm, SLOT(ChangeNullMhz(double)));
    QObject::connect(&tuneForm, SIGNAL(newFreq(double)), &specForm,  SLOT(ChangeNullMhz(double)));

    mainWindow.show();
    



    return a.exec();
}
