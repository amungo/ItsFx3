#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include <QFileDialog>

// test commit

DialogConfig::DialogConfig(FX3Config* cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig),
    cfg( cfg )
{
    ui->setupUi(this);
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked(bool)), this, SLOT(accept()));
    QObject::connect(this, SIGNAL(accepted()), this, SLOT(setConfigValues()));
    QObject::connect(ui->comboBoxBoardType, SIGNAL(currentIndexChanged(int)), this, SLOT(boardChanged(int)));
    QObject::connect(ui->pushButtonFileImg, SIGNAL(clicked(bool)), this, SLOT(openFileImg(bool)));
    QObject::connect(ui->pushButtonFileAddFw, SIGNAL(clicked(bool)), this, SLOT(openFileAddFw(bool)));
    QObject::connect(ui->checkBoxHaveSubs, SIGNAL(stateChanged(int)), this, SLOT(changedSubs(int)));


    int dtm = 0;
    bool have_cypress = false;
    int cypress_idx = 0;
#ifndef NO_CY_API
    have_cypress = true;
    cypress_idx = dtm;
    ui->comboBoxDriverType->insertItem( dtm++, "Cypress API", QVariant(DrvTypeCypress));
#endif
    int libusb_idx = dtm;
    ui->comboBoxDriverType->insertItem( dtm++, "LibUsb API", QVariant(DrvTypeLibUsb));
    int filesim_idx = dtm;
    ui->comboBoxDriverType->insertItem( dtm++, "File Sim", QVariant(DrvTypeFileSim));

    if ( have_cypress ) {
        ui->comboBoxDriverType->setCurrentIndex(cypress_idx);
    } else {
        ui->comboBoxDriverType->setCurrentIndex(libusb_idx);
    }
    ui->comboBoxDriverType->setCurrentIndex(filesim_idx);

    for ( int dtm = 0; dtm < ( int ) ADC_Types_Count; dtm++ ) {
        #ifdef NO_GPS
        if ( ( ADCType ) dtm == ADC_4ch_2bit ) {
            ui->comboBoxBoardType->insertItem( dtm, "DISABLED" ), QVariant( ( ADCType ) dtm ) );
            continue;
        }
        #endif
        ui->comboBoxBoardType->insertItem( dtm, FX3Config::GetAdcString( ( ADCType ) dtm ), QVariant( ( ADCType ) dtm ) );
    }
    ui->comboBoxBoardType->setCurrentIndex(0);

    reSetFields();

#ifdef NO_GPS
    ui->labelFreqInterGPS->setEnabled( false );
    ui->labelFreqInterGPS->setVisible( false );
    ui->lineEditGPSInterFreq->setEnabled( false );
    ui->lineEditGPSInterFreq->setVisible( false );
#endif
}

DialogConfig::~DialogConfig()
{
    delete ui;
}


void DialogConfig::boardChanged(int)
{
    bool ok;
    cfg->ConfigDefault( (ADCType)ui->comboBoxBoardType->currentData().toInt(&ok) );
    reSetFields();
}

void DialogConfig::reSetFields() {
    ui->lineEditADCSampleRate->setText( QString::number( cfg->adc_sample_rate_hz, 'f', 0 ) );
    ui->lineEditGPSInterFreq->setText(  QString::number( cfg->inter_freq_hz, 'f', 0 ) );
    ui->checkBoxHaveAtts->setChecked( cfg->have_atts );
    ui->checkBoxHaveSubs->setChecked( cfg->have_submodules );
    ui->checkBoxHaveDbg->setChecked( cfg->have_dbg );
    ui->lineEditImageFileName->setText( cfg->fn_img.c_str() );
    ui->lineEditAdditionalImageFileName->setText( cfg->fn_hex.c_str() );
    ui->checkBoxAutoStart->setChecked( cfg->auto_start_streams );
    changedSubs( cfg->have_submodules );
}

void DialogConfig::openFileImg(bool) {
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
        tr("Select main image file"),
        "",
        tr("Images (*.img);;All files (*.*)" )
    );
    if ( fileName.size() > 1 ) {
        ui->lineEditImageFileName->setText( fileName );
    }
}

void DialogConfig::openFileAddFw(bool) {
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
        tr("Select additional (hex) file"),
        "",
        tr("HEX-images (*.hex);;All files (*.*)" )
    );
    if ( fileName.size() > 1 ) {
        ui->lineEditAdditionalImageFileName->setText( fileName );
    }
}

void DialogConfig::changedSubs(int state ) {
    if ( state ) {
        ui->pushButtonFileAddFw->setEnabled(true);
        ui->lineEditAdditionalImageFileName->setEnabled(true);
    } else {
        ui->pushButtonFileAddFw->setEnabled(false);
        ui->lineEditAdditionalImageFileName->setEnabled(false);
    }
}


void DialogConfig::setConfigValues() {
    bool ok;
    cfg->drv_type = (DriverType_t)ui->comboBoxDriverType->currentData().toInt(&ok);
    cfg->adc_type = (ADCType)ui->comboBoxBoardType->currentData().toInt(&ok);
    cfg->have_submodules = ui->checkBoxHaveSubs->isChecked();
    cfg->have_atts = ui->checkBoxHaveAtts->isChecked();
    cfg->have_dbg  = ui->checkBoxHaveDbg->isChecked();
    cfg->fn_img = ui->lineEditImageFileName->text().toLatin1().data();
    cfg->fn_hex = ui->lineEditAdditionalImageFileName->text().toLatin1().data();
    cfg->adc_sample_rate_hz = ui->lineEditADCSampleRate->text().toDouble(&ok);
    cfg->inter_freq_hz = ui->lineEditGPSInterFreq->text().toDouble(&ok);
    cfg->auto_start_streams = ui->checkBoxAutoStart->isChecked();

    cfg->FinishConfigure();
}
