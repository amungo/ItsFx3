#include <vector>
#include "gcacorr/lazy_matrix.h"

#include "gpscorrform.h"
#include "ui_gpscorrform.h"
#include "leap/leapconverter.h"

#include "util/Chan2bitParser.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

GPSCorrForm::GPSCorrForm(FX3Config *cfg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSCorrForm),
    cfg(cfg),
    working( false ),
    running( true ),
    timer_corr_prepare( "corr prepare" ),
    timer_corr( "basic corr" ),
    timer_corr_precise( "precise corr" )
{
    timer_corr_prepare.SetPrintPeriod( PRN_CNT );
    timer_corr.SetPrintPeriod( PRN_CNT );
    timer_corr_precise.SetPrintPeriod( 4 );

    ui->setupUi(this);


    QObject::connect(this, SIGNAL(satInfo(int,float,int,double,bool)),
                     this, SLOT(satChanged(int,float,int,double,bool)) );

    QObject::connect(ui->tableRes, SIGNAL( cellDoubleClicked (int, int) ),
                     this, SLOT( cellSelected( int, int ) ) );

    QObject::connect(ui->pushButtonSaveFile, SIGNAL(clicked(bool)), this, SLOT(RecFile(bool)));
    QObject::connect(ui->pushButtonFile, SIGNAL(clicked(bool)), this, SLOT(ChooseFile(bool)));

    QObject::connect(ui->checkRefresh, SIGNAL(stateChanged(int)), this, SLOT(RefreshPressed(int)));

    QObject::connect(ui->spinBoxRelativeCorr, SIGNAL(valueChanged(int)), this, SLOT(relativeCorrChanged(int)) );

    plotCorrAll   = ui->widgetCorrAll;
    plotCorrGraph = ui->widgetCorrGraph;

    gr_vis = plotCorrAll->addGraph();
    gr_vis->setPen(QPen(Qt::green));
    gr_vis->setLineStyle( (QCPGraph::LineStyle) QCPGraph::lsImpulse);

    gr_inv = plotCorrAll->addGraph();
    gr_inv->setPen(QPen(Qt::red));
    gr_inv->setLineStyle( (QCPGraph::LineStyle) QCPGraph::lsImpulse);

    cdata.resize(PRN_MAX+1);

    visible_sats.resize( PRN_MAX );
    visible_corrs.resize( PRN_MAX );

    invisible_sats.resize( PRN_MAX );
    invisible_corrs.resize( PRN_MAX );

    for ( int i = 0; i < PRN_MAX; i++ ) {
        invisible_sats[ i ] = i;
        invisible_corrs[ i ] = 1.5;

        visible_sats[ i ] = i;
        visible_corrs[ i ] = 0.0;
    }

    gr_vis->setData( visible_sats, visible_corrs );
    gr_vis->rescaleAxes(true);

    gr_inv->setData( invisible_sats, invisible_corrs );
    gr_inv->rescaleAxes(true);

    plotCorrAll->yAxis->setRange( 0, 20 );

    ui->tableRes->setRowCount( PRN_MAX );
    ui->tableRes->setColumnCount( 4 );
    shifts.resize( PRN_MAX );
    for ( size_t i = 0; i < shifts.size(); i++ ) {
        shifts[i] = 0;
    }

    QStringList heads;
    heads << "Stat" << "Freq" << "Shift" << "Val";
    ui->tableRes->setHorizontalHeaderLabels( heads );
    ui->tableRes->setColumnWidth( 0, 40 );
    ui->tableRes->setColumnWidth( 1, 40 );
    ui->tableRes->setColumnWidth( 2, 40 );
    ui->tableRes->setColumnWidth( 3, 40 );

    for ( int i = 0; i < PRN_MAX; ++i ) {
        ui->tableRes->setRowHeight( i, 18 );
    }


    ui->comboBoxChannel->clear();
    for ( int i = 0; i < cfg->chan_count; i++ ) {
        QString str;
        str.sprintf( "ch%d", i );
        ui->comboBoxChannel->addItem( str, i );
    }
    antijamIdx = cfg->chan_count;
    ui->comboBoxChannel->addItem( "antijamming", antijamIdx );

    ui->comboBoxChannel->setCurrentIndex( 0 );

    relativeCorrChanged(0);

    set_tmp_dir( "M:\\tmp" );

    calc_thread = std::thread(&GPSCorrForm::calcLoop, this);
}

