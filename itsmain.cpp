#include "itsmain.h"
#include "ui_itsmain.h"

ItsMain::ItsMain(FX3Config* cfg, QWidget *parent) :
    QMainWindow(parent),
    rawForm( NULL ),
    gpsForm( NULL ),
    specForm( NULL ),
    ui(new Ui::ItsMain),
    timer( NULL ),
    hw_inited( false ),
    hw_started( false ),
    cfg( cfg )
{
    ui->setupUi(this);
    SetHWButtonsGrey();
    ui->buttonInitHardware->setEnabled(  true  );
    
    timer = new QTimer( this );
    
    QObject::connect(ui->buttonInitHardware,  SIGNAL(clicked(bool)), this, SLOT(onButtonInitHw(bool)         ) );
    QObject::connect(ui->buttonCloseHardware, SIGNAL(clicked(bool)), this, SLOT(onButtonCloseHw(bool)        ) );
    QObject::connect(ui->buttonStartStreams,  SIGNAL(clicked(bool)), this, SLOT(onButtonStartHWStreams(bool) ) );
    QObject::connect(ui->buttonStopStreams,   SIGNAL(clicked(bool)), this, SLOT(onButtonStopHWStreams(bool)  ) );
    QObject::connect(ui->comboBoxDbgTimer, SIGNAL(currentIndexChanged(int)), this, SLOT(onTimerChanged(int)) );
    
    QObject::connect(timer,                       SIGNAL(timeout()), this, SLOT(onTimerGetDebugInfo()        ) );
    
    QObject::connect(ui->comboBoxAtts, SIGNAL(currentIndexChanged(int)), this, SLOT(onAttChanged(int)) );
    
    QObject::connect(ui->buttonRawForm, SIGNAL(clicked(bool)), this, SLOT(onRawForm(bool)));
    QObject::connect(ui->buttonGPSForm, SIGNAL(clicked(bool)), this, SLOT(onGPSForm(bool)));
    QObject::connect(ui->buttonSpectrumForm, SIGNAL(clicked(bool)), this, SLOT(onSpectrumForm(bool)));
    QObject::connect(ui->buttonDeveloperCall, SIGNAL(clicked(bool)), this, SLOT(onButtonCallDeveloperFun(bool)));
    
    att.Clear();
    att.AddBit(0, 4);
    att.AddBit(1, 8);
    att.AddBit(2, 16);
    att.AddBit(3, 32);
    att.Init();
    std::vector<uint32_t> attvec = att.GetAttVector();
    for ( uint32_t i = 0; i < attvec.size(); i++ ) {
        ui->comboBoxAtts->insertItem(i, QString::number(attvec[i]), QVariant(attvec[i]) );
    }
    
    int dtm = 0;
    ui->comboBoxDbgTimer->insertItem( dtm++, "No poll", QVariant(0) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "10 sec", QVariant(10000) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "5 sec",  QVariant( 5000) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "2 sec",  QVariant( 2000) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "1 sec",  QVariant( 1000) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "500 ms", QVariant(  500) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "200 ms", QVariant(  200) );
    ui->comboBoxDbgTimer->insertItem( dtm++, "100 ms", QVariant(  100) );
    ui->comboBoxDbgTimer->setCurrentIndex(3);

#ifdef NO_GPS
    ui->buttonGPSForm->setEnabled( false );
    ui->buttonGPSForm->setVisible( false );
#endif
    if ( cfg->adc_type == ADC_AD9361 ) {
        ui->buttonDeveloperCall->setVisible( true );
    } else {
        ui->buttonDeveloperCall->setEnabled( false );
        ui->buttonDeveloperCall->setVisible( false );
    }

    if ( !cfg->have_atts ) {
        ui->labelAtt->setVisible(false);
        ui->comboBoxAtts->setVisible(false);
    }
}

ItsMain::~ItsMain()
{
    qDebug( "ItsMain::~ItsMain()" );
    delete ui;
    delete timer;
}

void ItsMain::AddHWLog(const char* str) {
    AddHWLog( QString(str) );
}

void ItsMain::AddHWLog(QString str) {
    ui->plainTextHWLog->insertPlainText( str );
    ui->plainTextHWLog->insertPlainText( "\n" );
    QTextCursor c = ui->plainTextHWLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->plainTextHWLog->setTextCursor( c );
}

void ItsMain::SetHWButtonsGrey() {
    ui->buttonInitHardware->setEnabled(  false );
    ui->buttonCloseHardware->setEnabled( false );
    ui->buttonStartStreams->setEnabled(  false );
    ui->buttonStopStreams->setEnabled(   false );
    ui->comboBoxAtts->setEnabled( false );
}

