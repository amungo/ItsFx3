#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <mutex>
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
            float avgval );

    void SetCurrentIdx(int idx);
    int  GetCurrentIdx();
    void SetSpectrumParams( double nullHz, double leftHz, double rightHz, double filterHz );

    enum SpecMode_e {
        spec_horiz = 0,
        spec_vert  = 1
    };
    void SetVisualMode( SpecMode_e newmode );

private:
    SpecMode_e mode = spec_horiz;
    std::vector< std::vector<float> >* powers;
    std::vector<float> maxpowers;
    std::mutex mtx;
    int idx      = 1;
    int skip_pts = 0;
    int pts_cnt  = 0;

    float minval = -10.0f;
    float maxval = 100.0f;
    float avgval =   0.0f;

    double nullHz   = 1590.0e6;
    double leftHz   =   10.0e6;
    double rightHz  =   20.0e6;
    double filterHz = 12400.0;

    Qt::GlobalColor chan_colors[4];

    void PaintHorizontal( QPainter& painter );
    void PaintVertical( QPainter& painter );

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
};

#endif // SPECTRUMWIDGET_H
