#include <cmath>
#include "phaseform.h"
#include "ui_phaseform.h"

const int fft_len = 2048;
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

    for ( size_t i = 0; i < fft_out.size(); i++ ) {
        fft_out.at( i ) = new Averager<float_cpx_t>( fft_len, avg_cnt );
    }


    ui->setupUi(this);

    QObject::connect(ui->checkBoxRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );
}

PhaseForm::~PhaseForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
    }
    delete ui;

    for ( size_t i = 0; i < fft_out.size(); i++ ) {
        delete fft_out.at( i );
    }
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

    fft_out[ channel ]->PushData( tbuf_fft_out.data() );

    const float_cpx_t* avg_data = fft_out[ channel ]->GetData();
    for ( int i = 0; i < fft_len; i++ ) {
        tbuf_powers[ i ] = 10.0 * log10( avg_data[i].len_squared() );
        tbuf_phases[ i ] = avg_data[i].angle_deg();
    }
}



















