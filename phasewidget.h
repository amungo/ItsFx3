#ifndef PHASEWIDGET_H
#define PHASEWIDGET_H

#include <mutex>
#include <QWidget>

class PhaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PhaseWidget(QWidget *parent = 0);

    void SetPhasesData(std::vector< std::vector<float> >* phases_data, int skip_pts, int pts_cnt);
    void SetCurrentIdx(int idx);
    int  GetCurrentIdx();

private:
    std::vector< std::vector<float> >* phases;
    std::mutex mtx;
    int idx      = 1;
    int skip_pts = 0;
    int pts_cnt  = 0;

    Qt::GlobalColor chan_colors[4];

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
};

#endif // PHASEWIDGET_H
