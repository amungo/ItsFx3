#ifndef ITSMAIN_H
#define ITSMAIN_H

#include <QTimer>
#include <QMainWindow>
#include "util/AttMap.h"
#include "hwfx3/fx3devdebuginfo.h"
#include "hwfx3/fx3devdrvtype.h"
#include "hwfx3/fx3config.h"

namespace Ui {
class ItsMain;
}

class ItsMain : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ItsMain(FX3Config* cfg, QWidget *parent = 0);
    ~ItsMain();
    QWidget* rawForm;
    QWidget* gpsForm;
    QWidget* specForm;
    QWidget* phaseForm;
    QWidget* tuneForm;
private:
    Ui::ItsMain *ui;
    QTimer* timer;
    AttMap att;
    bool hw_inited;
    bool hw_started;
    FX3Config* cfg;
    
    void AddHWLog( const char* );
    void AddHWLog( QString );
    void SetHWButtonsGrey();
    void SetHWButtons();
    
private slots:
    void onButtonInitHw( bool );
    void onButtonCloseHw( bool );
    void onButtonStartHWStreams( bool );
    void onButtonStopHWStreams( bool );

    void onTimerGetDebugInfo();
    void onTimerChanged(int);
    
    void onAttChanged( int );

    void onRawForm(bool);
    void onGPSForm(bool);
    void onSpectrumForm(bool);
    void onPhaseForm(bool);
    void onTuneForm(bool);
public slots:
    void hardwareInitStatus( bool hw_init_status, QString log_message );
    void hardwareCloseStatus( bool hw_close_status, QString log_message );
    void hardwareStartStatus( bool hw_start_status, QString log_message );
    void hardwareStopStatus( bool hw_stop_status, QString log_message );
    void hardwareDebugInfo( bool, fx3_dev_debug_info_t info );

    
signals:
    void signalInitHw( DriverType_t, const char*, const char* );
    void signalCloseHw();
    void signalStartHwStreams();
    void signalStopHwStreams();
    void signalGetDebugInfo();
    
    void signalSetAttReg( uint32_t reg_val );
};

#endif // ITSMAIN_H
