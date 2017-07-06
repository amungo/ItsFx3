#ifndef CONVOLUTIONWIDGET_H
#define CONVOLUTIONWIDGET_H

#include <mutex>
#include <vector>
#include <QWidget>
#include <QColor>
#include "gcacorr/convresult.h"

class ConvolutionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConvolutionWidget(QWidget *parent = 0);
    void SetConvolution( ConvResult* convolution );

private:
    std::mutex mtx;
    ConvResult* conv = nullptr;
    std::vector<QColor> colors;
    std::vector<std::vector<QColor>> conv_paint;
    QSize getFrameSize();
    QSize frameSize;

    std::vector<int> xtr;
    std::vector<int> ytr;
    int lastXSize = 0;
    int lastYSize = 0;
    int stepDeg = 1;
    QSize lastFrameSize;
    void recalcTransform();

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // CONVOLUTIONWIDGET_H
