#include <QtGui>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QMessageBox>

#include <cmath>
#include "gcacorr/dsp_utils.h"
#include "phaseform.h"
#include "ui_phaseform.h"

#include "gcacorr/etalometrgeo.h"
#include "gcacorr/etalometrfile.h"

#define MY_PI (3.14159265359f)

using namespace std;

const float nullMHz = 1590.0f;

const float leftMHz  = 10.0f;
const float rightMHz = 20.0f;
const float bandMHz  = 53.0f / 2.0f;

const int fft_len = 4096;
const int half_fft_len = fft_len / 2;
const double filterMHz = bandMHz / ( fft_len );

const int left_point  = leftMHz * half_fft_len / bandMHz;
const int right_point = rightMHz * half_fft_len / bandMHz;
const int points_cnt = right_point - left_point;

const int win_cnt = 1;
const int source_len = fft_len * win_cnt;
const int avg_cnt = 20;

const int deg_prec = 1;
const double deg_wide_X = 45.0;
const double deg_wide_Y = 30.0;

PhaseForm::PhaseForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PhaseForm),
    camera( NULL ),
    router( NULL ),
    tbuf_powers( fft_len ),
    tbuf_phases( fft_len ),
    fft( fft_len )
{
    data_valid = false;
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

    ui->radioButtonEtalonsGeo->setChecked(true);
    ui->radioButtonEtalonsFile->setChecked(false);

    QObject::connect(ui->widgetSpectrumVertical, SIGNAL(sendNewCurIdx(int)), this, SLOT(CurChangeOutside(int)) );
    QObject::connect(ui->widgetSpectrum,         SIGNAL(sendNewCurIdx(int)), this, SLOT(CurChangeOutside(int)) );

    QObject::connect(ui->pushButtonUp,         SIGNAL(clicked(bool)),     this, SLOT(CurChangeButtonUpSlow(bool)) );
    QObject::connect(ui->pushButtonUpFast,     SIGNAL(clicked(bool)),     this, SLOT(CurChangeButtonUpFast(bool)) );
    QObject::connect(ui->pushButtonDown,       SIGNAL(clicked(bool)),     this, SLOT(CurChangeButtonDownSlow(bool)) );
    QObject::connect(ui->pushButtonDownFast,   SIGNAL(clicked(bool)),     this, SLOT(CurChangeButtonDownFast(bool)) );

    QObject::connect(ui->pushButtonAvgBandUp,   SIGNAL(clicked(bool)), this, SLOT(CurBandChangeUp(bool)) );
    QObject::connect(ui->pushButtonAvgBandDown, SIGNAL(clicked(bool)), this, SLOT(CurBandChangeDown(bool)) );

    QObject::connect(ui->pushButtonCalibrate,         SIGNAL(clicked(bool)), this, SLOT(CalibrateApplyPhases(bool)) );
    QObject::connect(ui->pushButtonCalibrateDefault,  SIGNAL(clicked(bool)), this, SLOT(CalibrateDefault(bool)) );

    QObject::connect(ui->radioButtonEtalonsGeo,  SIGNAL(clicked(bool)), this, SLOT(ChangeEtalons(bool)) );
    QObject::connect(ui->radioButtonEtalonsFile, SIGNAL(clicked(bool)), this, SLOT(ChangeEtalons(bool)) );

    ui->pushButtonUp->setStyleSheet(      "background-color: lightGrey");
    ui->pushButtonUpFast->setStyleSheet(  "background-color: lightGrey");
    ui->pushButtonDown->setStyleSheet(    "background-color: lightGrey");
    ui->pushButtonDownFast->setStyleSheet("background-color: lightGrey");

    ui->pushButtonAvgBandUp->setStyleSheet(    "background-color: lightGrey");
    ui->pushButtonAvgBandDown->setStyleSheet(  "background-color: lightGrey");

    ui->pushButtonCalibrate->setStyleSheet(         "background-color: grey");
    ui->pushButtonCalibrateDefault->setStyleSheet(  "background-color: grey");

    ui->widgetSpectrum->SetVisualMode( SpectrumWidget::spec_horiz );
    ui->widgetSpectrum->SetSpectrumParams( nullMHz, leftMHz * 1e6, rightMHz * 1e6, filterMHz * 1e6 );

    ui->widgetSpectrumVertical->SetVisualMode( SpectrumWidget::spec_vert );
    ui->widgetSpectrumVertical->SetSpectrumParams( nullMHz, leftMHz * 1e6, rightMHz * 1e6, filterMHz * 1e6 );

    setStyleSheet("background-color: white;");

    QObject::connect(ui->checkBoxRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );

    InitCamera();
    ui->viewFinder->stackUnder(this);


    et_geo.SetBaseParams(  0.052f, 1575.42e6, deg_wide_Y, deg_wide_X, deg_prec );
    et_file.SetBaseParams( 0.052f, 1575.42e6, deg_wide_Y, deg_wide_X, deg_prec );

    ui->radioButtonEtalonsFile->setChecked(false);
    ui->radioButtonEtalonsGeo->setChecked(true);
    et = &et_geo;
    et->MakeEtalons();

    SetCurrentIdx( left_point + points_cnt/2 );
}

