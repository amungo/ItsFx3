#include <QtGui>
#include <QCameraInfo>
#include <QCameraViewfinder>

#include <cmath>
#include "gcacorr/dsp_utils.h"
#include "phaseform.h"
#include "ui_phaseform.h"

#include "gcacorr/etalometr.h"

#define MY_PI (3.14159265359f)

using namespace std;

const float leftMHz  = 10.0f;
const float rightMHz = 20.0f;
const float bandMHz  = 53.0f / 2.0f;

const int fft_len = 4096;
const int half_fft_len = fft_len / 2;

const int left_point  = leftMHz * half_fft_len / bandMHz;
const int right_point = rightMHz * half_fft_len / bandMHz;
const int points_cnt = right_point - left_point;

const int win_cnt = 1;
const int source_len = fft_len * win_cnt;
const int avg_cnt = 20;

const double deg_prec = 1.0;

PhaseForm::PhaseForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PhaseForm),
    camera( NULL ),
    router( NULL ),
    tbuf_powers( fft_len ),
    tbuf_phases( fft_len ),
    fft( fft_len ),
    et( 0.052f )
{
    pphs_valid = false;
    powers.resize(4);
    for ( size_t i = 0; i < powers.size(); i++ ) {
        powers.at(i).resize(half_fft_len);
    }

    phases.resize(4);
    for ( size_t i = 0; i < phases.size(); i++ ) {
        phases.at(i).resize(half_fft_len);
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
    tick_thr = std::thread( &PhaseForm::Tick, this );

    ui->setupUi(this);
    ui->spinBoxChoosenFilter->setMinimum(left_point);
    ui->spinBoxChoosenFilter->setMaximum(right_point);
    ui->spinBoxChoosenFilter->setValue( (left_point + right_point)/2 );

    setStyleSheet("background-color: white;");

    QObject::connect(ui->checkBoxRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );

    InitCamera();
    ui->viewFinder->stackUnder(this);

    et.SetFreq( 1575.42e6 );
    et.SetCalibDefault();
    et.CalcEtalons( deg_prec, 45.0 );
    et.debug();
    ui->widgetConvolution->SetConvolution( et.GetResult() );
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

    if ( camera ) {
        camera->stop();
    }

    delete ui;

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

void PhaseForm::HandleAllChansData( std::vector<short*>& all_ch_data, size_t pts_cnt )
{
    if ( pts_cnt < source_len * avg_cnt ) {
        return;
    }
    if ( all_ch_data.size() != 4 ) {
        return;
    }

    lock_guard< mutex > lock( mtx );
    if ( avg_cnt == 1 ) {

        for ( size_t channel = 0; channel < all_ch_data.size(); channel++ ) {
            fft.TransformShort( all_ch_data[ channel ], fft_out_averaged[ channel ].data() );
        }

    } else {

        for ( int iter = 0; iter < avg_cnt; iter++ ) {
            for ( size_t channel = 0; channel < 4; channel++ ) {
                fft.TransformShort(
                            all_ch_data[ channel ] + fft_len * iter,
                            tbuf_fft[ iter ][ channel ].data()
                            );
            }
        }

        for ( int pt = 0; pt < half_fft_len; pt++ ) {
            float_cpx_t corr[4];
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
                fft_out_averaged[ ch ][ pt ] = corr[ ch ].mul_real( 1.0f / (float) avg_cnt );
            }
        }
    }

    pphs_valid = false;
}

void PhaseForm::MakePphs() {
    lock_guard< mutex > lock( mtx );
    float xavg = 0.0f;

    if ( !pphs_valid ) {
        for ( int ch = 0; ch < 4; ch++ ) {
            const float_cpx_t* avg_data = fft_out_averaged[ ch ].data();
            vector<float>& pwr = powers[ch];
            vector<float>& phs = phases[ch];
            if ( ch == 0 ) {
                for ( int i = left_point; i < right_point; i++ ) {
                    pwr[ i ] = 10.0 * log10( avg_data[i].len_squared() );
                    xavg += pwr[ i ];

                    phs[ i ] = avg_data[i].angle_deg();
                }
            } else {
                vector<float>& phs0 = phases[0];
                for ( int i = left_point; i < right_point; i++ ) {
                    pwr[ i ] = 10.0 * log10( avg_data[i].len_squared() );
                    xavg += pwr[ i ];

                    float x = avg_data[i].angle_deg() - phs0[i];
                    if ( x > 180.0f ) {
                        x -= 360.0f;
                    } else if ( x < -180.0f ) {
                        x += 360.0f;
                    }
                    phs[ i ] = x;
                }
            }
        }
        xavg /= (right_point - left_point) * 4.0f;
        this->powerAvg = xavg;
        pphs_valid = true;
    }
}

void PhaseForm::Tick()
{
    this_thread::sleep_for(chrono::milliseconds(2000));
    while (running) {
        this_thread::sleep_for(chrono::milliseconds(100));
        MakePphs();

        ui->widgetPhases->SetPhasesData(   &phases, left_point, points_cnt );
        ui->widgetSpectrum->SetPowersData( &powers, left_point, points_cnt, powerMin, powerMax, powerAvg );

        ui->widgetPhases->SetCurrentIdx(   GetCurrentIdx() );
        ui->widgetSpectrum->SetCurrentIdx( GetCurrentIdx() );

        int idx = GetCurrentIdx();
        float phs[3];
        phs[0] = phases[1][idx];
        phs[1] = phases[2][idx];
        phs[2] = phases[3][idx];
        ConvResult* result = et.CalcConvolution( phs );
        ui->widgetConvolution->SetConvolution( result );

        update();
    }
}



int PhaseForm::GetCurrentIdx() {
    return ui->spinBoxChoosenFilter->value();
}

void PhaseForm::InitCamera() {
    for (const QCameraInfo &cameraInfo : QCameraInfo::availableCameras()) {
        QString desc = cameraInfo.description();
        fprintf( stderr, "\n*** camera: %s\n", desc.toLatin1().data() );
        camera = new QCamera(cameraInfo);
        break;
    }

    if ( camera ) {
        camera->setViewfinder( ui->viewFinder );
        camera->start();
    } else {
        fprintf( stderr, "\n\n !!!! NO CAMERA FOUND !!!!\n\n" );
    }
}



















