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
    QObject::connect(ui->pushButtonFileAlgo, SIGNAL(clicked(bool)), this, SLOT(openFileAlgo(bool)));
    QObject::connect(ui->pushButtonFileData, SIGNAL(clicked(bool)), this, SLOT(openFileData(bool)));
    QObject::connect(ui->checkBoxHaveSubs, SIGNAL(stateChanged(int)), this, SLOT(changedSubs(int)));

    int dtm = 0;
    bool have_cypress = false;
    int cypress_idx = 0;
#ifndef NO_CY_API
    have_cypress = true;
    drivers_combobox_map[DrvTypeCypress] = dtm;
    ui->comboBoxDriverType->insertItem( dtm++, "Cypress API", QVariant(DrvTypeCypress));
#endif
    drivers_combobox_map[DrvTypeLibUsb] = dtm;
    ui->comboBoxDriverType->insertItem( dtm++, "LibUsb API", QVariant(DrvTypeLibUsb));

    drivers_combobox_map[DrvTypeFileSim] = dtm;
    ui->comboBoxDriverType->insertItem( dtm++, "File Sim", QVariant(DrvTypeFileSim));

    if ( have_cypress ) {
        ui->comboBoxDriverType->setCurrentIndex(drivers_combobox_map[DrvTypeCypress]);
    } else {
        ui->comboBoxDriverType->setCurrentIndex(drivers_combobox_map[DrvTypeLibUsb]);
    }
    //ui->comboBoxDriverType->setCurrentIndex(drivers_combobox_map[DrvTypeFileSim]);

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

    int hli = 0;
    ui->comboBoxHackedLen->insertItem( hli++, "no hack", QVariant(0) );
    ui->comboBoxHackedLen->insertItem( hli++, "fft4k x avg10", QVariant((int)(4096 * sizeof(short) * 10)));
    ui->comboBoxHackedLen->insertItem( hli++, "fft4k x avg20", QVariant((int)(4096 * sizeof(short) * 20)));

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
    ui->lineEditAdditionalImageFileName->setText( cfg->fn_hex.c_str());
    ui->lineEditAlgoFileName->setText(cfg->fn_alg.c_str());
    ui->lineEditDataFileName->setText(cfg->fn_data.c_str());
    ui->checkBoxAutoStart->setChecked( cfg->auto_start_streams );
    changedSubs( cfg->have_submodules );
    ui->comboBoxDriverType->setCurrentIndex(drivers_combobox_map[cfg->drv_type]);
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

void DialogConfig::openFileAlgo(bool) {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Lattice algorithm file"),
        "",
        tr("Images (*.sea);;All files (*.*)" )
    );
    if(fileName.size() > 1) {
        ui->lineEditAlgoFileName->setText( fileName );
    }
}

void DialogConfig::openFileData(bool) {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Lattice data file"),
        "",
        tr("Images (*.sed);;All files (*.*)" )
    );
    if(fileName.size() > 1) {
        ui->lineEditDataFileName->setText( fileName );
    }
}

void DialogConfig::changedSubs(int state ) {
    if ( state ) {
        ui->pushButtonFileAddFw->setEnabled(true);
        ui->lineEditAdditionalImageFileName->setEnabled(true);
        ui->lineEditAlgoFileName->setEnabled(true);
        ui->lineEditDataFileName->setEnabled(true);
    } else {
        ui->pushButtonFileAddFw->setEnabled(false);
        ui->lineEditAdditionalImageFileName->setEnabled(false);
        ui->lineEditAlgoFileName->setEnabled(false);
        ui->lineEditDataFileName->setEnabled(false);
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
    cfg->fn_alg = ui->lineEditAlgoFileName->text().toLatin1().data();
    cfg->fn_data = ui->lineEditDataFileName->text().toLatin1().data();
    cfg->adc_sample_rate_hz = ui->lineEditADCSampleRate->text().toDouble(&ok);
    cfg->inter_freq_hz = ui->lineEditGPSInterFreq->text().toDouble(&ok);
    cfg->auto_start_streams = ui->checkBoxAutoStart->isChecked();
    cfg->hacked_len = (int)ui->comboBoxHackedLen->currentData().toInt(&ok);

    cfg->FinishConfigure();
}
