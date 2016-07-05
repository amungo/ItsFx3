#include "SpectrumForm.h"
#include "ui_SpectrumForm.h"

SpectrumForm::SpectrumForm( FX3Config* cfg, QWidget *parent ) :
    QWidget(parent),
    router( NULL ),
    ui(new Ui::SpectrumForm),
    cfg( cfg ),
    fft( NULL ),
    nFft( nFftDefault ),
    visN( nFft/2 ),
    fftbuf( NULL ),
    replot_is_in_progress( false )
{
    fftbuf = new float_cpx_t[ nFft ];
    fft = new FFTWrapper( nFft );

    ui->setupUi(this);

    specPlot = ui->widgetSpectrum;
    specPlot->addGraph();
    specPlot->addGraph();
    specPlot->addGraph();
    specPlot->addGraph();
    specPlot->graph(0)->setPen(QPen(Qt::green));
    specPlot->graph(1)->setPen(QPen(Qt::red));
    specPlot->graph(2)->setPen(QPen(Qt::blue));
    specPlot->graph(3)->setPen(QPen(Qt::lightGray));

    specPlot->xAxis->setRange(0, visN);
    specPlot->yAxis->setRange(0, 70);


    avg.resize( MAX_CHANS );
    for ( int i = 0; i < MAX_CHANS; i++ ) {
        avg[ i ] = new Averager<double>( visN, 10 );
    }

    checkBoxShowChannels.resize( MAX_CHANS );
    checkBoxShowChannels[ 0 ] = ui->checkBoxShowCh0;
    checkBoxShowChannels[ 1 ] = ui->checkBoxShowCh1;
    checkBoxShowChannels[ 2 ] = ui->checkBoxShowCh2;
    checkBoxShowChannels[ 3 ] = ui->checkBoxShowCh3;
    for ( int i = cfg->chan_count; i < MAX_CHANS; i++ ) {
        checkBoxShowChannels[ i ]->setChecked( false );
        checkBoxShowChannels[ i ]->setEnabled( false );
    }

    QObject::connect(this, SIGNAL(signalNeedReplot()), this, SLOT(slotReplot()) );
    QObject::connect(ui->widgetSpectrum, SIGNAL(afterReplot()), this, SLOT(slotReplotComplete()) );
    QObject::connect(ui->checkRun, SIGNAL(stateChanged(int)), this, SLOT(slotRun(int)) );
    QObject::connect(ui->comboBoxAvg, SIGNAL(currentIndexChanged(int)), this, SLOT(avgChanged(int)));
    QObject::connect(ui->widgetSpectrum, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(onMWheel(QWheelEvent*)));
    QObject::connect(ui->widgetSpectrum, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMPress(QMouseEvent*)));
    QObject::connect(ui->widgetSpectrum, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMRelease(QMouseEvent*)));

    ui->comboBoxAvg->addItem( "No avg", QVariant( 1 ) );
    ui->comboBoxAvg->addItem( "Avg 3 times", QVariant( 3 ) );
    ui->comboBoxAvg->addItem( "Avg 5 times", QVariant( 5 ) );
    ui->comboBoxAvg->addItem( "Avg 10 times", QVariant( 10 ) );
    ui->comboBoxAvg->addItem( "Avg 20 times", QVariant( 20 ) );
    ui->comboBoxAvg->addItem( "Avg 40 times", QVariant( 40 ) );
    ui->comboBoxAvg->setCurrentIndex( 1 );
}

SpectrumForm::~SpectrumForm()
{
    if ( router ) {
        router->DeleteOutPoint( this );
    }
    delete fft;
    if ( fftbuf ) {
        delete [] fftbuf;
    }
    delete ui;
}

void SpectrumForm::ShowSpectrumReal(const float *real_data, int pts_cnt, int channel_num) {
    if ( pts_cnt < nFft ) {
        return;
    }
    if ( !checkBoxShowChannels[ channel_num ]->isChecked() ) {
        specPlot->graph( channel_num )->clearData();
        return;
    }

    fft->Transform( real_data, fftbuf );

    double* pu = new double[ visN ];
    for ( int i = 0; i < visN; i++ ) {
        pu[ i ] = 10.0 * log10( fftbuf[i].len_squared() );
    }

    ShowSpectrum( channel_num, pu );
    delete [] pu;
}

void SpectrumForm::ShowSpectrumComplex(const float_cpx_t *complex_data, int pts_cnt, int channel_num) {
    if ( pts_cnt < nFft ) {
        return;
    }
    if ( !checkBoxShowChannels[ channel_num ]->isChecked() ) {
        specPlot->graph( channel_num )->clearData();
        return;
    }

    fft->Transform( complex_data, fftbuf, false );

    double* pu = new double[ visN ];
    for ( int i = 0; i < visN; i++ ) {
        pu[ i ] = 10.0 * log10( fftbuf[i].len_squared() );
    }

    ShowSpectrum( channel_num, pu );
    delete [] pu;
}

