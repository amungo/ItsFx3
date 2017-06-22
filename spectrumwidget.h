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
            float min,
            float max,
            float avg );

    void SetCurrentIdx(int idx);
    int  GetCurrentIdx();

private:
    std::vector< std::vector<float> >* powers;
    std::mutex mtx;
    int idx      = 1;
    int skip_pts = 0;
    int pts_cnt  = 0;

    float min = -10.0f;
    float max = 100.0f;
    float avg =   0.0f;

    Qt::GlobalColor chan_colors[4];

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
};

#endif // SPECTRUMWIDGET_H
