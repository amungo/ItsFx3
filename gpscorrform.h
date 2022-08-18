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

    QCustomPlot* plotCorrGraph;

    std::vector<short> cached_one_chan_data;
    void PrepareRawData();

    bool working;
    void SetWorking( bool b );
    std::map< int, std::vector< RawSignal* > > sigs;
    void calcSats();

    bool running;
    std::thread calc_thread;
    void calcLoop( void );

    std::vector< plot_data_t > cdata;

    std::vector<QCheckBox*> calc_checks;

    std::vector<bool> visibles;
    std::vector< int > shifts;
    int relativeShift = 0;
    bool relativeShitValid = false;
    int selectedPrn = 1;
    void setshifts();

    std::vector<char> gyro_data;

    int antijamIdx;

    void processRawData( const std::vector<short>* data );

    void uiRecalc();

    enum GNSSType {
        GPS_L1 = 0,
        GLONASS_L1 = 1,
        GLONASS_L2 = 2
    } gnss_type;

    int GetFilterLen();
    float* GetFir();
    double GetFreq( int prn_num = 1 );
    int GetPrnCount();

    void setTableItem( int row, int col, const QString& str, bool is_greyed );

private slots:
    void satChanged(int prn, float corr, int shift, double freq, bool is_visible );
    void cellSelected( int, int );
    void RecFile(bool);
    void ChooseFile(bool);
    void RefreshPressed(int);
    void gnssTypeChanged(int);
    void prnCheckUncheck(int);
    void checkAll(bool);
    void uncheckAll(bool);
    void uncheckInVis(bool);
    void gyroChanged();


signals:
    void satInfo( int prn, float corr, int shift, double freq, bool is_visible );

    // StreamDataHandler interface
public:
    void HandleADCStreamData(void *data, size_t size8);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int chip, int channel);
    virtual void HandleGyroData(char* data, size_t size8);

    // FileDumpCallbackIfce interface
public:
    void onFileDumpComplete(std::string fname, ChunkDumpParams params);
};

#endif // GPSCORRFORM_H
