#include <QPainter>
#include "spectrumwidget.h"

using namespace std;

SpectrumWidget::SpectrumWidget(QWidget *parent) :
    QWidget(parent),
    powers(NULL)
{
    chan_colors[0] = Qt::blue;
    chan_colors[1] = Qt::red;
    chan_colors[2] = Qt::green;
    chan_colors[3] = Qt::black;
}

void SpectrumWidget::SetPowersData(
        std::vector< std::vector<float> >* powers_data,
        int skip_pts,
        int pts_cnt,
        float min,
        float max,
        float avg )
{
    lock_guard< mutex > lock( mtx );
    this->powers   = powers_data;
    this->skip_pts = skip_pts;
    this->pts_cnt  = pts_cnt;
    this->min = min;
    this->max = max;
    this->avg = avg;
}

void SpectrumWidget::SetCurrentIdx(int idx)
{
    this->idx = idx;
}

int SpectrumWidget::GetCurrentIdx()
{
    return idx;
}

void SpectrumWidget::SetSpectrumParams( double nullHz, double leftHz, double rightHz, double filterHz )
{
    this->nullHz   = nullHz;
    this->leftHz   = leftHz;
    this->rightHz  = rightHz;
    this->filterHz = filterHz;
}

void SpectrumWidget::SetVisualMode(SpectrumWidget::SpecMode_e newmode)
{
    lock_guard<mutex> lock( mtx );
    this->mode = newmode;
}

void SpectrumWidget::PaintHorizontal(QPainter &painter)
{
    const float border = 20.0f;
    float stepX = ( this->width() - border * 2.0f ) / (float)pts_cnt;

    float range = max - min + 5.0f;
    float scale = height() / range;
    float shift = -avg;



    painter.setPen( QPen( Qt::gray, 2, Qt::SolidLine) );
    painter.drawRect( 0, 0, width(), height() );

    QPoint curp( 0, 0 );
    QPoint prvp( 0, 0 );
    float curX = 0;

    float H = height();
    float H2 = H/2.0f;

    int right_point = skip_pts + pts_cnt;
    float choosen = GetCurrentIdx();
    choosen -= skip_pts;

    painter.setPen( QPen( Qt::gray, 1, Qt::DotLine ) );
    painter.drawLine( border + choosen * stepX, 0, border + choosen * stepX, this->height() );


    for ( int ch = 0; ch < 4; ch++ ) {
        painter.setPen( QPen( chan_colors[ ch ], 1, Qt::SolidLine) );

        curX = border;
        curp = QPoint( 0, 0 );
        prvp = QPoint( curX, H2 - ( (*powers)[ ch ][skip_pts] + shift ) * scale );

        for ( int i = skip_pts + 1; i < right_point; i++ ) {
            curp.setX( curX );
            curp.setY( H2 - ( (*powers)[ ch ][i] + shift ) * scale );
            painter.drawLine( prvp, curp );
            prvp = curp;
            curX += stepX;
        }
    }

    painter.setPen( QPen( Qt::black, 2, Qt::SolidLine) );
    for ( float pwr = -200.0f; pwr < 200.0; pwr += 10.0f ) {
        curp.setX( 0 );
        curp.setY( H2 - ( pwr + shift ) * scale );
        painter.drawText( curp, QString(" %1 ").arg(QString::number((int)pwr)) );
    }

    painter.setPen( QPen( Qt::black, 2, Qt::SolidLine) );
    curp.setX( choosen * stepX );
    curp.setY( 15 );
    painter.drawText( curp, QString(" %1 MHz").arg(QString::number(
                                                       ( nullHz - leftHz - choosen*filterHz ) / 1.0e6
                                                       )) );
}

void SpectrumWidget::PaintVertical(QPainter &painter)
{

}

void SpectrumWidget::paintEvent(QPaintEvent *)
{
    lock_guard< mutex > lock( mtx );
    if (!powers) {
        return;
    }
    QPainter painter( this );

    if ( mode == spec_horiz ) {
        PaintHorizontal(painter);
    } else if ( mode == spec_vert ) {
        PaintVertical(painter);
    }
}

