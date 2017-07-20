#ifndef TUNEFORM_H
#define TUNEFORM_H

#include <QWidget>
#include "hwfx3/fx3tuner.h"

namespace Ui {
class TuneForm;
}

class TuneForm : public QWidget
{
    Q_OBJECT

public:
    explicit TuneForm(QWidget *parent = 0);
    ~TuneForm();

public slots:
    void setDevicePointer(FX3DevIfce* dev_pointer);

    void onButtonApplyFreq(bool);

    void onBandChanged(int);

private:
    Ui::TuneForm *ui;
    Fx3Tuner* tuner = nullptr;

    const double MAX_L1 = 1750.0;
    const double MIN_L1 = 1350.0;
    const double DEF_L1 = 1590.0;

    const double MAX_L2 = 1400.0;
    const double MIN_L2 = 1040.0;
    const double DEF_L2 = 1235.0;
};

#endif // TUNEFORM_H
