#ifndef SPECTRUMFORM_H
#define SPECTRUMFORM_H

#include <QWidget>
#include <mutex>
#include "gui/qcustomplot.h"

#include "datastreams/streamdatahandler.h"
#include "datastreams/streamrouter.h"
#include "hwfx3/fx3config.h"
#include "gcacorr/fftwrapper.h"
#include "gcacorr/averagervector.h"

namespace Ui {
class SpectrumForm;
}

class SpectrumForm : public QWidget, public StreamDataHandler
{
    Q_OBJECT

public:
    explicit SpectrumForm( FX3Config* cfg, QWidget *parent = 0);
    ~SpectrumForm();

    StreamRouter* router;

private:
    Ui::SpectrumForm *ui;
    QCustomPlot* specPlot;
    std::vector<QCheckBox*> checkBoxShowChannels;
    static const int MAX_CHANS = 4;

    FX3Config* cfg;
    FFTWrapper* fft;
    static const int nFftDefault = 1024;
    int nFft;
    int visN;
    float_cpx_t* fftbuf;
    std::vector< Averager<double>* > avg;

    bool replot_is_in_progress;
    std::mutex mtx;

    void ShowSpectrumReal( const float* real_data, int pts_cnt, int chan );
    void ShowSpectrumComplex( const float_cpx_t *complex_data, int pts_cnt, int chan );

    void ShowSpectrum(int chan , double *powers);

    int GetMaxCheckedChannel();

    QPoint startMove;


signals:
    void signalNeedReplot();

private slots:
    void slotReplot();
    void slotReplotComplete();

    void slotRun(int);
    void avgChanged(int);
    void onMWheel(QWheelEvent*);
    void onMPress(QMouseEvent*);
    void onMRelease(QMouseEvent*);


    // StreamDataHandler interface
public:
    void HandleADCStreamData(void*, size_t);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel);
};

#endif // SPECTRUMFORM_H
