#include <QtGui>
#include <QPainter>

#include <cmath>
#include "phaseform.h"
#include "ui_phaseform.h"

using namespace std;

const int fft_len = 512;
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
    newTick = true;
    powers.resize(4);
    for ( size_t i = 0; i < powers.size(); i++ ) {
        powers.at(i).resize(fft_len);
    }

    phases.resize(4);
    for ( size_t i = 0; i < phases.size(); i++ ) {
        phases.at(i).resize(fft_len);
    }

    for ( size_t i = 0; i < fft_out.size(); i++ ) {
        fft_out.at( i ) = new Averager<float_cpx_t>( fft_len, avg_cnt );
    }

    running = true;
    tick_thr = std::thread( &PhaseForm::Tick, this );

    ui->setupUi(this);

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
            for ( int i = 0; i < fft_len; i++ ) {
                pwr[ i ] = 10.0 * log10( avg_data[i].len_squared() );
                phs[ i ] = avg_data[i].angle_deg();
            }
        }
        pphs_valid = true;
    }
}

void PhaseForm::Tick()
{
    while (running) {
        this_thread::sleep_for(chrono::milliseconds(200));
        newTick = true;
        update();
    }
}


void PhaseForm::PaintPowers() {
    float scalePowers = -2.0f;
    float shiftPowers = 200.0f;
    float stepPowers = 2.0f;

    if ( newTick ) {
        newTick = false;
        MakePphs();
    }

    QPainter painter( this );

    float curX = stepPowers;
    QPoint curp( 0, 0 );
    QPoint prvp( 2, powers[0][0] * scalePowers + shiftPowers );
    printf( "%f %f %f %f\n", powers[0][0], powers[0][1], powers[0][2], powers[0][10] );

    for ( int i = 1; i < fft_len / 2; i++ ) {
        curp.setX( curX );
        curp.setY( powers[0][i] * scalePowers + shiftPowers );
        painter.drawLine( prvp, curp );
        prvp = curp;
        curX += stepPowers;
    }

}



















