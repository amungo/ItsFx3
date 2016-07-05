#include "rawsignalform.h"
#include "ui_rawsignalform.h"
#include "util/Chan2bitParser.h"

RawSignalForm::RawSignalForm(FX3Config *cfg_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RawSignalForm),
    cfg(cfg_),
    skipped_packets(0),
    first_run( true ),
    last_axis( 10.0 ),
    axis_blank_percent( 0.3 ),
    chan_count( cfg_->chan_count )
{
    last_shift.resize( chan_count );
    for ( uint32_t i = 0; i < last_shift.size(); i++ ) {
        last_shift[ i ] = 0;
    }

    ui->setupUi(this);
    
    rawPlot = ui->plotRaw;
    rawPlot->addGraph();
    rawPlot->addGraph();
    rawPlot->addGraph();
    rawPlot->addGraph();
    rawPlot->graph(0)->setPen(QPen(Qt::green));
    rawPlot->graph(1)->setPen(QPen(Qt::red));
    rawPlot->graph(2)->setPen(QPen(Qt::blue));
    rawPlot->graph(3)->setPen(QPen(Qt::gray));

    if ( cfg->signal_type == SigTypeIQParts ) {
        rawPlot->addGraph();
        rawPlot->addGraph();
        rawPlot->addGraph();
        rawPlot->addGraph();
        rawPlot->graph(4)->setPen(QPen(Qt::darkGreen));
        rawPlot->graph(5)->setPen(QPen(Qt::darkRed));
        rawPlot->graph(6)->setPen(QPen(Qt::darkBlue));
        rawPlot->graph(7)->setPen(QPen(Qt::darkGray));
    }


    rawPlot->xAxis->setRange(0, 1000);
    rawPlot->yAxis->setRange(-10, 10);
    
    QObject::connect(this, SIGNAL(signalNeedReplot()),               this, SLOT(slotReplot()) );
    QObject::connect(ui->plotRaw, SIGNAL(afterReplot()),             this, SLOT(slotReplotComplete()) );
    QObject::connect(ui->checkRun, SIGNAL(stateChanged(int)),        this, SLOT(slotRun(int)) );
    QObject::connect(ui->checkFileRecord, SIGNAL(stateChanged(int)), this, SLOT(slotRecOnOff(int)) );
    QObject::connect(ui->pushButtonFile,  SIGNAL(clicked(bool)),     this, SLOT(slotChooseFile(bool)) );
    QObject::connect(ui->buttonDump,      SIGNAL(clicked(bool)),     this, SLOT(slotRecOneShot(bool)) );
    
    QObject::connect(ui->checkBoxCountGaps, SIGNAL(stateChanged(int)), this, SLOT(slotGapCountOnOff(int)) );
    QObject::connect(ui->spinBoxGapVal, SIGNAL(valueChanged(int)), this, SLOT(slotGapValChanged(int)) );

    SetLabels();

    checkBoxShowChannels.resize( MAX_CHANS * 2 );
    checkBoxShowChannels[ 0 ] = ui->checkBoxShowCh0;
    checkBoxShowChannels[ 1 ] = ui->checkBoxShowCh1;
    checkBoxShowChannels[ 2 ] = ui->checkBoxShowCh2;
    checkBoxShowChannels[ 3 ] = ui->checkBoxShowCh3;
    for ( int i = chan_count; i < MAX_CHANS; i++ ) {
        checkBoxShowChannels[ i ]->setChecked( false );
        checkBoxShowChannels[ i ]->setEnabled( false );
    }

    ui->comboBoxDumpType->clear();
    for ( int i = 0; i < ( int ) StreamDump_TypeCount; i++ ) {
        ui->comboBoxDumpType->insertItem( i, StreamDumpTypeNames[ i ] );
    }
}

RawSignalForm::~RawSignalForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
        router->DeleteOutPoint( &dumper );
    }
    delete ui;
}

