#include "gcacorr/dsp_utils.h"
#include "SpectrumForm.h"
#include "ui_SpectrumForm.h"

using namespace std;

SpectrumForm::SpectrumForm( FX3Config* cfg, QWidget *parent ) :
    QWidget(parent),
    router( NULL ),
    ui(new Ui::SpectrumForm),
    cfg( cfg ),
    fft( NULL ),
    fft_len( nFftDefault ),
    half_fft_len( fft_len/2 )
{
    left_point = 0;
    right_point = half_fft_len;
    points_cnt = right_point - left_point - 1;
    filterMHz = bandMHz / ( fft_len );

    fft = new FFTWrapper( fft_len );

    powers.resize(4);
    for ( size_t i = 0; i < powers.size(); i++ ) {
        powers.at(i).resize(half_fft_len);
    }

    fft_out_averaged.resize(4);
    for ( size_t i = 0; i < fft_out_averaged.size(); i++ ) {
        fft_out_averaged.at(i).resize(fft_len);
    }

    tbuf_fft.resize(avg_cnt);
    for ( size_t iter = 0; iter < tbuf_fft.size(); iter++ ) {
        tbuf_fft[ iter ].resize( 4 );
        for ( size_t ch = 0; ch < tbuf_fft[ iter ].size(); ch++ ) {
            tbuf_fft[ iter ][ ch ].resize( fft_len );
        }
    }

    running = true;
    calc_thread = std::thread( &SpectrumForm::calc_loop, this );

    ui->setupUi(this);

    checkBoxShowChannels.resize( MAX_CHANS );
    checkBoxShowChannels[ 0 ] = ui->checkBoxShowCh0;
    checkBoxShowChannels[ 1 ] = ui->checkBoxShowCh1;
    checkBoxShowChannels[ 2 ] = ui->checkBoxShowCh2;
    checkBoxShowChannels[ 3 ] = ui->checkBoxShowCh3;
    for ( int i = cfg->chan_count; i < MAX_CHANS; i++ ) {
        checkBoxShowChannels[ i ]->setChecked( false );
        checkBoxShowChannels[ i ]->setEnabled( false );
    }

    QObject::connect(ui->checkRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );
    QObject::connect(ui->widgetSpectrum, SIGNAL(sendNewCurIdx(int)), this, SLOT(CurChangeOutside(int)) );

    SetCurrentIdx( ( right_point - left_point ) / 2 );
}

SpectrumForm::~SpectrumForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
    }

    running = false;
    data_valid = false;
    event_data.Notify();
    if ( calc_thread.joinable() ) {
        calc_thread.join();
    }


    delete fft;
    delete ui;
}

bool SpectrumForm::TryLockData()
{
    lock_guard<mutex> lock(mtx_data);
    if ( data_is_busy ) {
        return false;
    } else {
        data_is_busy = true;
        return true;
    }
}

void SpectrumForm::UnlockData()
{
    lock_guard<mutex> lock(mtx_data);
    data_is_busy = false;
}


void SpectrumForm::MakeFFTs()
{
    if ( avg_cnt == 1 ) {

        for ( size_t channel = 0; channel < all_ch_data.size(); channel++ ) {
            fft->TransformShort( all_ch_data[ channel ].data(), fft_out_averaged[ channel ].data() );
        }

    } else {

        for ( int iter = 0; iter < avg_cnt; iter++ ) {
            for ( size_t channel = 0; channel < 4; channel++ ) {
                fft->TransformShort(
                            all_ch_data[ channel ].data() + fft_len * iter,
                            tbuf_fft[ iter ][ channel ].data()
                            );
            }
        }

        float scale = 1.0f / ((float) avg_cnt * 4.0f);
        float_cpx_t corr[4];
        for ( int pt = 0; pt < half_fft_len; pt++ ) {
            corr[0] = float_cpx_t( 0.0f, 0.0f );
            corr[1] = float_cpx_t( 0.0f, 0.0f );
            corr[2] = float_cpx_t( 0.0f, 0.0f );
            corr[3] = float_cpx_t( 0.0f, 0.0f );

            for ( int iter = 0; iter < avg_cnt; iter++ ) {
                for ( int ch = 0; ch < 4; ch++ ) {
                    corr[ch].add(
                        calc_correlation(
                            tbuf_fft[ iter ][ 0  ][ pt ],
                            tbuf_fft[ iter ][ ch ][ pt ]
                        )
                    );
                }
            }

            for ( int ch = 0; ch < 4; ch++ ) {
                fft_out_averaged[ ch ][ pt ] = corr[ ch ].mul_real( scale );
            }
        }
    }
    data_valid = false;
}

