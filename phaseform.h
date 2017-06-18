#ifndef PHASEFORM_H
#define PHASEFORM_H
#include <mutex>
#include <vector>
#include <list>
#include <thread>

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

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::PhaseForm *ui;

    std::vector< Averager<float_cpx_t>* > fft_out;
    std::vector<float_cpx_t> tbuf_fft_out;
    std::vector<float> tbuf_powers;
    std::vector<float> tbuf_phases;
    FFTWrapper fft;


    std::mutex mtx;
    std::vector< std::vector<float> > powers;
    std::vector< std::vector<float> > phases;
    bool pphs_valid;
    void MakePphs();

    std::thread tick_thr;
    bool running;
    void Tick();

    void PaintPowers();

    Qt::GlobalColor chan_colors[4];

private slots:
    void slotRun(int);

    // StreamDataHandler interface
public:
    void HandleADCStreamData(void*, size_t);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel);
};

#endif // PHASEFORM_H
