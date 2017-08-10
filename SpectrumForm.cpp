#include <QVariant>
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
    left_point = 1;
    right_point = half_fft_len - 1;
    points_cnt = right_point - left_point - 1;
    nullMHz = cfg->inter_freq_hz / 1.0e6;
    bandMHz = (cfg->adc_sample_rate_hz / 1.0e6) / 2.0;
    filterMHz = bandMHz / ( fft_len );
    band_type = LSB;

    fft = new FFTWrapper( fft_len );

    powers_avg.resize( 4 );
    for ( size_t i = 0; i < powers_avg.size(); i++ ) {
        powers_avg[ i ] = new Averager<float>( half_fft_len, avg_simple_cnt );
    }

    powers.resize(4);
    for ( size_t i = 0; i < powers.size(); i++ ) {
        powers.at(i).resize(half_fft_len);
    }

    powers_avg_safe.resize(4);
    for ( size_t i = 0; i < powers_avg_safe.size(); i++ ) {
        powers_avg_safe.at(i).resize(half_fft_len);
    }

    fft_out_averaged.resize(4);
    for ( size_t i = 0; i < fft_out_averaged.size(); i++ ) {
        fft_out_averaged.at(i).resize(fft_len);
    }

    tbuf_fft.resize(avg_matrix_cnt_max);
    for ( size_t iter = 0; iter < tbuf_fft.size(); iter++ ) {
        tbuf_fft[ iter ].resize( 4 );
        for ( size_t ch = 0; ch < tbuf_fft[ iter ].size(); ch++ ) {
            tbuf_fft[ iter ][ ch ].resize( fft_len );
        }
    }

    running = true;
    calc_thread = std::thread( &SpectrumForm::calc_loop, this );

    ui->setupUi(this);
    ui->widgetSpectrum->SetVisualMode( SpectrumWidget::spec_horiz );
    ui->widgetSpectrum->SetChannelMask( 0xFF );

    checkBoxShowChannels.resize( MAX_CHANS );
    checkBoxShowChannels[ 0 ] = ui->checkBoxShowCh0;
    checkBoxShowChannels[ 1 ] = ui->checkBoxShowCh1;
    checkBoxShowChannels[ 2 ] = ui->checkBoxShowCh2;
    checkBoxShowChannels[ 3 ] = ui->checkBoxShowCh3;

    for ( size_t i = 0; i < checkBoxShowChannels.size(); i++ ) {
        QObject::connect(checkBoxShowChannels[ i ], SIGNAL(stateChanged(int)), this, SLOT(channelsChanged(int)) );
    }

    for ( int i = cfg->chan_count; i < MAX_CHANS; i++ ) {
        checkBoxShowChannels[ i ]->setChecked( false );
        checkBoxShowChannels[ i ]->setEnabled( false );
    }

    ui->comboBoxBandType->insertItem( 0, "LSB", QVariant(LSB));
    ui->comboBoxBandType->insertItem( 1, "USB", QVariant(USB));

    ui->comboBoxAvgMatrix->insertItem( 0, " no corr mtrx", QVariant( 1) );
    ui->comboBoxAvgMatrix->insertItem( 1, " 4x corr mtrx", QVariant( 4) );
    ui->comboBoxAvgMatrix->insertItem( 2, " 8x corr mtrx", QVariant( 8) );
    ui->comboBoxAvgMatrix->insertItem( 3, "10x corr mtrx", QVariant(10) );
    ui->comboBoxAvgMatrix->insertItem( 4, "20x corr mtrx", QVariant(20) );

    ui->comboBoxAvgSimple->insertItem( 0, " no avg", QVariant( 1) );
    ui->comboBoxAvgSimple->insertItem( 1, " 4x avg", QVariant( 4) );
    ui->comboBoxAvgSimple->insertItem( 2, " 8x avg", QVariant( 8) );
    ui->comboBoxAvgSimple->insertItem( 3, "16x avg", QVariant(16) );
    ui->comboBoxAvgSimple->insertItem( 4, "32x avg", QVariant(32) );
    ui->comboBoxAvgSimple->insertItem( 5, "64x avg", QVariant(64) );

    QObject::connect(ui->checkRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );
    QObject::connect(ui->widgetSpectrum, SIGNAL(sendNewCurIdx(int)), this, SLOT(CurChangeOutside(int)) );

    QObject::connect(ui->sliderLevelScale, SIGNAL(valueChanged(int)), this, SLOT(scalesShiftsChanged(int)) );
    QObject::connect(ui->sliderLevelShift, SIGNAL(valueChanged(int)), this, SLOT(scalesShiftsChanged(int)) );
    QObject::connect(ui->sliderFreqScale, SIGNAL(valueChanged(int)), this, SLOT(scalesShiftsChanged(int)) );
    QObject::connect(ui->sliderFreqShift, SIGNAL(valueChanged(int)), this, SLOT(scalesShiftsChanged(int)) );
    QObject::connect(ui->comboBoxBandType, SIGNAL(currentIndexChanged(int)), this, SLOT(bandTypeChanged(int)));
    QObject::connect(ui->comboBoxAvgMatrix, SIGNAL(currentIndexChanged(int)), this, SLOT(avgMatrixChanged(int)));
    QObject::connect(ui->comboBoxAvgSimple, SIGNAL(currentIndexChanged(int)), this, SLOT(avgSimpleChanged(int)));

    SetCurrentIdx( ( right_point - left_point ) / 2 );
    ChangeNullMhz( nullMHz * 1.0e6 );
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

    for ( size_t i = 0; i < powers_avg.size(); i++ ) {
        if ( powers_avg[ i ] ) {
            delete powers_avg[ i ];
        }
    }

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
    if ( avg_matrix_cnt == 1 ) {

        for ( size_t channel = 0; channel < all_ch_data.size(); channel++ ) {
            fft->TransformShort( all_ch_data[ channel ].data(), fft_out_averaged[ channel ].data(), false );
        }

    } else {

        for ( int iter = 0; iter < avg_matrix_cnt; iter++ ) {
            for ( size_t channel = 0; channel < 4; channel++ ) {
                fft->TransformShort(
                            all_ch_data[ channel ].data() + fft_len * iter,
                            tbuf_fft[ iter ][ channel ].data(),
                            false
                            );
            }
        }

        float scale = 1.0f / ((float) avg_matrix_cnt * 4.0f);
        float_cpx_t corr[4];
        for ( int pt = 0; pt < half_fft_len; pt++ ) {
            corr[0] = float_cpx_t( 0.0f, 0.0f );
            corr[1] = float_cpx_t( 0.0f, 0.0f );
            corr[2] = float_cpx_t( 0.0f, 0.0f );
            corr[3] = float_cpx_t( 0.0f, 0.0f );

            for ( int iter = 0; iter < avg_matrix_cnt; iter++ ) {
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
    //float xavg = 0.0f;
    //float xmax = -1000.0f;
    //float xmin = 1000.0f;

    int left_point_copy, right_point_copy;
    {
        lock_guard<mutex> lock( pts_param_mtx );
        left_point_copy  = left_point;
        right_point_copy = right_point;
    }

    float koef = 10.0;
    if ( avg_matrix_cnt > 1 ) {
        koef = 5.0;
    }

    for ( int ch = 0; ch < 4; ch++ ) {
        const float_cpx_t* avg_data = fft_out_averaged[ ch ].data();
        vector<float>& pwr = powers[ch];
        //for ( int i = left_point_copy; i < right_point_copy; i++ ) {
        for ( int i = 0; i < half_fft_len; i++ ) {
            float p = koef * log10f( avg_data[i].len_squared() );
            pwr[ i ] = p;

            //xavg += p;
            //if ( p > xmax ) { xmax = p; }
            //if ( p < xmin ) { xmin = p; }

        }

        lock_guard<mutex> lock(powers_avg_mtx);
        if ( powers_avg[ ch ] ) {
            powers_avg[ ch ]->PushData( pwr.data() );
        }
    }
    //xavg /= (left_point_copy - right_point_copy) * 4.0f;
    //this->powerAvg = xavg;
    //this->powerMax = xmax;
    //this->powerMin = xmin;
}

void SpectrumForm::SetWidgetData()
{
    scalesShiftsChanged(0);
    int left_point_copy, points_cnt_copy;
    {
        lock_guard<mutex> lock( pts_param_mtx );
        left_point_copy  = left_point;
        points_cnt_copy  = points_cnt;
    }

    {
        lock_guard<mutex> lock(powers_avg_mtx);
        for ( int ch = 0; ch < 4; ch++ ) {
            if ( powers_avg[ ch ] ) {
                powers_avg[ ch ]->GetData( powers_avg_safe[ ch ].data(), band_type == LSB );
            }
        }
    }

    ui->widgetSpectrum->SetPowersData(
        &powers_avg_safe, left_point_copy, points_cnt_copy, -40.0, 80.0, 10.0, 20.0, 100.0 );

    ui->widgetSpectrum->SetCurrentIdx( GetCurrentIdx(), 10.0 );
}

double SpectrumForm::GetCurrentFreqHz()
{
    if ( band_type == LSB ) {
        return ( nullMHz - bandMHz + curIdx*filterMHz*2.0 ) * 1.0e6;
    } else {
        return ( nullMHz + curIdx*filterMHz*2.0 ) * 1.0e6;
    }
}

int SpectrumForm::GetCurrentIdx()
{
    return curIdx;
}

void SpectrumForm::SetCurrentIdx(int x)
{
    {
        lock_guard<mutex> lock( pts_param_mtx );
        if ( x < left_point ) {
            x = left_point;
        }
        if ( x > right_point ) {
            x = right_point;
        }

        curIdx = x;
    }
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
    if ( !ui->checkRun->isChecked() ) {
        ui->widgetSpectrum->SetCurrentIdx( GetCurrentIdx(), 10.0 );
        update();
    }
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

void SpectrumForm::channelsChanged(int)
{
    uint32_t chanmask = 0;
    for ( size_t i = 0; i < checkBoxShowChannels.size(); i++ ) {
        if ( checkBoxShowChannels[ i ]->isChecked() ) {
            chanmask |= ( 1 << i );
        }
    }
    ui->widgetSpectrum->SetChannelMask( chanmask );
    if ( !ui->checkRun->isChecked() ) {
        update();
    }
}

void SpectrumForm::scalesShiftsChanged(int)
{
    float yscale = 80.0f * ( (float)ui->sliderLevelScale->value() / (float)ui->sliderLevelScale->maximum() );
    ui->widgetSpectrum->SetPowerRange(yscale);

    float yshift = -(float)ui->sliderLevelShift->value();
    ui->widgetSpectrum->SetPowerShift( yshift );
    update();

    float band = half_fft_len * ( (float)ui->sliderFreqScale->value() / (float)ui->sliderFreqScale->maximum() );

    float freq_shift = (float)half_fft_len * (float)ui->sliderFreqShift->value() / 200.0f;
    float center = ((float)half_fft_len) / 2.0f + freq_shift;

    {
        lock_guard<mutex> lock( pts_param_mtx );
        left_point  = (int) round( center - band / 2.0f );
        right_point = (int) round( center + band / 2.0f );

        if ( left_point < 0 ) {
            left_point = 0;
        }

        if ( right_point > half_fft_len ) {
            right_point = half_fft_len - 1;
        }

        if ( left_point > right_point ) {
            left_point = right_point - 1;
        }

        points_cnt = right_point - left_point - 1;
    }
    ui->widgetSpectrum->SetPointsParams( left_point, points_cnt );
}

void SpectrumForm::bandTypeChanged(int)
{
    bool ok;
    BandType new_band_type = (BandType)ui->comboBoxBandType->currentData().toInt(&ok);
    if ( new_band_type != band_type && !ui->checkRun->isChecked() ) {
        band_type = new_band_type;
        SetWidgetData();
    }
    band_type = new_band_type;
    SetCurrentIdx( GetCurrentIdx() );
    update();
}

void SpectrumForm::avgSimpleChanged(int)
{
    bool ok;
    int newval = ui->comboBoxAvgSimple->currentData().toInt(&ok);
    if ( newval != avg_simple_cnt ) {
        lock_guard<mutex> lock( powers_avg_mtx );
        avg_simple_cnt = newval;
        for ( size_t i = 0; i < powers_avg.size(); i++ ) {
            if ( powers_avg[ i ] ) {
                delete powers_avg[ i ];
            }
            powers_avg[ i ] = new Averager<float>( half_fft_len, avg_simple_cnt );
        }
    }
}

void SpectrumForm::avgMatrixChanged(int)
{
    bool ok;
    avg_matrix_cnt = ui->comboBoxAvgMatrix->currentData().toInt(&ok);
}

void SpectrumForm::hideEvent(QHideEvent* /*event*/ ) {
    ui->checkRun->setChecked(false);
}

void SpectrumForm::HandleAllChansData(std::vector<short *> &new_all_ch_data, size_t pts_cnt)
{
    if ( (int)pts_cnt < fft_len ) {
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