void SpectrumForm::ShowSpectrum(int channel_num, double* powers ) {
    mtx.lock(); // -------------

    avg[ channel_num ]->PushData( powers );


    QVector<double> keys( visN );
    QVector<double> values( visN );

    double freq = 0.0;
    double freq_step = cfg->adc_sample_rate_hz / ( double ) nFft;
    double max_freq = cfg->inter_freq_hz + cfg->adc_sample_rate_hz / 2.0;
    if ( cfg->signal_type == SigTypeIQParts ) {
        freq = cfg->inter_freq_hz - cfg->adc_sample_rate_hz / 2.0;
    } else if ( cfg->signal_type == SigTypeRealPartOnly ) {
        freq = cfg->inter_freq_hz;
    }

    double k = 1.0;
    QString legend = "Freq";
    if ( max_freq > 1.e9 ) {
        legend = "Freq, GHz";
        k = 1.0e-9;
    } else if ( max_freq > 1.e6 ) {
        legend = "Freq, MHz";
        k = 1.0e-6;
    } else if ( max_freq > 1.e3 ) {
        legend = "Freq, kHz";
        k = 1.0e-3;
    }
    freq *= k;
    freq_step *= k;

    const double* ap = avg[ channel_num ]->GetData();
    for ( int i = 0; i < visN; i++ ) {
        values[ i ] = ap[i];
        keys[ i ] = freq;
        freq += freq_step;
    }
    mtx.unlock(); // ------------

    specPlot->graph( channel_num )->setData(keys, values);
    specPlot->xAxis->setRange( keys[ 0 ], keys[ visN - 1 ] );
    specPlot->xAxis->setLabel( legend );
    //specPlot->rescaleAxes();

    if ( channel_num == GetMaxCheckedChannel() ) {
        replot_is_in_progress = true;
        emit signalNeedReplot();
    }
}

int SpectrumForm::GetMaxCheckedChannel() {
    for ( int i = MAX_CHANS - 1; i >= 0; i-- ) {
        if ( checkBoxShowChannels[ i ]->isChecked() ) {
            return i;
        }
    }
    return -1;
}

void SpectrumForm::slotReplot() {
    replot_is_in_progress = true;
    specPlot->replot();
}

void SpectrumForm::slotReplotComplete() {
    replot_is_in_progress = false;
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

void SpectrumForm::avgChanged(int) {
    mtx.lock();
    bool ok;
    int nAvg = ui->comboBoxAvg->currentData().toInt( &ok );
    fprintf( stderr, "avg count changed. New val = %d", nAvg );
    for ( size_t i = 0; i < avg.size(); i++ ) {
        delete avg[ i ];
        avg[ i ] = new Averager< double >( visN, nAvg );
    }
    mtx.unlock();
}

void SpectrumForm::onMWheel(QWheelEvent* mevt) {
    replot_is_in_progress = true;
    const QCPRange cur = specPlot->yAxis->range();
    double cur_size = cur.size();
    double center = cur.center();

    int d = mevt->delta();

    cur_size += d / 20.0;
    specPlot->yAxis->setRange(center - cur_size / 2.0, center + cur_size / 2.0);
    emit signalNeedReplot();
}

void SpectrumForm::onMPress(QMouseEvent* evt) {
    if ( evt->button() == Qt::LeftButton ) {
        startMove = evt->pos();
    }
}

void SpectrumForm::onMRelease(QMouseEvent* evt) {
    if ( evt->button() == Qt::LeftButton ) {
        QPoint endMove = evt->pos();
        QPoint delta = startMove - endMove;
        fprintf( stderr, "delta %d pixels, H = %d pixels\n", delta.y(), specPlot->size().height() );

        const QCPRange cur = specPlot->yAxis->range();
        double cur_size = cur.size();
        double center = cur.center();
        double vals_in_pixel = cur_size / ( double ) specPlot->size().height();


        center -= delta.y() * vals_in_pixel;
        specPlot->yAxis->setRange(center - cur_size / 2.0, center + cur_size / 2.0);
        emit signalNeedReplot();
    }
}

void SpectrumForm::HandleADCStreamData(void *, size_t) {
    // nop
}

void SpectrumForm::HandleStreamDataOneChan(short *one_ch_data, size_t pts_cnt, int channel) {
    if ( replot_is_in_progress ) {
        return;
    }

    if ( channel > cfg->chan_count ) {
        return;
    }
    if ( cfg->signal_type == SigTypeRealPartOnly ) {

        int N = pts_cnt >= nFft ? nFft : pts_cnt;
        float* pr = new float[ N ];
        for ( int i = 0; i < N; i++ ) {
            pr[ i ] = ( float ) one_ch_data[ i ];
        }
        ShowSpectrumReal( pr, N, channel );
        delete [] pr;
    } else {

        pts_cnt /= 2;
        int N = pts_cnt >= nFft ? nFft : pts_cnt;
        float_cpx_t* cpx = new float_cpx_t[ N ];
        for ( int k = 0; k < N; k++ ) {
            cpx[ k ].i = ( float ) one_ch_data[ 2*k + 0 ];
            cpx[ k ].q = ( float ) one_ch_data[ 2*k + 1 ];
        }
        ShowSpectrumComplex( cpx, N, channel );
        delete [] cpx;
    }
}
