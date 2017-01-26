#ifndef GPSCORRFORM_H
#define GPSCORRFORM_H

#include <QWidget>
#include <QThread>
#include <QMutex>
#include <thread>
#include <map>
#include <vector>

#include "gui/qcustomplot.h"
#include "gcacorr/gpsvis.h"
#include "hwfx3/fx3config.h"
#include "datastreams/streamrouter.h"
#include "datahandlers/streamleapdumper.h"
#include "util/TimeComputator.h"

static const int PRN_CNT = 30;
static const int PRN_MAX = 33;
static const int PRN_IN_OPER[ PRN_CNT ] =
{ 1, 2, 3,
  5, 6, 7, 8, 9,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};


struct plot_data_t {
    std::vector<double> freqs_vals;
    std::vector< std::vector<float> > cors;
    int center;
    bool inited;
    QMutex* mutex;
    plot_data_t() : inited( false ){ mutex = new QMutex(); }
    ~plot_data_t() { delete mutex; }
};

namespace Ui {
class GPSCorrForm;
}

class GPSCorrForm : public QWidget, public StreamDataHandler, public ChunkDumpCallbackIfce
{
    Q_OBJECT

public:
    explicit GPSCorrForm(FX3Config* cfg, QWidget *parent = 0);
    ~GPSCorrForm();

    StreamRouter* router;

private:
    Ui::GPSCorrForm *ui;
    FX3Config* cfg;
    StreamLEAPDumper dumper;

    QCustomPlot* plotCorrAll;
    QCustomPlot* plotCorrGraph;

    QCPGraph* gr_vis;
    QCPGraph* gr_inv;

    QVector<double> visible_sats;
    QVector<double> invisible_sats;
    QVector<double> visible_corrs;
    QVector<double> invisible_corrs;

    bool working;
    void SetWorking( bool b );
    std::vector< RawSignal* > sigs;
    void calcSats();

    bool running;
    std::thread calc_thread;
    void calcLoop( void );

    void redrawVisGraph();

    std::vector< plot_data_t > cdata;

    std::vector< int > shifts;
    int relativeShift;
    void setshifts();

    int antijamIdx;

    void processRawData( const std::vector<short>* data );

    void uiRecalc();

    TimeComputator timer_corr_prepare;
    TimeComputator timer_corr;
    TimeComputator timer_corr_precise;

private slots:
    void satChanged(int prn, float corr, int shift, double freq, bool is_visible );
    void cellSelected( int, int );
    void RecFile(bool);
    void ChooseFile(bool);
    void RefreshPressed(int);
    void relativeCorrChanged(int);


signals:
    void satInfo( int prn, float corr, int shift, double freq, bool is_visible );

    // StreamDataHandler interface
public:
    void HandleADCStreamData(void *data, size_t size8);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel);

    // FileDumpCallbackIfce interface
public:
    void onFileDumpComplete(std::string fname, ChunkDumpParams params);
};

#endif // GPSCORRFORM_H
