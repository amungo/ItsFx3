#include <QPainter>
#include "convolutionwidget.h"

#include <cmath>

using namespace std;

#define MY_PI (3.14159265359f)

ConvolutionWidget::ConvolutionWidget(QWidget *parent) : QWidget(parent)
{
    colors.resize(255);
    for ( size_t i = 0; i < colors.size(); i++ ) {
        colors[i] = QColor( i, 0, 0, 64 );
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

void ConvolutionWidget::paintEvent(QPaintEvent* /*event*/)
{
    lock_guard<mutex> lock(mtx);
    if ( !conv ) {
        return;
    }

    QPainter painter( this );
    float W = width();
    float W2 = W/2.0f;
    float H = height();
    float H2 = H/2.0f;

    QPoint Center( W2, H2 );
    float R = W < H ? W2 : H2;

    for ( size_t a = 0; a < conv_paint.size(); a++ ) {

        float alpha = a * 2.0 * MY_PI / (float)conv_paint.size();
        alpha -= MY_PI/2.0f;

        if ( abs( tanf( alpha ) ) < H/W ) {
            R = abs( W / (2.0f * cosf(alpha) ) );
        } else {
            R = abs( H / (2.0f * sinf(alpha) ) );
        }

        float cos_alpha = cosf( alpha );
        float sin_alpha = sinf( alpha );

        std::vector<QColor>& raw = conv_paint[a];

        for( size_t p = 0; p < raw.size(); p++ ) {

            if ( raw[p] == colors[ 0 ] ) {
                continue;
            } else {
                int shift = 0;

                float Z = R * ( p + shift ) / ((float) raw.size() + shift);
                QColor& color = raw[p];
                painter.setPen( QPen( color, 4, Qt::SolidLine) );
                painter.setBrush( QBrush( color ) );

                painter.drawPoint( Center.x() - Z * cos_alpha,
                                   Center.y() - Z * sin_alpha );
            }
        }
    }

    painter.setPen( QPen( Qt::green, 3, Qt::SolidLine) );
    painter.drawText( Center, QString(" %1 ").arg(QString::number((double)conv->max, 'g', 4 )) );

}