GPSCorrForm::~GPSCorrForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
        router->DeleteOutPoint( &dumper );
    }
    delete ui;
    qDebug( "GPSCorrForm::~GPSCorrForm() will wait for thread\n" );
    running = false;
    if ( calc_thread.joinable() ) {
        calc_thread.join();
    }
    qDebug( "GPSCorrForm::~GPSCorrForm() finished!\n" );
}

void GPSCorrForm::SetWorking(bool b) {
    working = b;
    uiRecalc();
}

void GPSCorrForm::calcSats()
{

    for ( int si = 0; si < PRN_CNT; si++ ) {
        double freq;
        int tshift;
        float corrval;

        timer_corr_prepare.Start();
        GPSVis sv( PRN_IN_OPER[ si ],
                   7000.0,
                   1000.0,
                   cfg->adc_sample_rate_hz,
                   0.0
                   //GPS_L1_FREQ - cfg->inter_freq_hz
                   );
        if ( sigs.size() == 1 ) {
            sv.SetEdgeKoef( 4.9 );
        } else {
            sv.SetEdgeKoef( 3.0 );
        }

        sv.SetSignal( &sigs );
        sv.CalcCorrMatrix();
        timer_corr_prepare.Finish();

        plot_data_t& p = cdata[ PRN_IN_OPER[ si ] ];
        timer_corr.Start();
        bool sv_vis = sv.FindMaxCorr( freq, tshift, corrval );
        timer_corr.Finish();
        if ( sv_vis ) {
            emit satInfo( PRN_IN_OPER[ si ], corrval, tshift, freq, true );

            timer_corr_precise.Start();
            sv.PreciseFreq( freq, tshift, corrval );
            timer_corr_precise.Finish();

            emit satInfo( PRN_IN_OPER[ si ], corrval, tshift, freq, true );

            p.mutex->lock();
            sv.GetCorrMatrix( p.cors, p.freqs_vals );
            p.center = tshift;
            p.inited = true;
            p.mutex->unlock();

            //file_dump( NULL, 0, "xcorr", "flt", PRN_IN_OPER[ si ], true );
            //for ( unsigned int fi = 0; fi < cm.size(); fi++ ) {
            //    file_dump( &cm[ fi ][ 0 ], cm[ fi ].size()*4, "xcorr", "flt", PRN_IN_OPER[ si ], true );
            //}
            //file_dump( &freqs[ 0 ], freqs.size()*8, "freqs", "flt", PRN_IN_OPER[ si ], false );
            //fprintf( stderr, "[%2d] size %d x %d\n", PRN_IN_OPER[ si ], cm.size(), cm[0].size() );

        } else {
            p.mutex->lock();

            //p.inited = false;

            sv.GetCorrMatrix( p.cors, p.freqs_vals );
            p.center = tshift;
            p.inited = true;

            p.mutex->unlock();
            emit satInfo( PRN_IN_OPER[ si ], corrval, tshift, freq, false );
        }

        if ( !running ) {
            break;
        }
    }

    for ( uint32_t i = 0; i < sigs.size(); i++ ) {
        delete sigs[ i ];
    }
    sigs.clear();
}

