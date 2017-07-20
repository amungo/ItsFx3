#include "tuneform.h"
#include "ui_tuneform.h"

TuneForm::TuneForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneForm)
{
    ui->setupUi(this);

    ui->comboBoxBand->setCurrentIndex(0);
    ui->doubleSpinBoxFreq->setMinimum(MIN_L1);
    ui->doubleSpinBoxFreq->setMaximum(MAX_L1);
    ui->doubleSpinBoxFreq->setValue(DEF_L1);

    QObject::connect(ui->pushButtonApplyFreq, SIGNAL(clicked(bool)), this, SLOT(onButtonApplyFreq(bool)) );
    QObject::connect(ui->comboBoxBand, SIGNAL(currentIndexChanged(int)), this, SLOT(onBandChanged(int)) );

    QObject::connect(ui->checkBoxAutoGC, SIGNAL(stateChanged(int)), this, SLOT(onAGCChanged(int)) );
    QObject::connect(ui->horizontalSliderRFGain,       SIGNAL(valueChanged(int)), this, SLOT(onManualGainChanged(int)) );
    QObject::connect(ui->horizontalSliderIFGainCoarse, SIGNAL(valueChanged(int)), this, SLOT(onManualGainChanged(int)) );
    QObject::connect(ui->horizontalSliderIFGainFine,   SIGNAL(valueChanged(int)), this, SLOT(onManualGainChanged(int)) );

    SetGainLabels();
    SetGainsControlsVisibility(ui->checkBoxAutoGC->isChecked());
}

TuneForm::~TuneForm()
{
    delete ui;
}

void TuneForm::setDevicePointer(FX3DevIfce *dev_pointer)
{
    Fx3Tuner* old_tuner = tuner;
    if ( dev_pointer ) {
        tuner = new Fx3Tuner( dev_pointer );
    } else {
        tuner = nullptr;
    }
    if ( old_tuner ) {
        delete old_tuner;
    }
}

void TuneForm::onButtonApplyFreq(bool)
{
    int idx = ui->comboBoxBand->currentIndex();
    int band;
    if ( idx == 0 ) {
        band = Fx3Tuner::BandL1;
    } else {
        band = Fx3Tuner::BandL2;
    }
    double freq_want = ui->doubleSpinBoxFreq->value() * 1000000.0;
    if ( tuner ) {
        tuner->SetPLL( 0, band, 1 );
        double freq_got = tuner->SetFreq(0, freq_want);
        ui->doubleSpinBoxFreq->setValue(freq_got/1000000.0);
        emit newFreq( freq_got );
    }

}

void TuneForm::onBandChanged(int) {
    int idx = ui->comboBoxBand->currentIndex();
    int band;
    if ( idx == 0 ) {
        band = Fx3Tuner::BandL1;
    } else {
        band = Fx3Tuner::BandL2;
    }

    if ( band == Fx3Tuner::BandL1 ) {

        double curval = ui->doubleSpinBoxFreq->value();
        if ( curval > MAX_L1 || curval < MIN_L1 ) {
            curval = DEF_L1;
        }
        ui->doubleSpinBoxFreq->setMinimum(MIN_L1);
        ui->doubleSpinBoxFreq->setMaximum(MAX_L1);
        ui->doubleSpinBoxFreq->setValue(curval);

    } else if ( band == Fx3Tuner::BandL2 ) {

        double curval = ui->doubleSpinBoxFreq->value();
        if ( curval > MAX_L2 || curval < MIN_L2 ) {
            curval = DEF_L2;
        }
        ui->doubleSpinBoxFreq->setMinimum(MIN_L2);
        ui->doubleSpinBoxFreq->setMaximum(MAX_L2);
        ui->doubleSpinBoxFreq->setValue(curval);

    }
}

void TuneForm::onAGCChanged(int)
{
    if ( ui->checkBoxAutoGC->isChecked() ) {
        SetGainsControlsVisibility(true);
    } else {
        SetGainsControlsVisibility(false);
        SetGainLabels();
    }
    SendGain();
}

void TuneForm::onManualGainChanged(int)
{
    SetGainLabels();
    SendGain();
}

void TuneForm::SendGain() {
    if ( ui->checkBoxAutoGC->isChecked() ) {
        if ( tuner ) {
            tuner->Set_AGC();
        }
    } else {
        if ( tuner ) {
            tuner->Set_MGC(
                ui->horizontalSliderRFGain->value(),
                ui->horizontalSliderIFGainCoarse->value(),
                ui->horizontalSliderIFGainFine->value()
            );
        }
    }
}

void TuneForm::SetGainLabels() {
    int rf   = ui->horizontalSliderRFGain->value();
    int ifac = ui->horizontalSliderIFGainCoarse->value();
    int ifaf = ui->horizontalSliderIFGainFine->value();

    double rfg_db = 11.0 + rf*0.95;

    double ifa_fine_db = 0.0;
    if ( ifaf < 8 ) {
        ifa_fine_db = -0.35;
    } else if ( ifaf > 24 ) {
        ifa_fine_db = 5.10;
    } else {
        switch ( ifaf  ) {
            case  8: ifa_fine_db = -0.30; break;
            case  9: ifa_fine_db = -0.10; break;
            case 10: ifa_fine_db =  0.30; break;
            case 11: ifa_fine_db =  0.90; break;
            case 12: ifa_fine_db =  1.70; break;
            case 13: ifa_fine_db =  2.40; break;
            case 14: ifa_fine_db =  3.00; break;
            case 15: ifa_fine_db =  3.40; break;
            case 16: ifa_fine_db =  3.80; break;
            case 17: ifa_fine_db =  4.10; break;
            case 18: ifa_fine_db =  4.40; break;
            case 19: ifa_fine_db =  4.55; break;
            case 20: ifa_fine_db =  4.70; break;
            case 21: ifa_fine_db =  4.80; break;
            case 22: ifa_fine_db =  4.90; break;
            case 23: ifa_fine_db =  5.00; break;
            case 24: ifa_fine_db =  5.05; break;
        }
    }
    double ifa_coarse_db = -0.5 + ifac * 2.64583;
    double ifag_db = ifa_coarse_db + ifa_fine_db;

    ui->labelRFGain->setText( QString(" %1 dB").arg( QString::number(
        rfg_db, 'f', 3  ) ));

    ui->labelIFAGain->setText( QString(" %1 dB").arg( QString::number(
                                                          ifag_db, 'f', 3  ) ));
}

void TuneForm::SetGainsControlsVisibility(bool is_auto)
{
    ui->horizontalSliderIFGainCoarse->setEnabled(!is_auto);
    ui->horizontalSliderIFGainFine->setEnabled(!is_auto);
    ui->horizontalSliderRFGain->setEnabled(!is_auto);
    ui->labelIFAGain->setEnabled(!is_auto);
    ui->labelRFGain->setEnabled(!is_auto);
}

