#ifndef RAWSIGNALFORM_H
#define RAWSIGNALFORM_H

#include <QWidget>
#include <QCheckBox>
#include "gui/qcustomplot.h"
#include "datastreams/streamdatahandler.h"
#include "datastreams/streamrouter.h"
#include "datahandlers/streamdumper.h"
#include "datahandlers/streamgapchecker.h"
#include "hwfx3/fx3config.h"

namespace Ui {
class RawSignalForm;
}

class RawSignalForm : public QWidget, public StreamDataHandler, public GapCounterCallBackIfce, public FileDumpCallbackIfce
{
    Q_OBJECT
    
public:
    explicit RawSignalForm(FX3Config* cfg, QWidget *parent = 0);
    ~RawSignalForm();
    
    // StreamDataHandler interface
    virtual void HandleADCStreamData(void* data, size_t size8);
    void HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int chip, int channel);

    StreamRouter* router;
    
private:
    Ui::RawSignalForm *ui;
    FX3Config* cfg;
    std::vector<QCheckBox*> checkBoxShowChannels;
    QCustomPlot* rawPlot;
    StreamDumper dumper;
    StreamGapChecker gap_checker;
    uint32_t points_num;
    uint32_t skipped_packets;
    bool first_run;
    
    double last_axis;
    double axis_blank_percent;
    
    bool replot_is_in_progress;
    void SetLabels();
    void showRawSignal(const std::vector<float>& data, int channel_num, bool is_second_component = false );
    int chan_count;
    std::vector<int> last_shift;
    static const int MAX_CHANS = 4;
signals:
    void signalNeedReplot();
private slots:
    void slotReplot();
    void slotReplotComplete();
    
    void slotRun(int);
    void slotRecOnOff(int);

    void slotChooseFile(bool);

    void slotGapCountOnOff(int);
    void slotGapValChanged(int);

    void slotRecOneShot(bool);
    
    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent* ev);

    // GapCounterCallBackIfce interface
public:
    void GapCountChanged(int gap_count, double max_gap_value);

    // FileDumpCallbackIfce interface
public:
    void onFileDumpComplete();
};

#endif // RAWSIGNALFORM_H