const int fir_gps_len = 23;
const float fir_gps[fir_gps_len] = {
    0.00212598918, 0.007456087042,  0.01171644684,  0.02019771934,  0.02972034924,
    0.04128643125,  0.05352092162,  0.06577334553,  0.07682032883,  0.08566072583,
     0.0913593322,  0.09333115071,   0.0913593322,  0.08566072583,  0.07682032883,
    0.06577334553,  0.05352092162,  0.04128643125,  0.02972034924,  0.02019771934,
    0.01171644684, 0.007456087042,  0.00212598918
};


void GPSCorrForm::HandleADCStreamData(void *data, size_t size8) {
    if ( ui->comboBoxChannel->currentIndex() == antijamIdx &&
         !working &&
         ui->checkRefresh->isChecked() )
    {
        int DATA_SIZE      = cfg->adc_sample_rate_hz / 1000.0;
        int DATA_SIZE_WFIR = DATA_SIZE + fir_gps_len;
        if ( DATA_SIZE_WFIR > size8 ) {
            fprintf( stderr, "DATA_SIZE_WFIR > size8\n" );
        }

        std::vector< float_cpx_t > s[4];
        for ( int i = 0; i < 4; i++ ) {
            s[i].resize(DATA_SIZE_WFIR);
        }

        int8_t* adcsrc = (int8_t*) data;
        for ( int i = 0; i < DATA_SIZE_WFIR; i++ ) {
            s[0][i].i = decode_2bchar_to_float_ch0( adcsrc[i] );
            s[0][i].q = 0.0f;

            s[1][i].i = decode_2bchar_to_float_ch1( adcsrc[i] );
            s[1][i].q = 0.0f;

            s[2][i].i = decode_2bchar_to_float_ch2( adcsrc[i] );
            s[2][i].q = 0.0f;

            s[3][i].i = decode_2bchar_to_float_ch3( adcsrc[i] );
            s[3][i].q = 0.0f;

        }

        for ( int i = 0; i < 4; i++ ) {
            double freq = GPS_L1_FREQ - cfg->inter_freq_hz; //-14.58e6 = 1575.42e6 - 1590.0e6
            float_cpx_t* shifted  = freq_shift( s[i].data(), DATA_SIZE_WFIR, cfg->adc_sample_rate_hz, freq );
            float_cpx_t* filtered = make_fir( shifted, fir_gps, DATA_SIZE, fir_gps_len );
            memcpy( s[i].data(), filtered, sizeof(float_cpx_t) * DATA_SIZE_WFIR );
            delete [] shifted;
            delete [] filtered;
        }

        matrix_t corr = create_matrix(4);
        for ( int i = 0; i < 4; i++ ) {
            for ( int k = 0; k < 4; k++ ) {
                corr[i][k] = calc_correlation(s[i].data(), s[k].data(), DATA_SIZE);
                corr[i][k].mul_real(1.0f/DATA_SIZE);
            }
        }
        matrix_t inv_corr = inverse_matrix(corr);

        print_matrix( corr );
        printf( "******************************************\n" );
        //print_matrix( mul_matrix( corr, float_cpx_t( 10.0, 0.0 ) ) );
        //print_matrix( cut_matrix(corr, 3, 0) );
        print_matrix( inv_corr );
        printf( "******************************************\n" );
        float_cpx_t det = determinant(corr);
        printf( "det = %f +%fi\n", det.i, det.q );

        for ( int i = 0; i < 4; i++ ) {
            mul_vec( s[i].data(), inv_corr[0][i], DATA_SIZE );
        }
        for ( int i = 1; i < 4; i++ ) {
            add_vector( s[0].data(), s[i].data(), DATA_SIZE );
        }


        sigs.resize(1);
        sigs.at(0) = new RawSignal( DATA_SIZE, cfg->adc_sample_rate_hz );
        sigs.at(0)->LoadData( s[0].data(), DT_FLOAT_IQ, 0 );

        SetWorking( true );
    }

}

