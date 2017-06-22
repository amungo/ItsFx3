#ifndef PHASEFORM_H
#define PHASEFORM_H
#include <mutex>
#include <vector>
#include <thread>

#include <QWidget>
#include <QCamera>

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
    QCamera *camera;

    std::vector< std::vector<float_cpx_t> > fft_out_averaged;
    std::vector<std::vector<std::vector<float_cpx_t>>> tbuf_fft;
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

    int GetCurrentIdx();

    void InitCamera();

private slots:
    void slotRun(int);

    // StreamDataHandler interface
public:
    void HandleAllChansData( std::vector<short*>& all_ch_data, size_t pts_cnt );
};

#endif // PHASEFORM_H
