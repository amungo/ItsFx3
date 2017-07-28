#ifndef PHASEFORM_H
#define PHASEFORM_H
#include <mutex>
#include <vector>
#include <thread>

#include <QWidget>
#include <QCamera>

#include "datastreams/streamdatahandler.h"
#include "datastreams/streamrouter.h"
#include "datastreams/singleevent.h"
#include "gcacorr/averagervector.h"
#include "gcacorr/fftwrapper.h"
#include "gcacorr/etalometrgeo.h"
#include "gcacorr/etalometrfile.h"

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

private:
    Ui::PhaseForm *ui;
    QCamera *camera;

    std::vector< std::vector<float_cpx_t> > fft_out_averaged;
    std::vector<std::vector<std::vector<float_cpx_t>>> tbuf_fft;
    std::vector<float> tbuf_powers;
    std::vector<float> tbuf_phases;
    FFTWrapper fft;

    SingleEvent event_data;
    std::mutex mtx_data;
    bool data_is_busy = false;
    bool data_valid = false;
    std::vector<std::vector<short>> all_ch_data;
    bool TryLockData();
    void UnlockData();

    std::mutex mtx_convolution;
    std::vector< std::vector<float> > powers;
    std::vector< std::vector<float> > phases;
    float_cpx_t cur_iqss[4];

    float powerMin = -20.0f;
    float powerMax = 100.0f;
    float powerAvg =   0.0f;
    float powerMaxCur = 25.0f;
    float GetThreshold();

    void MakeFFTs();
    void MakePphs();
    void SetWidgetsData();
    void CalcConvolution();

    std::thread tick_thr;
    bool running;
    void Tick();

    int curIdx = 1;
    int GetCurrentIdx();
    void SetCurrentIdx( int x );

    int avg_filter_cnt = 25;

    void InitCamera();

    EtalometrGeo  et_geo;
    EtalometrFile et_file;
    EtalometrBase* et;

    float nullMHz = 1590.0f;

    void RecalculateEtalons();
    double GetCurrentFreqHz();


public slots:
    void ChangeNullMhz(double newVal);

    void CurChangeOutside( int value );
    void CurChangeButtonUpSlow(bool);
    void CurChangeButtonUpFast(bool);
    void CurChangeButtonDownSlow(bool);
    void CurChangeButtonDownFast(bool);

    void CurBandChange( int value );
    void CurBandChangeUp(bool);
    void CurBandChangeDown(bool);

    void CalibrateApplyPhases(bool);
    void CalibrateDefault(bool);
    void ChangeEtalons(bool);

private slots:
    void slotRun(int);

    // StreamDataHandler interface
public:
    void HandleAllChansData(std::vector<short*>& new_all_ch_data, size_t pts_cnt );
};

#endif // PHASEFORM_H
