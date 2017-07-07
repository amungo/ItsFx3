#include <QPainter>
#include "spectrumwidget.h"

using namespace std;

SpectrumWidget::SpectrumWidget(QWidget *parent) :
    QWidget(parent),
    powers(NULL)
{
    chan_colors[0] = Qt::blue;
    chan_colors[1] = Qt::red;
    chan_colors[2] = QColor( 18, 92, 40, 255 );
    chan_colors[3] = Qt::black;
}

void SpectrumWidget::SetPowersData(
        std::vector< std::vector<float> >* powers_data,
        int skip_pts,
        int pts_cnt,
        float minval,
        float maxval,
        float avgval,
        float maxval_cur )
{
    lock_guard< mutex > lock( mtx );
    this->powers   = powers_data;
    this->skip_pts = skip_pts;
    this->pts_cnt  = pts_cnt;
    this->minval = minval;
    this->maxval = maxval;
    this->avgval = avgval;
    this->maxval_cur = maxval_cur;

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
    //fprintf( stderr, "%3.0f %3.0f %3.0f %3.0f\n", minval, avgval, maxval, maxval_cur );
}

void SpectrumWidget::SetCurrentIdx(int idx, int band)
{
    this->idx = idx;
    this->idxBand = band;
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

    float range = 40.0f;
    float scale = height() / range;
    float shift = -20.0f;



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

    for ( int ch = 0; ch < 4; ch++ ) {
        painter.setPen( QPen( chan_colors[ ch ], 1, Qt::SolidLine) );

        curX = border;
        curp = QPoint( 0, 0 );
        prvp = QPoint( curX, H - ( (*powers)[ ch ][skip_pts] + shift ) * scale );

        for ( int i = skip_pts + 1; i < right_point; i++ ) {
            curp.setX( curX );
            curp.setY( H - ( (*powers)[ ch ][i] + shift ) * scale );
            painter.drawLine( prvp, curp );
            prvp = curp;
            curX += stepX;
        }
    }

    painter.setPen(   QPen(   QColor( 64, 64, 64, 255), 1, Qt::DotLine ) );
    painter.setBrush( QBrush( QColor( 64, 64, 64, 64 ) ) );
    painter.drawRect( border + ( choosen - idxBand/2 )* stepX, 0, idxBand * stepX, this->height() );



    painter.setPen( QPen( Qt::black, 2, Qt::SolidLine) );
    for ( float pwr = -40.0f; pwr < 120.0; pwr += 10.0f ) {
        curp.setX( 0 );
        curp.setY( H - ( pwr + shift ) * scale );
        painter.drawText( curp, QString(" %1 ").arg(QString::number((int)pwr)) );
    }
}

void SpectrumWidget::PaintVertical(QPainter &painter)
{
    painter.translate( 0, height() );
    painter.rotate(-90.0);

    float stepX = ( height() - border * 2.0f ) / (float)pts_cnt;

    float range = 40.0f;
    float scale = width() / range;
    float shift = -20.0f;

    painter.setPen( QPen( Qt::gray, 2, Qt::SolidLine) );
    painter.drawRect( 0, 0, height(), width() );

    QPoint curp( 0, 0 );
    QPoint prvp( 0, 0 );
    float curX = 0;

    float H = width();
    float H2 = H/2.0f;

    float choosen = GetCurrentIdx();
    choosen -= skip_pts;

    painter.setPen( QPen( chan_colors[2], 1, Qt::SolidLine) );

    curX = border;
    curp = QPoint( 0, 0 );
    prvp = QPoint( curX, H - ( maxpowers[0] + shift ) * scale );

    for ( int i = 0; i < pts_cnt; i++ ) {
        curp.setX( curX );
        curp.setY( H - ( maxpowers[i] + shift ) * scale );
        painter.drawLine( prvp, curp );
        prvp = curp;
        curX += stepX;
    }

    int colshade = 64 + ( maxval_cur - 10.0f ) * 2;
    painter.setPen(   QPen(   QColor( colshade, 0, 0, colshade ), 1, Qt::SolidLine ) );
    painter.setBrush( QBrush( QColor( colshade, 0, 0, colshade ) ) );
    painter.drawRect( 0, 1, ( maxval_cur - 10.0f ) * (height() / 60.0f), 30 );

    painter.setPen(   QPen(   QColor( 64, 64, 64, 255), 1, Qt::DotLine ) );
    painter.setBrush( QBrush( QColor( 64, 64, 64, 64 ) ) );
    painter.drawRect( border + ( choosen - idxBand/2 )* stepX, 0, idxBand * stepX, this->width() );

    painter.setPen( QPen( Qt::black, 2, Qt::SolidLine) );
    for ( float pwr = -40.0f; pwr < 120.0; pwr += 10.0f ) {
        curp.setX( 0 );
        curp.setY( H - ( pwr + shift ) * scale );
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

        if ( mode == spec_horiz ) {
            float x = event->pos().x();
            x -= border;

            float stepX = ( width() - border * 2.0f ) / (float)pts_cnt;
            x /= stepX;
            x += skip_pts;

            emit sendNewCurIdx( x );

        } else if ( mode == spec_vert ) {
            float y = event->pos().y();
            y -= border;

            float stepX = ( height() - border * 2.0f ) / (float)pts_cnt;
            y /= stepX;
            y = pts_cnt - y;
            y += skip_pts;

            emit sendNewCurIdx( y );
        }
    }
}

