#include <QPainter>
#include "phasewidget.h"

using namespace std;

PhaseWidget::PhaseWidget(QWidget *parent) :
    QWidget(parent),
    phases(NULL)
{
    chan_colors[0] = Qt::blue;
    chan_colors[1] = Qt::red;
    chan_colors[2] = QColor( 18, 92, 40, 255 );
    chan_colors[3] = Qt::black;
}

void PhaseWidget::SetPhasesData(std::vector<std::vector<float> > *phases_data, int skip_pts, int pts_cnt)
{
    lock_guard< mutex > lock( mtx );
    this->phases   = phases_data;
    this->skip_pts = skip_pts;
    this->pts_cnt  = pts_cnt;
}

void PhaseWidget::SetCurrentIdx(int idx, int band)
{
    this->idx = idx;
    this->idxBand = band;
}

int PhaseWidget::GetCurrentIdx()
{
    return idx;
}

void PhaseWidget::paintEvent(QPaintEvent *) {
    lock_guard< mutex > lock( mtx );
    if (!phases) {
        return;
    }

    const float border = 20.0f;
    float stepX = ( this->width() - border * 2.0f ) / (float)pts_cnt;
    int right_point = skip_pts + pts_cnt;

    float range = 390.0f;
    float scalePhases = height() / range;

    QPainter painter( this );

    painter.setPen( QPen( Qt::gray, 2, Qt::SolidLine) );
    painter.drawRect( 0, 0, width(), height() );

    QPoint curp( 0, 0 );
    float curX = 0;


    float choosen = GetCurrentIdx();
    choosen -= skip_pts;

    float H = height();
    float H2 = H/2.0f;

    int curIdx = GetCurrentIdx();
    for ( int ch = 1; ch < 4; ch++ ) {
        painter.setPen( QPen( chan_colors[ ch ], 2, Qt::SolidLine) );
        curX = border;
        for ( int i = skip_pts; i < right_point; i++ ) {
            curp.setX( curX );
            curp.setY( H2 - (*phases)[ch][i] * scalePhases );
            painter.drawPoint( curp );

            if ( i == curIdx ) {
                curp.setX( curp.x() + 20 * ch );
                curp.setY( curp.y() - 10 );
                painter.drawText( curp, QString(" %1 ").arg(QString::number((int)(*phases)[ch][i])) );
            }

            curX += stepX;
        }        
    }

    painter.setPen(   QPen(   QColor( 64, 64, 64, 255), 1, Qt::DotLine ) );
    painter.setBrush( QBrush( QColor( 64, 64, 64, 64 ) ) );
    painter.drawRect( border + ( choosen - idxBand/2 )* stepX, 0, idxBand * stepX, this->height() );


    painter.setPen( QPen( Qt::black, 4, Qt::SolidLine) );
    for ( float angle = -180.0f; angle < 181.0; angle += 45.0f ) {
        curp.setX( 0 );
        curp.setY( H2  - angle * scalePhases );
        painter.drawText( curp, QString(" %1 ").arg(QString::number((int)angle)) );
    }

}

