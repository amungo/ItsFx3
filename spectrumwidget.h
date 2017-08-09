#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <mutex>
#include <QColor>
#include <QMouseEvent>
#include <QWidget>

class SpectrumWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrumWidget(QWidget *parent = 0);

    void SetPowersData(
            std::vector< std::vector<float> >* powers_data,
            int skip_pts,
            int pts_cnt,
            float minval,
            float maxval,
            float avgval,
            float maxval_cur,
            float norm_threshold );

    void SetPointsParams( int skip_pts, int pts_cnt );

    void SetCurrentIdx(int idx, int band = 1);
    int  GetCurrentIdx();
    void SetSpectrumParams( double nullHz, double leftHz, double rightHz, double filterHz );

    enum SpecMode_e {
        spec_horiz = 0,
        spec_vert  = 1
    };
    void SetVisualMode( SpecMode_e newmode );
    float GetThresholdDb();

    void SetPowerShift( float pshift_db ) { powerShift = pshift_db; }
    float GetPowerShift()                 { return powerShift;      }
    void SetPowerRange( float prange_db ) { powerRange = prange_db; }
    float GetPowerRange()                 { return powerRange;      }

    void SetChannelMask( uint32_t chanmask ) {this->chanmask = chanmask; }

private:
    SpecMode_e mode = spec_horiz;
    std::vector< std::vector<float> >* powers;
    std::vector<float> maxpowers;
    std::mutex mtx;
    int idx      = 1;
    int idxBand  = 1;
    int skip_pts = 0;
    int pts_cnt  = 0;

    float minval = -10.0f;
    float maxval = 100.0f;
    float avgval =   0.0f;
    float maxval_cur = 25.0f;

    double nullHz   = 1590.0e6;
    double leftHz   =   10.0e6;
    double rightHz  =   20.0e6;
    double filterHz = 12400.0;

    float powerRange =  42.0f;
    float powerShift = -18.0f;
    float thresholdNorm  = 0.05f;
    float thresholdDb    = 20.0f;
    void  SetThresholdNorm( float normValue );

    QColor chan_colors[4];
    QColor colorActive;
    QColor colorPassive;

    void PaintHorizontal( QPainter& painter );
    void PaintVertical( QPainter& painter );

    const float border = 20.0f;

    uint32_t chanmask = 0xFF;

signals:
    void sendNewCurIdx( int );
public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif // SPECTRUMWIDGET_H
