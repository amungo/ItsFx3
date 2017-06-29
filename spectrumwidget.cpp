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
        float minval,
        float maxval,
        float avgval )
{
    lock_guard< mutex > lock( mtx );
    this->powers   = powers_data;
    this->skip_pts = skip_pts;
    this->pts_cnt  = pts_cnt;
    this->minval = minval;
    this->maxval = maxval;
    this->avgval = avgval;

    if ( maxpowers.size() < pts_cnt ) {
        maxpowers.resize( pts_cnt + 1 );
    }

    int right_point = skip_pts + pts_cnt;

    for ( int i = skip_pts; i < right_point; i++ ) {
        float ch0 = (*powers)[ 0 ][i];
        float ch1 = (*powers)[ 1 ][i];
        float ch2 = (*powers)[ 2 ][i];
        float ch3 = (*powers)[ 3 ][i];
        maxpowers[i-skip_pts] = max( max(ch0, ch1), max(ch2, ch3) );
    }
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
    float stepX = ( this->width() - border * 2.0f ) / (float)pts_cnt;

    float range = maxval - minval + 5.0f;
    float scale = height() / range;
    float shift = -avgval;



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
}

void SpectrumWidget::PaintVertical(QPainter &painter)
{
    painter.translate( 0, height() );
    painter.rotate(-90.0);

    float stepX = ( height() - border * 2.0f ) / (float)pts_cnt;

    float range = maxval - minval + 5.0f;
    float scale = width() / range;
    float shift = -avgval;

    painter.setPen( QPen( Qt::gray, 2, Qt::SolidLine) );
    painter.drawRect( 0, 0, height(), width() );

    QPoint curp( 0, 0 );
    QPoint prvp( 0, 0 );
    float curX = 0;

    float H = width();
    float H2 = H/2.0f;

    float choosen = GetCurrentIdx();
    choosen -= skip_pts;

    painter.setPen( QPen( Qt::gray, 1, Qt::DotLine ) );
    painter.drawLine( border + choosen * stepX, 0, border + choosen * stepX, this->width() );

    painter.setPen( QPen( Qt::green, 1, Qt::SolidLine) );

    curX = border;
    curp = QPoint( 0, 0 );
    prvp = QPoint( curX, H2 - ( maxpowers[0] + shift ) * scale );

    for ( int i = 0; i < pts_cnt; i++ ) {
        curp.setX( curX );
        curp.setY( H2 - ( maxpowers[i] + shift ) * scale );
        painter.drawLine( prvp, curp );
        prvp = curp;
        curX += stepX;
    }

    painter.setPen( QPen( Qt::black, 2, Qt::SolidLine) );
    for ( float pwr = -200.0f; pwr < 200.0; pwr += 10.0f ) {
        curp.setX( 0 );
        curp.setY( H2 - ( pwr + shift ) * scale );
        painter.drawText( curp, QString(" %1 ").arg(QString::number((int)pwr)) );
    }

    painter.rotate(90.0);
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

void SpectrumWidget::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() & Qt::MouseButton::LeftButton ) {

        float y = event->pos().y();
        y -= border;

        float stepX = ( height() - border * 2.0f ) / (float)pts_cnt;
        y /= stepX;
        y = pts_cnt - y;
        y += skip_pts;

        emit sendNewCurIdx( y );
    }
}