PhaseForm::~PhaseForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
    }

    running = false;
    data_valid = false;
    event_data.Notify();
    if ( tick_thr.joinable() ) {
        tick_thr.join();
    }

    if ( camera ) {
        camera->stop();
    }

    delete ui;

}

bool PhaseForm::TryLockData()
{
    lock_guard<mutex> lock(mtx_data);
    if ( data_is_busy ) {
        return false;
    } else {
        data_is_busy = true;
        return true;
    }
}

void PhaseForm::UnlockData()
{
    lock_guard<mutex> lock(mtx_data);
    data_is_busy = false;
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

void PhaseForm::HandleAllChansData( std::vector<short*>& new_all_ch_data, size_t pts_cnt )
{
    if ( pts_cnt < source_len * avg_cnt ) {
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

void PhaseForm::MakeFFTs()
{
    if ( avg_cnt == 1 ) {

        for ( size_t channel = 0; channel < all_ch_data.size(); channel++ ) {
            fft.TransformShort( all_ch_data[ channel ].data(), fft_out_averaged[ channel ].data() );
        }

    } else {

        for ( int iter = 0; iter < avg_cnt; iter++ ) {
            for ( size_t channel = 0; channel < 4; channel++ ) {
                fft.TransformShort(
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


        if ( avg_filter_cnt >= 2 ) {
            // Additional averaging for curIdx point.
            int beg = curIdx - avg_filter_cnt/2;
            int end = curIdx + avg_filter_cnt/2;
            if ( beg < 0 ) {
                beg = 0;
            }
            if ( end > half_fft_len - 1 ) {
                end = half_fft_len - 1;
            }
            scale = 1.0f / ((float) avg_cnt * 4.0f * (end - beg) );

            corr[0] = float_cpx_t( 0.0f, 0.0f );
            corr[1] = float_cpx_t( 0.0f, 0.0f );
            corr[2] = float_cpx_t( 0.0f, 0.0f );
            corr[3] = float_cpx_t( 0.0f, 0.0f );
            for ( int pt = beg; pt < end; pt++ ) {
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
            }
            float cur_power = 0.0f;
            for ( int ch = 0; ch < 4; ch++ ) {
                float_cpx_t x = corr[ ch ].mul_real( scale );
                fft_out_averaged[ ch ][ curIdx ] = x;
                cur_iqss[ ch ] = x;
                cur_power += x.len();
            }
            cur_power *= 0.25f;
            cur_power = 10.0f * log10f( cur_power );
            powerMaxCur = cur_power;
        }
    }
    data_valid = false;
}


void PhaseForm::MakePphs() {
    float xavg = 0.0f;
    float xmax = -1000.0f;
    float xmin = 1000.0f;

    for ( int ch = 0; ch < 4; ch++ ) {
        const float_cpx_t* avg_data = fft_out_averaged[ ch ].data();
        vector<float>& pwr = powers[ch];
        vector<float>& phs = phases[ch];
        if ( ch == 0 ) {
            for ( int i = left_point; i < right_point; i++ ) {
                float p = 5.0f * log10f( avg_data[i].len_squared() );
                pwr[ i ] = p;
                phs[ i ] = avg_data[i].angle_deg();

                xavg += p;
                if ( p > xmax ) { xmax = p; }
                if ( p < xmin ) { xmin = p; }

            }
        } else {
            vector<float>& phs0 = phases[0];
            for ( int i = left_point; i < right_point; i++ ) {
                float p = 5.0f * log10f( avg_data[i].len_squared() );
                pwr[ i ] = p;

                float x = avg_data[i].angle_deg() - phs0[i];
                if ( x > 180.0f ) {
                    x -= 360.0f;
                } else if ( x < -180.0f ) {
                    x += 360.0f;
                }
                phs[ i ] = p;

                xavg += p;
                if ( p > xmax ) { xmax = p; }
                if ( p < xmin ) { xmin = p; }

            }
        }
    }
    xavg /= (right_point - left_point) * 4.0f;
    this->powerAvg = xavg;
    this->powerMax = xmax;
    this->powerMin = xmin;
}

void PhaseForm::SetWidgetsData()
{
    ui->widgetPhases->SetPhasesData(   &phases, left_point, points_cnt );
    ui->widgetSpectrum->SetPowersData( &powers, left_point, points_cnt, powerMin, powerMax, powerAvg, powerMaxCur );
    ui->widgetSpectrumVertical->SetPowersData( &powers, left_point, points_cnt, powerMin, powerMax, powerAvg, powerMaxCur );

    ui->widgetPhases->SetCurrentIdx(   GetCurrentIdx(), avg_filter_cnt );
    ui->widgetSpectrum->SetCurrentIdx( GetCurrentIdx(), avg_filter_cnt );
    ui->widgetSpectrumVertical->SetCurrentIdx( GetCurrentIdx(), avg_filter_cnt );
}

void PhaseForm::CalcConvolution()
{
    lock_guard< mutex > lock( mtx_convolution );
    int idx = GetCurrentIdx();
    float_cpx_t iqss[4];
    float phs[3];
    {
        iqss[0] = cur_iqss[0];
        iqss[1] = cur_iqss[1];
        iqss[2] = cur_iqss[2];
        iqss[3] = cur_iqss[3];

        phs[0] = phases[1][idx];
        phs[1] = phases[2][idx];
        phs[2] = phases[3][idx];
    }

    ConvResult* result = et->CalcConvolution( iqss );
    ui->widgetConvolution->SetConvolution( result );

    ui->labelPhases->setText( QString("%1  %2  %3").arg(
        QString::number( phs[0], 'f', 0  ),
        QString::number( phs[1], 'f', 0  ),
        QString::number( phs[2], 'f', 0  )
    ));

    float diff12 = phs[0] - phs[1];
    if ( diff12 > 180.0f ) {
        diff12 -= 360.0f;
    } else if ( diff12 < -180.0f ) {
        diff12 += 360.0f;
    }

    float diff23 = phs[1] - phs[2];
    if ( diff23 > 180.0f ) {
        diff23 -= 360.0f;
    } else if ( diff23 < -180.0f ) {
        diff23 += 360.0f;
    }

    ui->labelPhasesDiff->setText( QString("%1  %2").arg(
        QString::number( diff12, 'f', 0  ),
        QString::number( diff23, 'f', 0  )
    ));
}

void PhaseForm::Tick()
{
    this_thread::sleep_for(chrono::milliseconds(2000));
    while (running) {
        event_data.WaitAndFlush();

        if ( TryLockData() ) {
            if ( data_valid ) {
                // have new data

                MakeFFTs();
                UnlockData();

                MakePphs();
                SetWidgetsData();

                CalcConvolution();

                update();
            } else {
                // no new data
                UnlockData();
            }

        } // if TryLockData()

    } // while running
}



int PhaseForm::GetCurrentIdx() {
    return curIdx;
}

void PhaseForm::SetCurrentIdx( int x )
{
    if ( x < left_point ) {
        x = left_point;
    }
    if ( x > right_point ) {
        x = right_point;
    }

    curIdx = x;
    ui->labelFreq->setText( QString(" %1 MHz").arg( QString::number(
        nullMHz - leftMHz - curIdx*filterMHz, 'f', 2  ) ));

}

void PhaseForm::InitCamera() {

    QList<QCameraInfo>& camerasList = QCameraInfo::availableCameras();

    int iter = 0;
    for (const QCameraInfo &cameraInfo : camerasList) {
        QString desc = cameraInfo.description();
        fprintf( stderr, "\n*** camera: %s\n", desc.toLatin1().data() );

        bool gotit = false;
        if ( cameraInfo.position() == QCamera::BackFace ) {
            gotit = true;
        }

        if ( ++iter == camerasList.size() ) {
            gotit = true;
        }

        if ( gotit ) {
            camera = new QCamera( cameraInfo );
            break;
        }
    }

    if ( camera ) {
        camera->setViewfinder( ui->viewFinder->videoSurface() );
        camera->start();
    } else {
        fprintf( stderr, "\n\n !!!! NO CAMERA FOUND !!!!\n\n" );
    }
}

void PhaseForm::CurChangeOutside(int value)
{
    SetCurrentIdx( value );
}


void PhaseForm::CurChangeButtonUpSlow(bool)
{
    SetCurrentIdx( GetCurrentIdx() + 1 );
}

void PhaseForm::CurChangeButtonUpFast(bool)
{
    SetCurrentIdx( GetCurrentIdx() + 10 );
}

void PhaseForm::CurChangeButtonDownSlow(bool)
{
    SetCurrentIdx( GetCurrentIdx()- 1 );
}

void PhaseForm::CurChangeButtonDownFast(bool)
{
    SetCurrentIdx( GetCurrentIdx() - 10 );
}

void PhaseForm::CurBandChange(int value)
{
    if ( value < 1 ) {
        value = 1;
    }
    if ( value > points_cnt ) {
        value = points_cnt;
    }
    avg_filter_cnt = value;
    ui->labelAvgBand->setText( QString(" %1 MHz").arg( QString::number(
        avg_filter_cnt*filterMHz, 'f', 2  ) ));
}

void PhaseForm::CurBandChangeUp(bool)
{
    CurBandChange( avg_filter_cnt + 1 );
}

void PhaseForm::CurBandChangeDown(bool)
{
    CurBandChange( avg_filter_cnt - 1 );
}

void PhaseForm::CalibrateApplyPhases(bool)
{
    int idx = GetCurrentIdx();
    float_cpx_t iqss[4];
    float phs[3];
    {
        lock_guard< mutex > lock( mtx_convolution );
        iqss[0] = cur_iqss[0];
        iqss[1] = cur_iqss[1];
        iqss[2] = cur_iqss[2];
        iqss[3] = cur_iqss[3];

        phs[0] = phases[1][idx];
        phs[1] = phases[2][idx];
        phs[2] = phases[3][idx];
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Apply new calibration",
        QString("Phases: %1  %2  %3\nApply as new calibration?").arg(
            QString::number( phs[0], 'f', 0 ),
            QString::number( phs[1], 'f', 0 ),
            QString::number( phs[2], 'f', 0 ) ),
        QMessageBox::Yes|QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
        lock_guard< mutex > lock( mtx_convolution );
        et->SetNewCalibration(iqss);
    } else {
        // nop
    }
    update();

}

void PhaseForm::CalibrateDefault(bool)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Reset to default calibration",
        "Reset to defaults?",
        QMessageBox::Yes|QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
        lock_guard< mutex > lock( mtx_convolution );
        et->ResetCalibration();
    } else {
        // nop
    }
    update();

}

void PhaseForm::ChangeEtalons(bool)
{
    if ( ui->radioButtonEtalonsFile->isChecked() ) {
        ui->radioButtonEtalonsGeo->setChecked(false);
        et = &et_file;
    } else {
        ui->radioButtonEtalonsFile->setChecked(false);
        et = &et_geo;
    }


    bool show_warning = false;
    lock_guard< mutex > lock( mtx_convolution );
    if ( et->MakeEtalons() ) {
        show_warning = true;
    }
    if ( show_warning ) {
        QMessageBox::StandardButton warning;
        warning = QMessageBox::warning(
            this,
            "Error",
            "There was an error while making etalons.\n"
            "Check console log for details\n",
            QMessageBox::Ok
        );
    }

    update();

}



