void SpectrumForm::MakePowers()
{
    float xavg = 0.0f;
    float xmax = -1000.0f;
    float xmin = 1000.0f;

    for ( int ch = 0; ch < 4; ch++ ) {
        const float_cpx_t* avg_data = fft_out_averaged[ ch ].data();
        vector<float>& pwr = powers[ch];
        for ( int i = left_point; i < right_point; i++ ) {
            float p = 5.0f * log10f( avg_data[i].len_squared() );
            pwr[ i ] = p;

            xavg += p;
            if ( p > xmax ) { xmax = p; }
            if ( p < xmin ) { xmin = p; }

        }
    }
    xavg /= (right_point - left_point) * 4.0f;
    //this->powerAvg = xavg;
    //this->powerMax = xmax;
    //this->powerMin = xmin;
}

void SpectrumForm::SetWidgetData()
{
    ui->widgetSpectrum->SetPowersData(
        &powers, left_point, points_cnt, -40.0, 80.0, 10.0, 20.0, 100.0 );
    //  &powers, left_point, points_cnt, powerMin, powerMax, powerAvg, powerMaxCur, GetThreshold()

    ui->widgetSpectrum->SetCurrentIdx( GetCurrentIdx(), 10.0 );
}

double SpectrumForm::GetCurrentFreqHz()
{
    return ( nullMHz - bandMHz + curIdx*filterMHz*2.0 ) * 1.0e6;
}

int SpectrumForm::GetCurrentIdx()
{
    return curIdx;
}

void SpectrumForm::SetCurrentIdx(int x)
{
    if ( x < left_point ) {
        x = left_point;
    }
    if ( x > right_point ) {
        x = right_point;
    }

    curIdx = x;
    ui->labelFreq->setText( QString("   %1 MHz").arg( QString::number(
        GetCurrentFreqHz() / 1.0e6, 'f', 2  ) ));
}

void SpectrumForm::ChangeNullMhz(double newVal)
{
    this->nullMHz = newVal/1.0e6;
    SetCurrentIdx( GetCurrentIdx() );
}

void SpectrumForm::CurChangeOutside(int value)
{
    SetCurrentIdx( value );
}

void SpectrumForm::calc_loop()
{
    this_thread::sleep_for(chrono::milliseconds(2000));
    while (running) {
        event_data.WaitAndFlush();

        if ( TryLockData() ) {
            if ( data_valid ) {
                // have new data

                MakeFFTs();
                UnlockData();
                MakePowers();
                SetWidgetData();
                update();

            } else {
                // no new data
                UnlockData();
            }

        } // if TryLockData()

    } // while running
}

void SpectrumForm::slotRun(int state) {
    if ( router ) {
        if ( state ) {
            router->AddOutPoint(this);
        } else {
            router->DeleteOutPoint(this);
        }
    }

}

void SpectrumForm::hideEvent(QHideEvent* /*event*/ ) {
    ui->checkRun->setChecked(false);
}

void SpectrumForm::HandleAllChansData(std::vector<short *> &new_all_ch_data, size_t pts_cnt)
{
    if ( pts_cnt < fft_len ) {
        return;
    }
    if ( new_all_ch_data.size() != 4 ) {
        return;
    }

    if ( TryLockData() ) {
        if ( all_ch_data.size() != 4 ) {
            all_ch_data.resize(4);
        }
        for ( int ch = 0; ch < 4; ch++ ) {
            if ( all_ch_data[ch].size() != pts_cnt ) {
                all_ch_data[ch].resize(pts_cnt);
            }
            memcpy( all_ch_data[ch].data(), new_all_ch_data[ch], sizeof(short)*pts_cnt);
        }
        data_valid = true;
        UnlockData();
        event_data.Notify();
    }
}



