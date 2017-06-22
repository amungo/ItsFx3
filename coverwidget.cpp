#include <QPainter>
#include "coverwidget.h"

CoverWidget::CoverWidget(QWidget *parent) : QWidget(parent)
{

}

void CoverWidget::paintEvent(QPaintEvent *)
{
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

    float perc = 0.05;
    int rad = H*perc;
    color = QColor(128, 128, 255, 32);
    painter.setPen( QPen( color, 1, Qt::SolidLine) );
    painter.setBrush( QBrush(color) );
    painter.drawEllipse( center, rad, rad );
}

