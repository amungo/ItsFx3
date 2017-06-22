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
}

