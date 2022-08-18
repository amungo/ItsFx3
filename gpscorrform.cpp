#include <vector>
#include <chrono>
#include "gcacorr/lazy_matrix.h"
#include "gcacorr/filters.h"

#include "gpscorrform.h"
#include "ui_gpscorrform.h"
#include "leap/leapconverter.h"

#include "util/Chan2bitParser.h"

enum TableColumnsIndexes {
    COL_STAT = 0,
    COL_FREQ,
    COL_SHIFT,
    COL_VAL,
    COL_CHCK,
    COL_NUM
};

GPSCorrForm::GPSCorrForm(FX3Config *cfg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSCorrForm),
    cfg(cfg),
    working( false ),
    running( true ),
    gnss_type( GPS_L1 )
{

    ui->setupUi(this);


    ui->comboBoxGnssType->insertItem(0, "GPS L1", QVariant(GPS_L1) );
    ui->comboBoxGnssType->insertItem(1, "GLONASS L1", QVariant(GLONASS_L1) );
    ui->comboBoxGnssType->insertItem(2, "GLONASS L2", QVariant(GLONASS_L2) );

    QObject::connect(this, SIGNAL(satInfo(int,float,int,double,bool)),
                     this, SLOT(satChanged(int,float,int,double,bool)) );

    QObject::connect(ui->tableRes, SIGNAL( cellDoubleClicked (int, int) ),
                     this, SLOT( cellSelected( int, int ) ) );

    QObject::connect(ui->pushButtonSaveFile, SIGNAL(clicked(bool)), this, SLOT(RecFile(bool)));
    QObject::connect(ui->pushButtonFile, SIGNAL(clicked(bool)), this, SLOT(ChooseFile(bool)));

    QObject::connect(ui->checkRefresh, SIGNAL(stateChanged(int)), this, SLOT(RefreshPressed(int)));

    QObject::connect(ui->comboBoxGnssType, SIGNAL(currentIndexChanged(int)), this, SLOT(gnssTypeChanged(int)));

    QObject::connect(ui->pushButtonCheckAll,     SIGNAL(clicked(bool)), this, SLOT(checkAll(bool)));
    QObject::connect(ui->pushButtonUnchekAll,    SIGNAL(clicked(bool)), this, SLOT(uncheckAll(bool)));
    QObject::connect(ui->pushButtonUncheckInVis, SIGNAL(clicked(bool)), this, SLOT(uncheckInVis(bool)));

    plotCorrGraph = ui->widgetCorrGraph;

    cdata.resize(PRN_MAX+1);

    ui->tableRes->setRowCount( PRN_MAX );
    ui->tableRes->setColumnCount( COL_NUM );
    shifts.resize( PRN_MAX + 1 );
    visibles.resize( PRN_MAX + 1 );
    for ( size_t i = 0; i < shifts.size(); i++ ) {
        shifts[i] = 0;
        visibles[i] = false;
    }

    QStringList heads;
    heads << "Stat" << "Freq" << "Shift" << "Val" << "Calc";
    ui->tableRes->setHorizontalHeaderLabels( heads );
    ui->tableRes->setColumnWidth( COL_STAT,  40 );
    ui->tableRes->setColumnWidth( COL_FREQ,  50 );
    ui->tableRes->setColumnWidth( COL_SHIFT, 50 );
    ui->tableRes->setColumnWidth( COL_VAL,   40 );
    ui->tableRes->setColumnWidth( COL_CHCK,  40 );

    calc_checks.resize(PRN_MAX+1);
    for ( int i = 0; i < PRN_MAX; ++i ) {
        ui->tableRes->setRowHeight( i, 18 );
        QCheckBox* chbx = new QCheckBox();
        calc_checks.at(i+1) = chbx;
        chbx->setChecked(true);
        ui->tableRes->setCellWidget( i, COL_CHCK, chbx );
        QObject::connect(chbx, SIGNAL(stateChanged(int)), this, SLOT(prnCheckUncheck(int)));
    }


    ui->comboBoxChannel->clear();
    for ( int i = 0; i < cfg->chan_count; i++ ) {
        QString str;
        str.sprintf( "ch%d", i );
        ui->comboBoxChannel->addItem( str, i );
    }
    //antijamIdx = cfg->chan_count;
    //ui->comboBoxChannel->addItem( "antijamming", antijamIdx );

    ui->comboBoxChip->setCurrentIndex( 0 );
    ui->comboBoxChannel->setCurrentIndex( 0 );

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

void GPSCorrForm::setTableItem(int row, int col, const QString &str, bool is_greyed)
{
    QTableWidgetItem* item = ui->tableRes->item( row, col );
    if ( !item ) {
        item = new QTableWidgetItem( str );
        item->setTextAlignment( Qt::AlignRight );
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->tableRes->setItem( row, col, item );
    } else {
        item->setText( str );
    }
    if ( is_greyed ) {
        item->setForeground(Qt::gray);
        item->setBackgroundColor(Qt::white);
    } else {
        item->setForeground(Qt::black);
        if ( col == COL_STAT ) {
            item->setBackgroundColor(Qt::green);
        }
    }
}


void GPSCorrForm::SetWorking(bool b) {
    working = b;
    uiRecalc();
}

int GPSCorrForm::GetFilterLen()
{
    if ( gnss_type == GPS_L1 ) {
        return filter_53M_2M_10M_1_40_len;

    } else if ( gnss_type == GLONASS_L1 ) {
        return filter_53M_1M_10M_1_40_len;

    } else /*if ( gnss_type == GLONASS_L2 )*/ {
        return filter_53M_1M_10M_1_40_len;
    }
}

float *GPSCorrForm::GetFir()
{
    if ( gnss_type == GPS_L1 ) {
        return filter_53M_2M_10M_1_40;

    } else if ( gnss_type == GLONASS_L1 ) {
        return filter_53M_1M_10M_1_40;

    } else /*if ( gnss_type == GLONASS_L2 )*/ {
        return filter_53M_1M_10M_1_40;
    }
}

double GPSCorrForm::GetFreq(int prn_num)
{
    if ( gnss_type == GPS_L1 ) {
        double prn_freq = 1575.42e6;
        return prn_freq - 1590.0e6;

    } else if ( gnss_type == GLONASS_L1 ) {
        double prn_freq = ( 1602.0 + (prn_num - 8) * 0.5625 ) * 1.0e6;
        return prn_freq - 1590.0e6;

    } else /*if ( gnss_type == GLONASS_L2 )*/ {
        double prn_freq = ( 1246.0 + (prn_num - 8) * 0.4375 ) * 1.0e6;
        return prn_freq - 1235.0e6;

    }
}

int GPSCorrForm::GetPrnCount()
{
    if ( gnss_type == GPS_L1 ) {
        return 32;

    } else if ( gnss_type == GLONASS_L1 ) {
        return 15;

    } else /*if ( gnss_type == GLONASS_L2 )*/ {
        return 15;

    }
}

void GPSCorrForm::PrepareRawData()
{
    fprintf( stderr, "Preparing raw data (filtering): \n");
    int avg_cnt = ui->checkAverageX8->isChecked() ? 8 : 1;

    int DATA_SIZE = cfg->adc_sample_rate_hz / 1000.0;
    int ALL_DATA_SIZE = DATA_SIZE * avg_cnt;
    int ALL_DATA_SIZE_WFIR = ALL_DATA_SIZE + GetFilterLen();

    float_cpx_t* sss = new float_cpx_t[ ALL_DATA_SIZE_WFIR ];

    for ( int i = 0; i < ALL_DATA_SIZE_WFIR; i++ ) {
        sss[i].i = (float) cached_one_chan_data[ i ];
        sss[i].q = 0.0f;
    }

    for ( int prn = 1; prn <= GetPrnCount(); prn++ ) {
        if ( !calc_checks.at(prn)->isChecked() ) {
            continue;
        }
        fprintf( stderr, "%3d", prn);
        sigs[ prn ].resize( avg_cnt );

        if ( ui->checkBoxUseFilter->isChecked() ) {

            // ****** SHIFT & FILTER ******
            float_cpx_t* shifted  = freq_shift( sss, ALL_DATA_SIZE_WFIR, cfg->adc_sample_rate_hz, -GetFreq(prn) );
            float_cpx_t* filtered = make_fir( shifted, GetFir(), ALL_DATA_SIZE, GetFilterLen() );

            for ( uint32_t i = 0; i < sigs[ prn ].size(); i++ ) {
                sigs[ prn ][ i ] = new RawSignal( DATA_SIZE, cfg->adc_sample_rate_hz );
                sigs[ prn ][ i ]->LoadData( filtered, DT_FLOAT_IQ, i*DATA_SIZE );
            }

            if ( gnss_type == GPS_L1 ) {
                for ( int prn2 = 2; prn2 < GetPrnCount(); prn2++ ) {
                    sigs[ prn2 ].resize( avg_cnt );

                    for ( uint32_t i = 0; i < sigs[ prn2 ].size(); i++ ) {
                        sigs[ prn2 ][ i ] = new RawSignal( DATA_SIZE, cfg->adc_sample_rate_hz );
                        sigs[ prn2 ][ i ]->LoadData( filtered, DT_FLOAT_IQ, i*DATA_SIZE );
                    }
                }

                delete [] shifted;
                delete [] filtered;
                break; // goto DONE
            }

            delete [] shifted;
            delete [] filtered;

        } else {

            // ****** Use original signal ******
            for ( uint32_t i = 0; i < sigs[ prn ].size(); i++ ) {
                sigs[ prn ][ i ] = new RawSignal( DATA_SIZE, cfg->adc_sample_rate_hz );
                sigs[ prn ][ i ]->LoadData( sss, DT_FLOAT_IQ, i*DATA_SIZE );
            }

        }
    }
    // DONE
    delete [] sss;
    fprintf( stderr, "\nPreparing raw data DONE\n");
}



void GPSCorrForm::calcSats()
{

    relativeShitValid = false;

    for ( int prn = 1; prn <= GetPrnCount(); prn++ ) {

        if ( !calc_checks.at(prn)->isChecked() ) {
            if ( sigs.find(prn) != sigs.end() ) {
                for ( uint32_t i = 0; i < sigs[prn].size(); i++ ) {
                    delete sigs[ prn ][ i ];
                }
                sigs.erase( prn );
            }
            visibles.at(prn-1) = false;
            continue;
        }

        if ( sigs.find(prn) == sigs.end() ) {
            continue;
        }

        double freq;
        int tshift;
        float corrval;

        GPSVis sv( prn,
                   7000.0,
                   ui->spinBoxFreqStep->value(),
                   cfg->adc_sample_rate_hz,
                   ui->checkBoxUseFilter->isChecked() ? 0.0 : GetFreq( prn ),
                   gnss_type == GLONASS_L1 || gnss_type == GLONASS_L2
                   );
        if ( sigs[prn].size() == 1 ) {
            sv.SetEdgeKoef( 4.9 );
        } else {
            sv.SetEdgeKoef( 3.0 );
        }

        sv.SetSignal( &sigs[prn] );
        sv.CalcCorrMatrix();

        plot_data_t& p = cdata[ prn ];
        bool sv_vis = sv.FindMaxCorr( freq, tshift, corrval );
        if ( sv_vis ) {
            emit satInfo( prn, corrval, tshift, freq, true );

            if ( ui->checkBoxPrecise->isChecked() ) {
                sv.PreciseFreq( freq, tshift, corrval );
            }

            emit satInfo( prn, corrval, tshift, freq, true );

            p.mutex->lock();
            sv.GetCorrMatrix( p.cors, p.freqs_vals );
            p.center = tshift;
            p.inited = true;
            p.mutex->unlock();

            //file_dump( NULL, 0, "xcorr", "flt", si, true );
            //for ( unsigned int fi = 0; fi < cm.size(); fi++ ) {
            //    file_dump( &cm[ fi ][ 0 ], cm[ fi ].size()*4, "xcorr", "flt", si, true );
            //}
            //file_dump( &freqs[ 0 ], freqs.size()*8, "freqs", "flt", si, false );
            //fprintf( stderr, "[%2d] size %d x %d\n", si, cm.size(), cm[0].size() );

        } else {
            p.mutex->lock();

            //p.inited = false;

            sv.GetCorrMatrix( p.cors, p.freqs_vals );
            p.center = tshift;
            p.inited = true;

            p.mutex->unlock();
            emit satInfo( prn, corrval, tshift, freq, false );
        }

        for ( uint32_t i = 0; i < sigs[prn].size(); i++ ) {
            delete sigs[ prn ][ i ];
        }
        sigs.erase( prn );

        if ( !running ) {
            break;
        }
    }

    sigs.clear();
}


void GPSCorrForm::HandleADCStreamData(void *data, size_t size8) {
#if 0
    if ( ui->comboBoxChannel->currentIndex() == antijamIdx &&
         !working &&
         ui->checkRefresh->isChecked() )
    {
        int DATA_SIZE      = cfg->adc_sample_rate_hz / 1000.0;
        int DATA_SIZE_WFIR = DATA_SIZE + GetFilterLen();
        if ( DATA_SIZE_WFIR > (int)size8 ) {
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
            float_cpx_t* filtered = make_fir( shifted, GetFir(), DATA_SIZE, GetFilterLen() );
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
#endif
}

void GPSCorrForm::HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int chip, int channel) {
    if ( ui->comboBoxChannel->currentIndex() != channel || ui->comboBoxChip->currentIndex() != chip) {
        return;
    }

    if ( working || !ui->checkRefresh->isChecked() ) {
        return;
    } else {

        if ( cached_one_chan_data.size() != pts_cnt ) {
            cached_one_chan_data.resize( pts_cnt );
        }
        memcpy( cached_one_chan_data.data(), one_ch_data, pts_cnt * sizeof( short ) );

        SetWorking( true );
    }
}


void GPSCorrForm::HandleGyroData(char* data, size_t size8)
{

    if(gyro_data.size() != size8)
        gyro_data.resize(size8);

    memcpy(gyro_data.data(), data, size8);
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if ( working ) {
            PrepareRawData();
            calcSats();
            SetWorking( false );
        }
    }
    qDebug( "GPSCorrForm::calcLoop() FINISH\n" );
}


void GPSCorrForm::uiRecalc() {
    bool enabled;
    if ( !ui->checkRefresh->isChecked() && !working ) {
        enabled = true;
    } else {
        enabled = false;
    }
    ui->comboBoxChannel->setEnabled( enabled );
    ui->comboBoxGnssType->setEnabled( enabled );
    ui->checkAverageX8->setEnabled( enabled );
    ui->checkBoxUseFilter->setEnabled( enabled );
    //ui->checkBoxPrecise->setEnabled( enabled );
}

void GPSCorrForm::setshifts() {
    if ( relativeShitValid ) {
        for ( int i = 0; i < ui->tableRes->rowCount() && i < (int)shifts.size(); i++ ) {
            if ( calc_checks.at(i+1)->isChecked() ) {
                setTableItem( i, COL_SHIFT, QString::number( shifts.at(i) - relativeShift ), !visibles.at(i) );
            }
        }
    }
}


void GPSCorrForm::satChanged(int prn, float corr, int shift, double freq, bool is_visible) {
    int tidx = prn - 1;

    shifts.at(tidx) = shift;
    visibles.at(tidx) = is_visible;

    if ( prn == selectedPrn ) {
        relativeShift = shift;
        relativeShitValid = true;
    }


    if ( is_visible ) {
        setTableItem( tidx, COL_STAT, QString("VIS"), !is_visible );
    } else {
        setTableItem( tidx, COL_STAT, QString("-"), !is_visible );
    }

    setTableItem( tidx, COL_FREQ, QString::number( freq, 'f', 0  ), !is_visible );
    setTableItem( tidx, COL_VAL,  QString::number( corr, 'g', 2 ), !is_visible );

    if ( relativeShitValid ) {
        setTableItem( tidx, COL_SHIFT, QString::number( shifts.at(tidx) - relativeShift ), !is_visible );
    }

    setshifts();

    gyroChanged();
}

void GPSCorrForm::gyroChanged()
{
    unsigned short* p16 = (unsigned short*)gyro_data.data();

    unsigned short adxr1 = p16[0];
    unsigned short adxr2 = p16[1];
    unsigned short adxr3 = p16[2];

    unsigned int x_coord = (p16[3] << 16) | p16[4];
    unsigned int y_coord = (p16[5] << 16) | p16[6];
    unsigned int z_coord = (p16[7] << 16) | p16[8];

    ui->le_gyro1->setText(QString("%1").arg(adxr1, 0, 16));
    ui->le_gyro2->setText(QString("%1").arg(adxr2, 0, 16));
    ui->le_gyro3->setText(QString("%1").arg(adxr3, 0, 16));

    ui->le_accel_x->setText(QString("%1").arg(x_coord, 0, 16));
    ui->le_accel_y->setText(QString("%1").arg(y_coord, 0, 16));
    ui->le_accel_z->setText(QString("%1").arg(z_coord, 0, 16));

}

void GPSCorrForm::cellSelected(int x, int) {
    int idx = x + 1;

    selectedPrn = idx;
    relativeShift = shifts.at(x);
    setshifts();

    plot_data_t& p = cdata[ idx ];
    p.mutex->lock();

    plotCorrGraph->clearGraphs();
    if ( p.inited ) {
        int N = p.cors[ 0 ].size();

        QVector< double > times;
        times.resize( N );

        for ( int i = 0; i < N; i++ ) {
            times[ i ] = p.center - N/2 + i;
        }

        for ( size_t i = 0; i < p.cors.size(); i++ ) {
            QCPGraph* g = plotCorrGraph->addGraph();

            QVector< double > vals;
            std::vector<float> src = p.cors[ i ];
            vals.resize( src.size() );

            for ( size_t i = 0; i < src.size(); i++ ) {
                vals[ i ] = src[ i ];
            }

            g->setData( times, vals );
            g->rescaleAxes( true );
        }
        plotCorrGraph->xAxis->setRange(p.center - N/2, p.center + N/2);
        if ( ui->checkAverageX8->isChecked() ) {
            plotCorrGraph->yAxis->setRange(0, 35000);
        } else {
            plotCorrGraph->yAxis->setRange(0, 10000);
        }
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
    }
}

void GPSCorrForm::RefreshPressed(int /*state*/) {
    uiRecalc();
}

void GPSCorrForm::gnssTypeChanged(int)
{
    bool ok;
    GNSSType newtype = (GNSSType)ui->comboBoxGnssType->currentData().toInt(&ok);
    if ( gnss_type != newtype ) {
        for ( int i = 0; i < PRN_MAX; i++ ) {

            shifts.at(i) = 0;
            setTableItem( i, COL_STAT, QString("-"), true );
            setTableItem( i, COL_FREQ,  "-", true );
            setTableItem( i, COL_SHIFT, "-", true );
            setTableItem( i, COL_VAL,   "-", true );
        }
    }
    gnss_type = newtype;
}

void GPSCorrForm::prnCheckUncheck(int)
{
    for ( int i = 0; i < PRN_MAX; i++ ) {
        if ( !calc_checks.at(i+1)->isChecked() ) {
            setTableItem( i, COL_STAT, QString("-"), true );
            setTableItem( i, COL_FREQ,  "-", true );
            setTableItem( i, COL_SHIFT, "-", true );
            setTableItem( i, COL_VAL,   "-", true );
        }
    }
}

void GPSCorrForm::checkAll(bool)
{
    for ( size_t i = 1; i < calc_checks.size(); i++ ) {
        calc_checks.at(i)->setChecked(true);
    }
}

void GPSCorrForm::uncheckAll(bool)
{
    for ( size_t i = 1; i < calc_checks.size(); i++ ) {
        calc_checks.at(i)->setChecked(false);
    }
}

void GPSCorrForm::uncheckInVis(bool)
{
    for ( size_t i = 1; i < calc_checks.size(); i++ ) {
        if ( !visibles.at(i-1) ) {
            calc_checks.at(i)->setChecked(false);
        }
    }
}
