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

    min = max * 0.995;

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

void ConvolutionWidget::recalcTransform()
{
    if ( !conv ) {
        return;
    }
    if ( lastFrameSize.height() != frameSize.height() ||
         lastFrameSize.width()  != frameSize.width()  ||
         lastYSize != conv_paint.size() ||
         lastXSize != conv_paint.at(0).size() )
    {
        lastFrameSize = frameSize;
        lastYSize = conv_paint.size(); // thetta
        lastXSize = conv_paint.at(0).size(); // phi
        stepDeg = (int)round( lastXSize / ( 2.0f * conv->rangePhi) );

        float W = frameSize.width();
        float H = frameSize.height();

        float xscale = (float)W/(float)lastXSize;
        float yscale = (float)H/(float)lastYSize;
        fprintf( stderr, "ConvolutionWidget::recalcTransform(): "
                         "%d x %d   %.0f x %.0f    %d\n",
                 lastXSize, lastYSize, W, H, stepDeg );

        if ( xtr.size() != lastXSize && lastXSize != 0 ) {
            xtr.resize(lastXSize);
        }
        for ( int i = 0; i < xtr.size(); i++ ) {
            xtr[i] = i * xscale;
        }

        if ( ytr.size() != lastYSize && lastYSize != 0 ) {
            ytr.resize(lastYSize);
        }
        for ( int i = 0; i < ytr.size(); i++ ) {
            ytr[i] = H - i * yscale - 1;
        }
    }
}

void ConvolutionWidget::paintEvent(QPaintEvent* /*event*/)
{
    lock_guard<mutex> lock(mtx);
    if ( !conv ) {
        return;
    }

    getFrameSize();
    recalcTransform();
    QPainter painter( this );

    float W = frameSize.width();
    float W2 = W/2.0f;
    float H = frameSize.height();
    float H2 = H/2.0f;
    painter.translate((width() - W)/2, (height() - H)/2);

    for ( size_t th_idx = 0; th_idx < conv_paint.size(); th_idx++ ) {

        std::vector<QColor>& raw = conv_paint[th_idx];

        for( size_t ph_idx = 0; ph_idx < raw.size(); ph_idx++ ) {

            if ( raw[ph_idx] == colors[ 0 ] ) {
                continue;
            } else {
                QColor& color = raw[ph_idx];
                painter.setPen( QPen( color, 4, Qt::SolidLine) );
                painter.setBrush( QBrush( color ) );

                painter.drawPoint( xtr[ph_idx], ytr[th_idx] );
            }
        }
    }

    painter.setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
//    painter.drawLine( W2, 0, W2, H );
//    painter.drawLine( 0, H2, W, H2 );

    int idx_step = 10 / stepDeg;
    for ( int i = 0; i < lastXSize/2; i += idx_step ) {
        int idx = lastXSize/2 - i;
        painter.drawLine(xtr[idx], H2-5, xtr[idx], H2+5);
        idx = lastXSize/2 + i;
        painter.drawLine(xtr[idx], H2-5, xtr[idx], H2+5);
    }

    for ( int i = 0; i < lastYSize/2; i += idx_step ) {
        int idx = lastYSize/2 - i;
        painter.drawLine(W2-5, ytr[idx], W2+5, ytr[idx]);
        idx = lastYSize/2 + i;
        painter.drawLine(W2-5, ytr[idx], W2+5, ytr[idx]);
    }
}