void GPSCorrForm::HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel) {
    if ( ui->comboBoxChannel->currentIndex() != channel ) {
        return;
    }

    if ( working || !ui->checkRefresh->isChecked() ) {
        return;
    } else {
        sigs.resize( ui->checkAverageX8->isChecked() ? 8 : 1 );

        int DATA_SIZE = cfg->adc_sample_rate_hz / 1000.0;
        int ALL_DATA_SIZE = DATA_SIZE * sigs.size();
        int ALL_DATA_SIZE_WFIR = ALL_DATA_SIZE + fir_gps_len;

        float_cpx_t* sss = new float_cpx_t[ ALL_DATA_SIZE_WFIR ];

        for ( int i = 0; i < ALL_DATA_SIZE_WFIR; i++ ) {
            sss[i].i = (float) one_ch_data[i];
            sss[i].q = 0.0f;
        }

        double freq = GPS_L1_FREQ - cfg->inter_freq_hz; //-14.58e6 = 1575.42e6 - 1590.0e6
        float_cpx_t* shifted  = freq_shift( sss, ALL_DATA_SIZE_WFIR, cfg->adc_sample_rate_hz, freq );
        float_cpx_t* filtered = make_fir( shifted, fir_gps, ALL_DATA_SIZE, fir_gps_len );

        for ( uint32_t i = 0; i < sigs.size(); i++ ) {
            sigs[ i ] = new RawSignal(
                        DATA_SIZE,
                        cfg->adc_sample_rate_hz );

            sigs[ i ]->LoadData( filtered, DT_FLOAT_IQ, i*DATA_SIZE );
        }

        delete [] sss;
        delete [] shifted;
        delete [] filtered;

        SetWorking( true );
    }
}

void GPSCorrForm::onFileDumpComplete(std::string fname, ChunkDumpParams params) {
    router->DeleteOutPoint( &dumper );
    fprintf( stderr, "GPSCorrForm::onFileDumpComplete %s %d x (%d + %d)\n",
             fname.c_str(), params.count, params.len, params.gap );

    LEAPConverter converter( ( int ) cfg->adc_sample_rate_hz, cfg->inter_freq_hz );
    int bytes_per_ms = ( int ) ( cfg->adc_sample_rate_hz / 1000.0 ) * sizeof( short );

    converter.convertFile( fname.c_str(),
                           params.count,
                           params.len / bytes_per_ms,
                           params.gap / bytes_per_ms );

    ui->pushButtonSaveFile->setEnabled( true );
}

void GPSCorrForm::calcLoop() {
    qDebug( "GPSCorrForm::calcLoop() STARTED\n" );
    while ( running ) {
        #ifdef WIN32
        Sleep( 500 );
        #else
        usleep( 500 * 1000 );
        #endif
        if ( working ) {
            calcSats();
            SetWorking( false );
        }
    }
    qDebug( "GPSCorrForm::calcLoop() FINISH\n" );
}

void GPSCorrForm::redrawVisGraph() {
    gr_vis->setData( visible_sats, visible_corrs );
    gr_vis->rescaleAxes(true);

    gr_inv->setData( invisible_sats, invisible_corrs );
    gr_inv->rescaleAxes(true);

    plotCorrAll->replot();
}

void GPSCorrForm::uiRecalc() {
    if ( !ui->checkRefresh->isChecked() && !working ) {
        ui->comboBoxChannel->setEnabled( true );
    } else {
        ui->comboBoxChannel->setEnabled( false );
    }
}

QTableWidgetItem* MakeTableItem( const QString& str, bool grey ) {
    QTableWidgetItem* item = new QTableWidgetItem( str );
    item->setTextAlignment( Qt::AlignRight );
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    if ( grey ) {
        item->setForeground(QColor::fromRgb(128,128,128));
    }
    return item;
}

void GPSCorrForm::setshifts() {
    for ( int i = 0; i < ui->tableRes->rowCount() && i < shifts.size(); i++ ) {
        ui->tableRes->setItem( i, 2,
            MakeTableItem( QString::number( shifts.at(i) - relativeShift ), true ) );
    }
}


