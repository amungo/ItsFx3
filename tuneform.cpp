#include "tuneform.h"
#include "ui_tuneform.h"

TuneForm::TuneForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneForm)
{
    ui->setupUi(this);
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

