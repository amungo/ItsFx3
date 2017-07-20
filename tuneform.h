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

private:
    Ui::TuneForm *ui;
    Fx3Tuner* tuner = nullptr;
};

#endif // TUNEFORM_H