void ItsMain::SetHWButtons() {
    if ( hw_inited ) {
        ui->buttonInitHardware->setEnabled(  false );
        ui->buttonCloseHardware->setEnabled( true );
        if ( hw_started ) {
            ui->buttonStartStreams->setEnabled( false );
            ui->buttonStopStreams->setEnabled( true );
            ui->comboBoxAtts->setEnabled( cfg->have_atts );
        } else {
            ui->buttonStartStreams->setEnabled( true );
            ui->buttonStopStreams->setEnabled( false );
            ui->comboBoxAtts->setEnabled( false );
        }
    } else {
        ui->buttonInitHardware->setEnabled(  true );
        ui->buttonCloseHardware->setEnabled( false );
        ui->buttonStartStreams->setEnabled( false );
        ui->buttonStopStreams->setEnabled( false );
        ui->comboBoxAtts->setEnabled( false );
    }
}

void ItsMain::onButtonInitHw(bool) {
    SetHWButtonsGrey();
    emit signalInitHw( cfg->drv_type,
                       cfg->fn_img.c_str(),
                       cfg->have_submodules ? cfg->fn_hex.c_str() : NULL );
}

void ItsMain::onButtonCloseHw(bool) {
    SetHWButtonsGrey();
    emit signalCloseHw();
}

void ItsMain::onButtonStartHWStreams(bool) {
    SetHWButtonsGrey();
    emit signalStartHwStreams();
}

void ItsMain::onButtonStopHWStreams(bool) {
    SetHWButtonsGrey();
    emit signalStopHwStreams();
}

void ItsMain::onButtonCallDeveloperFun(bool) {
    AddHWLog( "Calling developer function" );
    emit signalCallDeveloperFun();
}

void ItsMain::onTimerGetDebugInfo() {
    if ( cfg->have_dbg ) {
        emit signalGetDebugInfo();
    } else {
        AddHWLog( "Debug polling disabled on start config" );
    }
}

void ItsMain::onTimerChanged(int) {
    bool ok;
    uint32_t tval = ui->comboBoxDbgTimer->currentData().toUInt(&ok);
    
    if ( tval == 0 || !ok ) {
        // Stop timer
        AddHWLog( "Timer stopped" );
        timer->stop();
    } else {
        // Start or restart timer
        AddHWLog( "Timer started" );
        timer->start( tval );
    }
}

void ItsMain::onAttChanged(int) {
    bool ok;
    uint32_t aval = ui->comboBoxAtts->currentData().toUInt(&ok);
    if ( !ok ) {
        aval = 0;
    }
    emit signalSetAttReg( att.GetRegForAtt(aval) );
}

void ItsMain::onRawForm(bool) {
    if ( rawForm ) {
        rawForm->show();
    }
}

void ItsMain::onGPSForm(bool) {
    if ( gpsForm ) {
        gpsForm->show();
    }
}

void ItsMain::onSpectrumForm(bool) {
    if ( specForm ) {
        specForm->show();
    }
}

void ItsMain::hardwareInitStatus(bool hw_init_status, QString log_message ) {
    AddHWLog( log_message );
    SetHWButtonsGrey();
    if ( hw_init_status ) {
        hw_inited = true;
        hw_started = false;        
        if ( cfg->auto_start_streams ) {
            emit signalStartHwStreams();
        }
    }
    SetHWButtons();
}

void ItsMain::hardwareCloseStatus(bool hw_close_status, QString log_message) {
    AddHWLog( log_message );
    SetHWButtonsGrey();
    if ( hw_close_status ) {
        hw_inited = false;
        hw_started = false;
    }
    SetHWButtons();
}

void ItsMain::hardwareStartStatus(bool hw_start_status, QString log_message) {
    AddHWLog( log_message );
    SetHWButtonsGrey();
    if ( hw_start_status ) {
        hw_started = true;
    }
    SetHWButtons();
}

void ItsMain::hardwareStopStatus(bool hw_stop_status, QString log_message) {
    AddHWLog( log_message );
    SetHWButtonsGrey();
    if ( hw_stop_status ) {
        hw_started = false;
    }
    SetHWButtons();
}

void ItsMain::hardwareDebugInfo(bool isok, fx3_dev_debug_info_t info) {
    QString str;
    if ( isok ) {
        bool ok;
        double seconds = ui->comboBoxDbgTimer->currentData().toUInt(&ok) / 1000.0;
        double speed = info.size_tx_mb_inc / seconds;

        if ( info.speed_only ) {
            str = str.sprintf( "Speed = %.1f Mb/sec", speed );
        } else {
            str = str.sprintf( "[%3d] Over: %4d +%d  Phy %5d +%4d  Lnk %d +%d ER[%08X]  %.1f Mb",
                      info.transfers,
                      info.overflows, info.overflows_inc,
                      info.phy_errs, info.phy_err_inc,
                      info.lnk_errs, info.lnk_err_inc,
                      info.err_reg_hex, speed );
        }
    } else {
        str = str.sprintf( "GetDebugInfo FAILED with error code %d", (int32_t) info.status );
    }
    AddHWLog( str );
    SetHWButtonsGrey();
    SetHWButtons();
}