void GPSCorrForm::satChanged(int prn, float corr, int shift, double freq, bool is_visible) {

    int tidx = prn - 1;


    if ( is_visible ) {
        visible_corrs[ prn ]   = corr;
        invisible_corrs[ prn ] = 0;

        ui->tableRes->setItem( tidx, 0, MakeTableItem( QString("VIS"), !is_visible ) );
    } else {
        visible_corrs[ prn ]   = 0;
        invisible_corrs[ prn ] = corr;

        ui->tableRes->setItem( tidx, 0, MakeTableItem(QString("-"), !is_visible ) );
    }

    shifts.at(tidx) = shift;

    ui->tableRes->setItem( tidx, 1,
                           MakeTableItem( QString::number( freq, 'f', 0  ), !is_visible ) );
    ui->tableRes->setItem( tidx, 2,
                           MakeTableItem( QString::number( shifts.at(tidx) - relativeShift ), !is_visible ) );
    ui->tableRes->setItem( tidx, 3,
                           MakeTableItem( QString::number( corr, 'g', 2 ), !is_visible ) );

    setshifts();
    redrawVisGraph();
}

void GPSCorrForm::cellSelected(int x, int) {
    int idx = x + 1;
    plot_data_t& p = cdata[ idx ];
    p.mutex->lock();
    qDebug( "PRN %d, init = %d", idx, p.inited );
    ui->spinBoxRelativeCorr->setValue(shifts.at(x));

    plotCorrGraph->clearGraphs();
    if ( p.inited ) {
        int N = p.cors[ 0 ].size();

        QVector< double > times;
        times.resize( N );

        for ( int i = 0; i < N; i++ ) {
            times[ i ] = p.center - N/2 + i;
        }

        for ( int i = 0; i < p.cors.size(); i++ ) {
            QCPGraph* g = plotCorrGraph->addGraph();

            QVector< double > vals;
            std::vector<float> src = p.cors[ i ];
            vals.resize( src.size() );

            for ( int i = 0; i < src.size(); i++ ) {
                vals[ i ] = src[ i ];
            }

            g->setData( times, vals );
            g->rescaleAxes( true );
        }
        plotCorrGraph->xAxis->setRange(p.center - N/2, p.center + N/2);
        plotCorrGraph->yAxis->setRange(0, 35000);
    }
    p.mutex->unlock();
    plotCorrGraph->replot();
}

void GPSCorrForm::RecFile(bool) {
    ui->pushButtonSaveFile->setEnabled( false );

    int bytes_per_ms = ( int ) ( cfg->adc_sample_rate_hz / 1000.0 ) * sizeof( short );

    ChunkDumpParams params;
    params.all_chans_raw_sig = false;
    params.chan_num = ui->comboBoxChannel->currentIndex();
    params.count    = ui->spinBoxChunkCount->value();
    params.len      = ui->spinBoxChunkLen->value() * bytes_per_ms;
    params.gap      = ui->spinBoxChunkGap->value() * bytes_per_ms;
    int res = dumper.DumpAsync( ui->lineRecFileName->text().toLatin1().data(), params, this );
    if ( res ) {
        fprintf( stderr, "GPSCorrForm::RecFile() bad file\n" );
        ui->pushButtonSaveFile->setEnabled( true );
    }
    router->AddOutPoint( &dumper );
}

void GPSCorrForm::ChooseFile(bool) {
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
        tr("Select file for MS LEAP dump"),
        "",
        tr("Binary files (*.bin)" )
    );
    if ( fileName.size() > 1 ) {
        ui->lineRecFileName->setText( fileName );
    }}

void GPSCorrForm::RefreshPressed(int state) {
    uiRecalc();
}

void GPSCorrForm::relativeCorrChanged(int x) {
    fprintf( stderr, "relativeCorrChanged(%d)\n\n", x );
    relativeShift = ui->spinBoxRelativeCorr->value();
    setshifts();
}