void RawSignalForm::HandleADCStreamData(void*, size_t) {

}

void RawSignalForm::HandleStreamDataOneChan(short *one_ch_data, size_t in_pts_cnt, int channel) {
    if ( channel > chan_count ) {
        fprintf( stderr, "RawSignalForm Bad chan count config( rcv %d, cfg is %d )\n", channel, chan_count );
        return;
    }

    int module = ui->spinModule->value();
    int new_shift = last_shift[ channel ] + ui->spinShiftPts->value();
    int shift = ((int)new_shift) % module;
    shift = ( shift + module ) % module; // operation % can return negative values
    last_shift[ channel ] = shift;

    if ( replot_is_in_progress && channel == 0 ) {
        if ( ui->checkNoSkipMode->isChecked() ) {
            while ( replot_is_in_progress ) {
                QThread::msleep( 1 );
            }
        } else {
            skipped_packets++;
            return;
        }
    }
    points_num = ui->spinPtsNum->value();
    if ( points_num < 100 ) {
        points_num = 100;
    }

    if ( cfg->signal_type == SigTypeIQParts ) {
        std::vector<float> floatvec[ 2 ];

        int16_t* p16 = one_ch_data;
        p16 += shift;
        uint32_t out_pts_cnt = points_num > in_pts_cnt ? in_pts_cnt : points_num;
        floatvec[0].resize( out_pts_cnt / 2 );
        floatvec[1].resize( out_pts_cnt / 2 );
        for ( uint32_t i = 0; i < floatvec[0].size(); i++) {
            floatvec[0][i] = ( float ) *p16++;
            floatvec[1][i] = ( float ) *p16++;
        }
        showRawSignal( floatvec[0], channel, false );
        showRawSignal( floatvec[1], channel, true );
    } else {
        std::vector<float> floatvec;

        int16_t* p16 = one_ch_data;
        p16 += shift;
        uint32_t out_pts_cnt = points_num > in_pts_cnt ? in_pts_cnt : points_num;
        floatvec.resize( out_pts_cnt );
        for ( uint32_t i = 0; i < floatvec.size(); i++) {
            floatvec[i] = ( float ) p16[i];
        }
        showRawSignal( floatvec, channel, false );
    }
}

void RawSignalForm::SetLabels() {
    QString str( "Skipped packets = " );
    str += QString::number(skipped_packets);
    ui->labelSkippedCount->setText( str );
}

void RawSignalForm::showRawSignal(const std::vector<float>& data, int channel_num, bool is_second_component) {
    if ( channel_num > chan_count ) {
        fprintf( stderr, "RawSignalForm Bad chan count config( rcv %d, cfg is %d )\n", channel_num, chan_count );
        return;
    }

    if ( !checkBoxShowChannels[ channel_num ]->isChecked() ) {
        rawPlot->graph( channel_num )->clearData();
        if ( cfg->signal_type == SigTypeIQParts ) {
            rawPlot->graph( channel_num + MAX_CHANS )->clearData();
        }
        return;
    }
    QVector<double> keys(   data.size() );
    QVector<double> values( data.size() );
    
    double max = data[0];
    double min = data[0];
    for ( unsigned int i = 0; i < data.size(); i++ ) {
        keys[ i ] = i;
        values[ i ] = (double) data[i];
        if ( values[ i ] > max ) {
            max = values[ i ];
        }
        if ( values[ i ] < min ) {
            min = values[ i ];
        }
    }
    double avg = ( max + min ) / 2.0;
    double amp = ( max - min ) / 2.0;
    if ( amp < 0.1 ) {
        amp += 0.1;
    }
    amp *= 1.0 + axis_blank_percent;
    
    double axis_min = avg - amp;
    double axis_max = avg + amp;
    
    if ( first_run || ( fabs( axis_max - last_axis ) / fabs( last_axis ) > axis_blank_percent ) ) {
        first_run = false;
        rawPlot->yAxis->setRange( axis_min, axis_max );
        last_axis = axis_max;
    }
    
    rawPlot->xAxis->setRange(0, data.size());
    //rawPlot->rescaleAxes();

    int plot_idx = channel_num;
    if ( is_second_component ) {
        plot_idx += MAX_CHANS;
    }
    rawPlot->graph( plot_idx )->setData(keys, values);
    
    replot_is_in_progress = true;
    emit signalNeedReplot();
}

