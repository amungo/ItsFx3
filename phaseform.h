#ifndef PHASEFORM_H
#define PHASEFORM_H

#include <vector>
#include <list>

#include <QWidget>

#include "datastreams/streamdatahandler.h"
#include "datastreams/streamrouter.h"
#include "gcacorr/averagervector.h"
#include "gcacorr/fftwrapper.h"

namespace Ui {
class PhaseForm;
}

class PhaseForm : public QWidget, StreamDataHandler
{
    Q_OBJECT

public:
    explicit PhaseForm(QWidget *parent = 0);
    ~PhaseForm();

    StreamRouter* router;

private:
    Ui::PhaseForm *ui;

    std::vector< Averager<float_cpx_t>* > fft_out;
    std::vector<float_cpx_t> tbuf_fft_out;
    std::vector<float> tbuf_powers;
    std::vector<float> tbuf_phases;

    FFTWrapper fft;

private slots:
    void slotRun(int);

    // StreamDataHandler interface
public:
    void HandleADCStreamData(void*, size_t);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel);
};

#endif // PHASEFORM_H
