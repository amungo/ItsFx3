#include "tuneform.h"
#include "ui_tuneform.h"

TuneForm::TuneForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneForm)
{
    ui->setupUi(this);

    QObject::connect(ui->pushButtonApplyFreq, SIGNAL(clicked(bool)), this, SLOT(onButtonApplyFreq(bool)) );
    QObject::connect(ui->comboBoxBand, SIGNAL(currentIndexChanged(int)), this, SLOT(onBandChanged(int)) );

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

