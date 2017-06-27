#ifndef COVERWIDGET_H
#define COVERWIDGET_H

#include <mutex>
#include <QWidget>

class CoverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CoverWidget(QWidget *parent = 0);
    void SetTarget( float alpha_deg, float phi_deg );

private:
    std::mutex mtx;
    float alpha_deg;
    float phi_deg;

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
};

#endif // COVERWIDGET_H
