#include <QPainter>
#include "convolutionwidget.h"

#include <cmath>

using namespace std;

#define MY_PI (3.14159265359f)

ConvolutionWidget::ConvolutionWidget(QWidget *parent) : QWidget(parent)
{
    colors.resize(255);
    for ( size_t i = 0; i < colors.size(); i++ ) {
        colors[i] = QColor( i, i, i, 255 );
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

    QPainter painter( this );
    float W = width();
    float W2 = W/2.0f;
    float H = height();
    float H2 = H/2.0f;

    painter.setPen( QPen( Qt::black, 2, Qt::SolidLine) );
    painter.setBrush( QBrush( Qt::black ) );
    painter.drawRect( 0, 0, W, H );

    //painter.setPen( QPen( Qt::green, 1, Qt::DotLine) );
    //painter.drawLine( W2, 0, W2, H );
    //painter.drawLine( 0, H2, W, H2 );

//    int Ycnt = conv_paint.size();
//    int Xcnt = conv_paint[0].size();

//    float Xstep = W / (float)Xcnt;
//    float Ystep = H / (float)Ycnt;

//    for ( size_t a = 0; a < Ycnt; a++ ) {

//        std::vector<QColor>& raw = conv_paint[a];
//        int Y0 = Ystep * a;
//        int Y1 = Ystep * (a+1);

//        for( size_t p = 0; p < Xcnt; p++ ) {
//            QColor& color = raw[p];
//            painter.setPen( QPen( color, 1, Qt::SolidLine) );
//            painter.setBrush( QBrush( color ) );

//            painter.drawRect( Xstep*p, Y0, Xstep*(p+1), Y1 );
//        }
//    }


    QPoint Center( W2, H2 );
    float R = W < H ? W2 : H2;

    for ( size_t a = 0; a < conv_paint.size(); a++ ) {

        float alpha = a * 2.0 * MY_PI / (float)conv_paint.size();

        std::vector<QColor>& raw = conv_paint[a];

        for( size_t p = 0; p < raw.size(); p++ ) {
            float Z = R * ( p + 5 ) / ((float) raw.size() + 5.0f);
            QColor& color = raw[p];
            painter.setPen( QPen( color, 4, Qt::SolidLine) );
            painter.setBrush( QBrush( color ) );

            painter.drawPoint( Center.x() + Z * cosf(alpha),
                               Center.y() - Z * sinf(alpha) );
        }
    }

}
