#include <QPainter>
#include "coverwidget.h"

#include <cmath>

using namespace std;

#define MY_PI (3.14159265359f)

CoverWidget::CoverWidget(QWidget *parent) : QWidget(parent)
{

}

void CoverWidget::SetTarget(float alpha_deg, float phi_deg)
{
    lock_guard<mutex> lock(mtx);
    this->alpha_deg = alpha_deg;
    this->phi_deg = phi_deg;
}

void CoverWidget::paintEvent(QPaintEvent *)
{
    lock_guard<mutex> lock(mtx);
    QPainter painter( this );

    float W = width();
    float W2 = W/2.0f;
    float H = height();
    float H2 = H/2.0f;

    painter.setPen( QPen( Qt::gray, 2, Qt::SolidLine) );
    painter.drawRect( 0, 0, W, H );

    painter.setPen( QPen( Qt::green, 1, Qt::DotLine) );
    painter.drawLine( W2, 0, W2, H );

    painter.drawLine( 0, H2, W, H2 );

    QPoint center( W2, H2 );
    QColor color( Qt::red );
    painter.setPen( QPen( color, 4, Qt::SolidLine) );
    painter.setBrush( QBrush( color ) );
    painter.drawEllipse( center, 4, 4 );



    float alpha_rad = (float)( MY_PI *  alpha_deg / 180.0 );
    float R = W < H ? W2 : H2;
    float Z = R * phi_deg / 45.0f;
    QPoint target( center.x() - Z * cosf(alpha_rad),
                   center.y() - Z * sinf(alpha_rad) );


    float perc = 0.05;
    int rad = H*perc;
    color = QColor(128, 128, 255, 128);
    painter.setPen( QPen( color, 1, Qt::SolidLine) );
    painter.setBrush( QBrush(color) );
    painter.drawEllipse( target, rad, rad );
}

