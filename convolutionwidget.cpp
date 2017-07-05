#include <QPainter>
#include "videowidget.h"
#include "convolutionwidget.h"

#include <cmath>

using namespace std;

#define MY_PI (3.14159265359f)

ConvolutionWidget::ConvolutionWidget(QWidget *parent) : QWidget(parent)
{
    colors.resize(255);
    for ( size_t i = 0; i < colors.size(); i++ ) {
        colors[i] = QColor( 0, i, 0, 64 );
    }
    conv_paint.resize(1);
    conv_paint[0].resize(1);
}

void ConvolutionWidget::SetConvolution(ConvResult *convolution)
{
    lock_guard<mutex> lock(mtx);
    this->conv = convolution;
    float min = conv->min;
    float max = conv->max;

    min = max * 0.95;

    float len = max - min;
    float color_range = 250.0;
    float coef = color_range / len;

    conv_paint.resize( conv->data.size() );
    for ( size_t a = 0; a < conv->data.size(); a++ ) {
        std::vector<float>& raw = conv->data[a];
        conv_paint[a].resize( raw.size() );
        for( size_t p = 0; p < raw.size(); p++ ) {
            int color_idx = (int)(  (raw[p] - min) * coef  );
            conv_paint[a][p] = color_idx >= 0 ? colors[ color_idx ] : colors[ 0 ];
        }
    }

}

QSize ConvolutionWidget::getFrameSize()
{
    bool is_set = false;
    QObject* prnt = this->parent();
    if ( prnt ) {
        VideoWidget* videoWidget = dynamic_cast<VideoWidget*>( prnt );
        if ( videoWidget ) {
            this->frameSize = videoWidget->getFrameSize();
            if ( frameSize.width() > 10 ) {
                is_set = true;
            }
        }
    }
    if ( !is_set ) {
        this->frameSize.setHeight( this->height() );
        this->frameSize.setWidth( this->width() );
    }
    return this->frameSize;
}

void ConvolutionWidget::paintEvent(QPaintEvent* /*event*/)
{
    lock_guard<mutex> lock(mtx);
    if ( !conv ) {
        return;
    }

    QPainter painter( this );
    getFrameSize();
    float W = frameSize.width();
    float W2 = W/2.0f;
    float H = frameSize.height();
    float H2 = H/2.0f;
    painter.translate((width() - W)/2, (height() - H)/2);

    float conv_xsize = (float)conv_paint.size();
    float conv_ysize = (float)conv_paint.at(0).size();
    float xscale = (float)W/conv_xsize;
    float yscale = (float)H/conv_ysize;
    QPoint Center( conv_xsize/2, conv_ysize/2 );


    const QTransform originalTransform = painter.transform();
    //painter.translate(Center);
    painter.scale(xscale, yscale);

    for ( size_t th_idx = 0; th_idx < conv_paint.size(); th_idx++ ) {

        std::vector<QColor>& raw = conv_paint[th_idx];

        for( size_t ph_idx = 0; ph_idx < raw.size(); ph_idx++ ) {

            if ( raw[ph_idx] == colors[ 0 ] ) {
                continue;
            } else {
                QColor& color = raw[ph_idx];
                painter.setPen( QPen( color, 4, Qt::SolidLine) );
                painter.setBrush( QBrush( color ) );

                painter.drawPoint( ph_idx, conv_ysize - th_idx - 1 );
            }
        }
    }

    painter.setTransform( originalTransform );
    painter.setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
    painter.drawLine( W2, 0, W2, H );
    painter.drawLine( 0, H2, W, H2 );


    //painter.setPen( QPen( Qt::red, 1, Qt::SolidLine) );
    //painter.drawText( Center, QString(" %1 ").arg(QString::number((double)conv->max, 'g', 4 )) );

}