void RawSignalForm::slotReplot() {
    replot_is_in_progress = true;
    rawPlot->replot();
}

void RawSignalForm::slotReplotComplete() {
    replot_is_in_progress = false;
    SetLabels();
}

void RawSignalForm::slotRun(int state ) {
    fprintf( stderr, "RawSignalForm::slotRun(%d)\n", state );
    if ( router ) {
        if ( state ) {
            skipped_packets = 0;
            first_run = true;
            router->AddOutPoint(this);
        } else {
            router->DeleteOutPoint(this);
        }
    }
}

void RawSignalForm::slotRecOnOff(int state) {
    fprintf( stderr, "RawSignalForm::slotRecOnOff(%d)\n", state );
    if ( state ) {
        ui->lineRecFileName->setEnabled( false );
        dumper.SetDumpFileName(ui->lineRecFileName->text().toLatin1().data());
        dumper.StartDump( ( StreamDumpType_t ) ui->comboBoxDumpType->currentIndex(), ui->checkSaveTimeStamp->isChecked() );
        router->AddOutPoint(&dumper);
    } else {
        dumper.StopDump();
        ui->lineRecFileName->setEnabled( true );
        router->DeleteOutPoint(&dumper);
    }
}

void RawSignalForm::slotRecOneShot(bool) {
    ui->buttonDump->setEnabled( false );
    ui->lineRecFileName->setEnabled( false );

    int seconds = ui->spinBoxDumpSeconds->value();
    int64_t samples = seconds * cfg->adc_sample_rate_hz;
    fprintf( stderr, "RawSignalForm::slotRecOneShot(): %d seconds = %d samples\n",
             seconds, samples);

    dumper.SetDumpFileName(ui->lineRecFileName->text().toLatin1().data());
    dumper.StartDump( ( StreamDumpType_t ) ui->comboBoxDumpType->currentIndex(),
                      samples, this );
    router->AddOutPoint(&dumper);

}

void RawSignalForm::onFileDumpComplete() {
    ui->buttonDump->setEnabled( true );
    ui->lineRecFileName->setEnabled( true );
    router->DeleteOutPoint(&dumper);
}


void RawSignalForm::slotChooseFile(bool) {
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this,
        tr("Select file for signal dump"),
        "",
        tr("Binary files (*.bin);;All files (*.*)" )
    );
    if ( fileName.size() > 1 ) {
        ui->lineRecFileName->setText( fileName );
    }
}

void RawSignalForm::slotGapCountOnOff(int state) {
    if ( router && state ) {
        gap_checker.FlushStatistic();
        gap_checker.SetGap( ( double ) ui->spinBoxGapVal->value() );
        gap_checker.SetCallBack( this );
        router->AddOutPoint( &gap_checker );
    } else if ( router && !state ) {
        router->DeleteOutPoint( &gap_checker );
    }
}

void RawSignalForm::slotGapValChanged(int value ) {
    gap_checker.SetGap( value );
}

void RawSignalForm::closeEvent(QCloseEvent*ev) {
    ev->accept();
    ui->checkRun->setChecked(false);
    ui->checkFileRecord->setChecked(false);
    
}

void RawSignalForm::GapCountChanged(int gap_count, double max_gap_value) {
    QString str_gc( "Gaps count = " );
    str_gc += QString::number(gap_count);
    ui->labelGapsCount->setText( str_gc );

    QString str_mg( "Max gap = " );
    str_mg += QString::number(max_gap_value);
    ui->labelMaxGap->setText( str_mg );
}



