#include <QtGui>
#include <QPainter>

#include <cmath>
#include "phaseform.h"
#include "ui_phaseform.h"

using namespace std;

const int fft_len = 512;
const int visible_len = fft_len / 2;
const int win_cnt = 1;
const int source_len = fft_len * win_cnt;
const int avg_cnt = 10;

PhaseForm::PhaseForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PhaseForm),
    router( NULL ),
    fft_out( fft_len ),
    tbuf_fft_out( fft_len ),
    tbuf_powers( fft_len ),
    tbuf_phases( fft_len ),
    fft( fft_len )
{
    pphs_valid = false;
    powers.resize(4);
    for ( size_t i = 0; i < powers.size(); i++ ) {
        powers.at(i).resize(visible_len);
    }

    phases.resize(4);
    for ( size_t i = 0; i < phases.size(); i++ ) {
        phases.at(i).resize(visible_len);
    }

    for ( size_t i = 0; i < fft_out.size(); i++ ) {
        fft_out.at( i ) = new Averager<float_cpx_t>( fft_len, avg_cnt );
    }

    chan_colors[0] = Qt::blue;
    chan_colors[1] = Qt::red;
    chan_colors[2] = Qt::green;
    chan_colors[3] = Qt::black;

    running = true;
    tick_thr = std::thread( &PhaseForm::Tick, this );

    ui->setupUi(this);

    setStyleSheet("background-color: white;");

    QObject::connect(ui->checkBoxRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );
}

PhaseForm::~PhaseForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
    }

    running = false;
    if ( tick_thr.joinable() ) {
        tick_thr.join();
    }

    delete ui;

    for ( size_t i = 0; i < fft_out.size(); i++ ) {
        delete fft_out.at( i );
    }
}

void PhaseForm::paintEvent(QPaintEvent *event) {
    PaintPowers();
}

void PhaseForm::slotRun(int state)
{
    if ( router ) {
        if ( state ) {
            router->AddOutPoint(this);
        } else {
            router->DeleteOutPoint(this);
        }
    }
}

void PhaseForm::HandleADCStreamData(void *, size_t)
{
    // nop
}

void PhaseForm::HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel)
{
    //printf( "ch%d=%u  ", channel, pts_cnt );
    if ( pts_cnt < source_len ) {
        return;
    }

    // TODO: make window here

    fft.TransformShort( one_ch_data, tbuf_fft_out.data() );

    lock_guard< mutex > lock( mtx );
    pphs_valid = false;
    fft_out[ channel ]->PushData( tbuf_fft_out.data() );
}

void PhaseForm::MakePphs() {
    lock_guard< mutex > lock( mtx );
    if ( !pphs_valid ) {
        for ( int ch = 0; ch < 4; ch++ ) {
            const float_cpx_t* avg_data = fft_out[ ch ]->GetData();
            vector<float>& pwr = powers[ch];
            vector<float>& phs = phases[ch];
            if ( ch == 0 ) {
                for ( int i = 0; i < visible_len; i++ ) {
                    pwr[ i ] = 10.0 * log10( avg_data[i].len_squared() );
                    phs[ i ] = avg_data[i].angle_deg();
                }
            } else {
                vector<float>& phs0 = phases[0];
                for ( int i = 0; i < visible_len; i++ ) {
                    pwr[ i ] = 10.0 * log10( avg_data[i].len_squared() );
                    phs[ i ] = avg_data[i].angle_deg() - phs0[i];
                }
            }
        }
        pphs_valid = true;
    }
}

void PhaseForm::Tick()
{
    while (running) {
        this_thread::sleep_for(chrono::milliseconds(100));
        MakePphs();
        update();
    }
}


void PhaseForm::PaintPowers() {
    //printf( "%f %f %f %f\n", phases[0][0], phases[0][1], phases[0][2], phases[0][10] );

    float shiftPowers = 200.0f;
    float scalePowers = -3.0f;

    float shiftPhases = 400.0f;
    float scalePhases = 0.5f;

    float stepX = 4.0f;

    QPainter painter( this );

    painter.setPen( QPen( chan_colors[ 0 ], 1, Qt::SolidLine) );

    float curX = stepX;
    QPoint curp( 0, 0 );
    QPoint prvp( 2, powers[0][0] * scalePowers + shiftPowers );

    for ( int i = 1; i < visible_len; i++ ) {
        curp.setX( curX );
        curp.setY( powers[0][i] * scalePowers + shiftPowers );
        painter.drawLine( prvp, curp );
        prvp = curp;
        curX += stepX;
    }


    for ( int ch = 1; ch < 4; ch++ ) {
        painter.setPen( QPen( chan_colors[ ch ], 2, Qt::SolidLine) );
        curX = stepX;
        for ( int i = 0; i < visible_len; i++ ) {
            curp.setX( curX );
            curp.setY( phases[ch][i] * scalePhases + shiftPhases );
            painter.drawPoint( curp );
            curX += stepX;
        }
    }
}



















