#ifndef SPECTRUMFORM_H
#define SPECTRUMFORM_H

#include <QWidget>
#include <QCheckBox>
#include <mutex>
#include <thread>

#include "datastreams/streamdatahandler.h"
#include "datastreams/streamrouter.h"
#include "datastreams/singleevent.h"
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
    std::vector<QCheckBox*> checkBoxShowChannels;
    static const int MAX_CHANS = 4;
    int avg_cnt = 8;

    FX3Config* cfg;
    FFTWrapper* fft;
    static const int nFftDefault = 1024;
    int fft_len;
    int half_fft_len;

    SingleEvent event_data;
    std::mutex mtx_data;
    bool data_is_busy = false;
    bool data_valid = false;
    std::vector<std::vector<short>> all_ch_data;
    bool TryLockData();
    void UnlockData();

    std::thread calc_thread;
    bool running = false;
    void calc_loop();

    std::vector< std::vector<float_cpx_t> > fft_out_averaged;
    std::vector<std::vector<std::vector<float_cpx_t>>> tbuf_fft;
    std::vector< std::vector<float> > powers;
    void MakeFFTs();
    void MakePowers();
    void SetWidgetData();

    std::mutex pts_param_mtx;
    float nullMHz = 1590.0f;
    float bandMHz  = 53.0f / 2.0f;

    //float leftMHz  =  0.0f;
    //float rightMHz = 53.0f / 2.0f;
    double filterMHz;
    int left_point;
    int right_point;
    int points_cnt;
    double GetCurrentFreqHz();

    int curIdx = 1;
    int GetCurrentIdx();
    void SetCurrentIdx( int x );

public slots:
    void ChangeNullMhz(double newVal);
    void CurChangeOutside( int value );

private slots:
    void slotRun(int);
    void channelsChanged(int);
    void scalesShiftsChanged( int );

    // StreamDataHandler interface
public:
    void HandleAllChansData(std::vector<short*>& new_all_ch_data, size_t pts_cnt );

    // QWidget interface
protected:
    void hideEvent(QHideEvent *event);
};

#endif // SPECTRUMFORM_H
